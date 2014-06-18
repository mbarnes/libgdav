/*
 * Copyright (C) 2014 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the licence, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Matthew Barnes <mbarnes@redhat.com>
 */

#include "config.h"

#include "gdav-methods.h"

#include <glib/gi18n-lib.h>

#include "gdav-utils.h"

typedef struct _TaskData TaskData;

struct _TaskData {
	SoupMessage *message;
	GDavAllow allow;
	GDavOptions options;
};

static void
task_data_free (TaskData *task_data)
{
	g_clear_object (&task_data->message);

	g_slice_free (TaskData, task_data);
}

static void
gdav_request_apply_response (SoupMessage *message,
                             GOutputStream *output_stream)
{
	g_return_if_fail (message != NULL);

	/* XXX That the input stream content is not automatically
	 *     copied to the SoupMessage's response_body is a known
	 *     libsoup bug which may be fixed in a future release.
	 *     Check that the response body is empty so we don't
	 *     accidentally duplicate the body. */
	if (message->response_body->data == NULL) {
		GMemoryOutputStream *memory_stream;
		gpointer data;
		gsize size;

		memory_stream = G_MEMORY_OUTPUT_STREAM (output_stream);
		size = g_memory_output_stream_get_data_size (memory_stream);
		data = g_memory_output_stream_steal_data (memory_stream);

		soup_message_body_append_take (
			message->response_body, data, size);
		soup_message_body_flatten (message->response_body);
	}
}

static gboolean
gdav_request_send_sync (SoupRequestHTTP *request,
                        GCancellable *cancellable,
                        GError **error)
{
	GInputStream *input_stream;
	gboolean success = FALSE;

	/* This is an internal wrapper for soup_request_send().
	 * The input stream contents are written to the SoupMessage
	 * response body to ensure a SoupLogger sees it. */

	input_stream = soup_request_send (
		SOUP_REQUEST (request), cancellable, error);

	if (input_stream != NULL) {
		GOutputStream *output_stream;
		gssize n_bytes = -1;

		output_stream = g_memory_output_stream_new_resizable ();

		/* Don't close the input stream automatically,
		 * we'll need to do some post-processing first. */
		n_bytes = g_output_stream_splice (
			output_stream, input_stream,
			G_OUTPUT_STREAM_SPLICE_CLOSE_TARGET,
			cancellable, error);

		if (n_bytes >= 0) {
			SoupMessage *message;

			message = soup_request_http_get_message (request);
			gdav_request_apply_response (message, output_stream);
			success = gdav_message_is_successful (message, error);
			g_object_unref (message);
		}

		g_object_unref (output_stream);
		g_object_unref (input_stream);
	}

	return success;
}

/* Helper for gdav_request_send() */
static void
gdav_request_splice_cb (GObject *source_object,
                        GAsyncResult *result,
                        gpointer user_data)
{
	GInputStream *input_stream;
	GOutputStream *output_stream;
	SoupMessage *message = NULL;
	GTask *task = G_TASK (user_data);
	GError *local_error = NULL;

	input_stream = g_task_get_task_data (task);
	output_stream = G_OUTPUT_STREAM (source_object);

	/* XXX We're not supposed to know this, but the input stream
	 *     is a SoupClientInputStream which holds the SoupMessage. */
	g_object_get (input_stream, "message", &message, NULL);
	g_return_if_fail (message != NULL);

	g_output_stream_splice_finish (output_stream, result, &local_error);

	if (local_error == NULL) {
		gdav_request_apply_response (message, output_stream);
		gdav_message_is_successful (message, &local_error);
	}

	if (local_error != NULL)
		g_task_return_error (task, local_error);
	else
		g_task_return_boolean (task, TRUE);

	g_object_unref (message);
	g_object_unref (task);
}

/* Helper for gdav_request_send() */
static void
gdav_request_send_cb (GObject *source_object,
                      GAsyncResult *result,
                      gpointer user_data)
{
	GInputStream *input_stream;
	GTask *task = G_TASK (user_data);
	GError *local_error = NULL;

	input_stream = soup_request_send_finish (
		SOUP_REQUEST (source_object), result, &local_error);

	/* Sanity check */
	g_warn_if_fail (
		((input_stream != NULL) && (local_error == NULL)) ||
		((input_stream == NULL) && (local_error != NULL)));

	if (input_stream != NULL) {
		GCancellable *cancellable;
		GOutputStream *output_stream;

		g_task_set_task_data (
			task, g_object_ref (input_stream),
			(GDestroyNotify) g_object_unref);

		cancellable = g_task_get_cancellable (task);

		output_stream = g_memory_output_stream_new_resizable ();

		/* Don't close the input stream automatically,
		 * we'll need to do some post-processing first. */
		g_output_stream_splice_async (
			output_stream, input_stream,
			G_OUTPUT_STREAM_SPLICE_CLOSE_TARGET,
			G_PRIORITY_DEFAULT, cancellable,
			gdav_request_splice_cb,
			g_object_ref (task));

		g_object_unref (output_stream);
		g_object_unref (input_stream);
	}

	if (local_error != NULL)
		g_task_return_error (task, local_error);

	g_object_unref (task);
}

static void
gdav_request_send (SoupRequestHTTP *request,
                   GCancellable *cancellable,
                   GAsyncReadyCallback callback,
                   gpointer user_data)
{
	GTask *task;

	/* This is an internal wrapper for soup_request_send_async().
	 * The input stream contents are written to the SoupMessage
	 * response body to ensure a SoupLogger sees it. */

	task = g_task_new (request, cancellable, callback, user_data);

	soup_request_send_async (
		SOUP_REQUEST (request),
		cancellable,
		gdav_request_send_cb,
		g_object_ref (task));

	g_object_unref (task);
}

static gboolean
gdav_request_send_finish (SoupRequestHTTP *request,
                          GAsyncResult *result,
                          GError **error)
{
	g_return_val_if_fail (g_task_is_valid (result, request), FALSE);

	return g_task_propagate_boolean (G_TASK (result), error);
}

gboolean
gdav_options_sync (SoupSession *session,
                   SoupURI *uri,
                   GDavAllow *out_allow,
                   GDavOptions *out_options,
                   SoupMessage **out_message,
                   GCancellable *cancellable,
                   GError **error)
{
	SoupRequestHTTP *request;
	SoupMessage *message;
	gboolean success;

	g_return_val_if_fail (SOUP_IS_SESSION (session), FALSE);
	g_return_val_if_fail (uri != NULL, FALSE);

	request = gdav_request_options_uri (session, uri, error);

	if (request == NULL) {
		if (out_message != NULL)
			*out_message = NULL;
		return FALSE;
	}

	message = soup_request_http_get_message (request);
	success = gdav_request_send_sync (request, cancellable, error);

	if (success) {
		if (out_allow != NULL) {
			*out_allow = gdav_allow_from_headers (
				message->response_headers);
		}

		if (out_options != NULL) {
			*out_options = gdav_options_from_headers (
				message->response_headers);
		}
	}

	/* SoupMessage is set even in case of error for uses
	 * like calling soup_message_get_https_status() when
	 * SSL/TLS negotiation fails. */
	if (out_message != NULL)
		*out_message = g_object_ref (message);

	g_object_unref (message);
	g_object_unref (request);

	return success;
}

static void
gdav_options_request_cb (GObject *source_object,
                         GAsyncResult *result,
                         gpointer user_data)
{
	SoupRequestHTTP *request;
	SoupMessage *message;
	GTask *task = G_TASK (user_data);
	TaskData *task_data;
	GError *local_error = NULL;

	request = SOUP_REQUEST_HTTP (source_object);
	message = soup_request_http_get_message (request);
	task_data = g_task_get_task_data (task);

	if (gdav_request_send_finish (request, result, &local_error)) {
		task_data->allow =
			gdav_allow_from_headers (
			message->response_headers);
		task_data->options =
			gdav_options_from_headers (
			message->response_headers);
		g_task_return_boolean (task, TRUE);
	} else {
		g_task_return_error (task, local_error);
	}

	g_object_unref (message);
	g_object_unref (task);
}

void
gdav_options (SoupSession *session,
              SoupURI *uri,
              GCancellable *cancellable,
              GAsyncReadyCallback callback,
              gpointer user_data)
{
	GTask *task;
	SoupRequestHTTP *request;
	TaskData *task_data;
	GError *local_error = NULL;

	g_return_if_fail (SOUP_IS_SESSION (session));
	g_return_if_fail (uri != NULL);

	task_data = g_slice_new0 (TaskData);

	task = g_task_new (session, cancellable, callback, user_data);
	g_task_set_source_tag (task, gdav_options);

	g_task_set_task_data (
		task, task_data, (GDestroyNotify) task_data_free);

	request = gdav_request_options_uri (session, uri, &local_error);

	/* Sanity check */
	g_return_if_fail (
		((request != NULL) && (local_error == NULL)) ||
		((request == NULL) && (local_error != NULL)));

	if (request != NULL) {
		task_data->message =
			soup_request_http_get_message (request);

		gdav_request_send (
			request, cancellable,
			gdav_options_request_cb,
			g_object_ref (task));

		g_object_unref (request);
	} else {
		g_task_return_error (task, local_error);
	}

	g_object_unref (task);
}

gboolean
gdav_options_finish (SoupSession *session,
                     GAsyncResult *result,
                     GDavAllow *out_allow,
                     GDavOptions *out_options,
                     SoupMessage **out_message,
                     GError **error)
{
	TaskData *task_data;

	g_return_val_if_fail (
		g_task_is_valid (result, session), FALSE);
	g_return_val_if_fail (
		g_async_result_is_tagged (result, gdav_options), FALSE);

	task_data = g_task_get_task_data (G_TASK (result));

	if (!g_task_had_error (G_TASK (result))) {
		if (out_allow != NULL)
			*out_allow = task_data->allow;
		if (out_options != NULL)
			*out_options = task_data->options;
	}

	/* SoupMessage is set even in case of error for uses
	 * like calling soup_message_get_https_status() when
	 * SSL/TLS negotiation fails, though SoupMessage may
	 * be NULL if the Request-URI was invalid. */
	if (out_message != NULL) {
		*out_message = task_data->message;
		task_data->message = NULL;
	}

	return g_task_propagate_boolean (G_TASK (result), error);
}

GDavMultiStatus *
gdav_propfind_sync (SoupSession *session,
                    SoupURI *uri,
                    GDavPropFindType type,
                    GDavPropertySet *prop,
                    GDavDepth depth,
                    SoupMessage **out_message,
                    GCancellable *cancellable,
                    GError **error)
{
	SoupRequestHTTP *request;
	SoupMessage *message;
	GDavMultiStatus *multi_status = NULL;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri != NULL, NULL);

	request = gdav_request_propfind_uri (
		session, uri, type, prop, depth, error);

	if (request == NULL) {
		if (out_message != NULL)
			*out_message = NULL;
		return NULL;
	}

	message = soup_request_http_get_message (request);

	if (gdav_request_send_sync (request, cancellable, error)) {
		multi_status = gdav_multi_status_new_from_message (
			message, error);
	}

	/* SoupMessage is set even in case of error for uses
	 * like calling soup_message_get_https_status() when
	 * SSL/TLS negotiation fails. */
	if (out_message != NULL)
		*out_message = g_object_ref (message);

	g_object_unref (message);
	g_object_unref (request);

	return multi_status;
}

static void
gdav_propfind_request_cb (GObject *source_object,
                          GAsyncResult *result,
                          gpointer user_data)
{
	SoupRequestHTTP *request;
	SoupMessage *message;
	GTask *task = G_TASK (user_data);
	GDavMultiStatus *multi_status;
	GError *local_error = NULL;

	request = SOUP_REQUEST_HTTP (source_object);
	message = soup_request_http_get_message (request);

	if (gdav_request_send_finish (request, result, &local_error)) {
		multi_status = gdav_multi_status_new_from_message (
			message, &local_error);
	}

	/* Sanity check */
	g_warn_if_fail (
		((multi_status != NULL) && (local_error == NULL)) ||
		((multi_status == NULL) && (local_error != NULL)));

	if (multi_status != NULL)
		g_task_return_pointer (task, multi_status, g_object_unref);
	else
		g_task_return_error (task, local_error);

	g_object_unref (message);
	g_object_unref (task);
}

void
gdav_propfind (SoupSession *session,
               SoupURI *uri,
               GDavPropFindType type,
               GDavPropertySet *prop,
               GDavDepth depth,
               GCancellable *cancellable,
               GAsyncReadyCallback callback,
               gpointer user_data)
{
	GTask *task;
	SoupRequestHTTP *request;
	TaskData *task_data;
	GError *local_error = NULL;

	g_return_if_fail (SOUP_IS_SESSION (session));
	g_return_if_fail (uri != NULL);

	task_data = g_slice_new0 (TaskData);

	task = g_task_new (session, cancellable, callback, user_data);
	g_task_set_source_tag (task, gdav_propfind);

	g_task_set_task_data (
		task, task_data, (GDestroyNotify) task_data_free);

	request = gdav_request_propfind_uri (
		session, uri, type, prop, depth, &local_error);

	/* Sanity check */
	g_warn_if_fail (
		((request != NULL) && (local_error == NULL)) ||
		((request == NULL) && (local_error != NULL)));

	if (request != NULL) {
		task_data->message =
			soup_request_http_get_message (request);

		gdav_request_send (
			request, cancellable,
			gdav_propfind_request_cb,
			g_object_ref (task));

		g_object_unref (request);
	} else {
		g_task_return_error (task, local_error);
	}

	g_object_unref (task);
}

GDavMultiStatus *
gdav_propfind_finish (SoupSession *session,
                      GAsyncResult *result,
                      SoupMessage **out_message,
                      GError **error)
{
	TaskData *task_data;

	g_return_val_if_fail (
		g_task_is_valid (result, session), NULL);
	g_return_val_if_fail (
		g_async_result_is_tagged (result, gdav_propfind), NULL);

	task_data = g_task_get_task_data (G_TASK (result));

	/* SoupMessage is set even in case of error for uses
	 * like calling soup_message_get_https_status() when
	 * SSL/TLS negotiation fails, though SoupMessage may
	 * be NULL if the Request-URI was invalid. */
	if (out_message != NULL) {
		*out_message = task_data->message;
		task_data->message = NULL;
	}

	return g_task_propagate_pointer (G_TASK (result), error);
}

GDavMultiStatus *
gdav_proppatch_sync (SoupSession *session,
                     SoupURI *uri,
                     GDavPropertyUpdate *update,
                     SoupMessage **out_message,
                     GCancellable *cancellable,
                     GError **error)
{
	SoupRequestHTTP *request;
	SoupMessage *message;
	GDavMultiStatus *multi_status = NULL;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri != NULL, NULL);

	request = gdav_request_proppatch_uri (session, uri, update, error);

	if (request == NULL) {
		if (out_message != NULL)
			*out_message = NULL;
		return NULL;
	}

	message = soup_request_http_get_message (request);

	if (gdav_request_send_sync (request, cancellable, error)) {
		multi_status = gdav_multi_status_new_from_message (
			message, error);
	}

	/* SoupMessage is set even in case of error for uses
	 * like calling soup_message_get_https_status() when
	 * SSL/TLS negotiation fails. */
	if (out_message != NULL)
		*out_message = g_object_ref (message);

	g_object_unref (message);
	g_object_unref (request);

	return multi_status;
}

static void
gdav_proppatch_request_cb (GObject *source_object,
                           GAsyncResult *result,
                           gpointer user_data)
{
	SoupRequestHTTP *request;
	SoupMessage *message;
	GTask *task = G_TASK (user_data);
	GDavMultiStatus *multi_status = NULL;
	GError *local_error = NULL;

	request = SOUP_REQUEST_HTTP (source_object);
	message = soup_request_http_get_message (request);

	if (gdav_request_send_finish (request, result, &local_error)) {
		multi_status = gdav_multi_status_new_from_message (
			message, &local_error);
	}

	/* Sanity check */
	g_warn_if_fail (
		((multi_status != NULL) && (local_error == NULL)) ||
		((multi_status == NULL) && (local_error != NULL)));

	if (multi_status != NULL)
		g_task_return_pointer (task, multi_status, g_object_unref);
	else
		g_task_return_error (task, local_error);

	g_object_unref (message);
	g_object_unref (task);
}

void
gdav_proppatch (SoupSession *session,
                SoupURI *uri,
                GDavPropertyUpdate *update,
                GCancellable *cancellable,
                GAsyncReadyCallback callback,
                gpointer user_data)
{
	GTask *task;
	SoupRequestHTTP *request;
	TaskData *task_data;
	GError *local_error = NULL;

	g_return_if_fail (SOUP_IS_SESSION (session));
	g_return_if_fail (uri != NULL);
	g_return_if_fail (GDAV_IS_PROPERTY_UPDATE (update));

	task_data = g_slice_new0 (TaskData);

	task = g_task_new (session, cancellable, callback, user_data);
	g_task_set_source_tag (task, gdav_proppatch);

	g_task_set_task_data (
		task, task_data, (GDestroyNotify) task_data_free);

	request = gdav_request_proppatch_uri (
		session, uri, update, &local_error);

	/* Sanity check */
	g_warn_if_fail (
		((request != NULL) && (local_error == NULL)) ||
		((request == NULL) && (local_error != NULL)));

	if (request != NULL) {
		task_data->message =
			soup_request_http_get_message (request);

		gdav_request_send (
			request, cancellable,
			gdav_proppatch_request_cb,
			g_object_ref (task));

		g_object_unref (request);
	} else {
		g_task_return_error (task, local_error);
	}

	g_object_unref (task);
}

GDavMultiStatus *
gdav_proppatch_finish (SoupSession *session,
                       GAsyncResult *result,
                       SoupMessage **out_message,
                       GError **error)
{
	TaskData *task_data;

	g_return_val_if_fail (
		g_task_is_valid (result, session), NULL);
	g_return_val_if_fail (
		g_async_result_is_tagged (result, gdav_proppatch), NULL);

	task_data = g_task_get_task_data (G_TASK (result));

	/* SoupMessage is set even in case of error for uses
	 * like calling soup_message_get_https_status() when
	 * SSL/TLS negotiation fails, though SoupMessage may
	 * be NULL if the Request-URI was invalid. */
	if (out_message != NULL) {
		*out_message = task_data->message;
		task_data->message = NULL;
	}

	return g_task_propagate_pointer (G_TASK (result), error);
}

gboolean
gdav_mkcol_sync (SoupSession *session,
                 SoupURI *uri,
                 SoupMessage **out_message,
                 GCancellable *cancellable,
                 GError **error)
{
	SoupRequestHTTP *request;
	SoupMessage *message;
	gboolean success;

	g_return_val_if_fail (SOUP_IS_SESSION (session), FALSE);
	g_return_val_if_fail (uri != NULL, FALSE);

	request = gdav_request_mkcol_uri (session, uri, error);

	if (request == NULL) {
		if (out_message != NULL)
			*out_message = NULL;
		return FALSE;
	}

	message = soup_request_http_get_message (request);
	success = gdav_request_send_sync (request, cancellable, error);

	/* SoupMessage is set even in case of error for uses
	 * like calling soup_message_get_https_status() when
	 * SSL/TLS negotiation fails. */
	if (out_message != NULL)
		*out_message = g_object_ref (message);

	g_object_unref (message);
	g_object_unref (request);

	return success;
}

static void
gdav_mkcol_request_cb (GObject *source_object,
                       GAsyncResult *result,
                       gpointer user_data)
{
	SoupRequestHTTP *request;
	GTask *task = G_TASK (user_data);
	GError *local_error = NULL;

	request = SOUP_REQUEST_HTTP (source_object);

	if (gdav_request_send_finish (request, result, &local_error))
		g_task_return_boolean (task, TRUE);
	else
		g_task_return_error (task, local_error);

	g_object_unref (task);
}

void
gdav_mkcol (SoupSession *session,
            SoupURI *uri,
            GCancellable *cancellable,
            GAsyncReadyCallback callback,
            gpointer user_data)
{
	GTask *task;
	SoupRequestHTTP *request;
	TaskData *task_data;
	GError *local_error = NULL;

	g_return_if_fail (SOUP_IS_SESSION (session));
	g_return_if_fail (uri != NULL);

	task_data = g_slice_new0 (TaskData);

	task = g_task_new (session, cancellable, callback, user_data);
	g_task_set_source_tag (task, gdav_mkcol);

	g_task_set_task_data (
		task, task_data, (GDestroyNotify) task_data_free);

	request = gdav_request_mkcol_uri (session, uri, &local_error);

	/* Sanity check */
	g_warn_if_fail (
		((request != NULL) && (local_error == NULL)) ||
		((request == NULL) && (local_error != NULL)));

	if (request != NULL) {
		task_data->message =
			soup_request_http_get_message (request);

		gdav_request_send (
			request, cancellable,
			gdav_mkcol_request_cb,
			g_object_ref (task));

		g_object_unref (request);
	} else {
		g_task_return_error (task, local_error);
	}

	g_object_unref (task);
}

gboolean
gdav_mkcol_finish (SoupSession *session,
                   GAsyncResult *result,
                   SoupMessage **out_message,
                   GError **error)
{
	TaskData *task_data;

	g_return_val_if_fail (
		g_task_is_valid (result, session), FALSE);
	g_return_val_if_fail (
		g_async_result_is_tagged (result, gdav_mkcol), FALSE);

	task_data = g_task_get_task_data (G_TASK (result));

	/* SoupMessage is set even in case of error for uses
	 * like calling soup_message_get_https_status() when
	 * SSL/TLS negotiation fails, though SoupMessage may
	 * be NULL if the Request-URI was invalid. */
	if (out_message != NULL) {
		*out_message = task_data->message;
		task_data->message = NULL;
	}

	return g_task_propagate_boolean (G_TASK (result), error);
}

gboolean
gdav_delete_sync (SoupSession *session,
                  SoupURI *uri,
                  SoupMessage **out_message,
                  GCancellable *cancellable,
                  GError **error)
{
	SoupRequestHTTP *request;
	SoupMessage *message;
	gboolean success;

	g_return_val_if_fail (SOUP_IS_SESSION (session), FALSE);
	g_return_val_if_fail (uri != NULL, FALSE);

	request = gdav_request_delete_uri (session, uri, error);

	if (request == NULL) {
		if (out_message != NULL)
			*out_message = NULL;
		return FALSE;
	}

	message = soup_request_http_get_message (request);
	success = gdav_request_send_sync (request, cancellable, error);

	/* SoupMessage is set even in case of error for uses
	 * like calling soup_message_get_https_status() when
	 * SSL/TLS negotiation fails. */
	if (out_message != NULL)
		*out_message = g_object_ref (message);

	g_object_unref (message);
	g_object_unref (request);

	return success;
}

static void
gdav_delete_request_cb (GObject *source_object,
                        GAsyncResult *result,
                        gpointer user_data)
{
	SoupRequestHTTP *request;
	GTask *task = G_TASK (user_data);
	GError *local_error = NULL;

	request = SOUP_REQUEST_HTTP (source_object);

	if (gdav_request_send_finish (request, result, &local_error))
		g_task_return_boolean (task, TRUE);
	else
		g_task_return_error (task, local_error);

	g_object_unref (task);
}

void
gdav_delete (SoupSession *session,
             SoupURI *uri,
             GCancellable *cancellable,
             GAsyncReadyCallback callback,
             gpointer user_data)
{
	GTask *task;
	SoupRequestHTTP *request;
	TaskData *task_data;
	GError *local_error = NULL;

	g_return_if_fail (SOUP_IS_SESSION (session));
	g_return_if_fail (uri != NULL);

	task_data = g_slice_new0 (TaskData);

	task = g_task_new (session, cancellable, callback, user_data);
	g_task_set_source_tag (task, gdav_delete);

	g_task_set_task_data (
		task, task_data, (GDestroyNotify) task_data_free);

	request = gdav_request_delete_uri (session, uri, &local_error);

	/* Sanity check */
	g_warn_if_fail (
		((request != NULL) && (local_error == NULL)) ||
		((request == NULL) && (local_error != NULL)));

	if (request != NULL) {
		task_data->message =
			soup_request_http_get_message (request);

		gdav_request_send (
			request, cancellable,
			gdav_delete_request_cb,
			g_object_ref (task));

		g_object_unref (request);
	} else {
		g_task_return_error (task, local_error);
	}

	g_object_unref (task);
}

gboolean
gdav_delete_finish (SoupSession *session,
                    GAsyncResult *result,
                    SoupMessage **out_message,
                    GError **error)
{
	TaskData *task_data;

	g_return_val_if_fail (
		g_task_is_valid (result, session), FALSE);
	g_return_val_if_fail (
		g_async_result_is_tagged (result, gdav_delete), FALSE);

	task_data = g_task_get_task_data (G_TASK (result));

	/* SoupMessage is set even in case of error for uses
	 * like calling soup_message_get_https_status() when
	 * SSL/TLS negotiation fails, though SoupMessage may
	 * be NULL if the Request-URI was invalid. */
	if (out_message != NULL) {
		*out_message = task_data->message;
		task_data->message = NULL;
	}

	return g_task_propagate_boolean (G_TASK (result), error);
}

gboolean
gdav_copy_sync (SoupSession *session,
                SoupURI *uri,
                const gchar *destination,
                GDavCopyFlags flags,
                SoupMessage **out_message,
                GCancellable *cancellable,
                GError **error)
{
	SoupRequestHTTP *request;
	SoupMessage *message;
	gboolean success;

	g_return_val_if_fail (SOUP_IS_SESSION (session), FALSE);
	g_return_val_if_fail (uri != NULL, FALSE);

	request = gdav_request_copy_uri (
		session, uri, destination, flags, error);

	if (request == NULL) {
		if (out_message != NULL)
			*out_message = NULL;
		return FALSE;
	}

	message = soup_request_http_get_message (request);
	success = gdav_request_send_sync (request, cancellable, error);

	/* SoupMessage is set even in case of error for uses
	 * like calling soup_message_get_https_status() when
	 * SSL/TLS negotiation fails. */
	if (out_message != NULL)
		*out_message = g_object_ref (message);

	g_object_unref (message);
	g_object_unref (request);

	return success;
}

static void
gdav_copy_request_cb (GObject *source_object,
                      GAsyncResult *result,
                      gpointer user_data)
{
	SoupRequestHTTP *request;
	GTask *task = G_TASK (user_data);
	GError *local_error = NULL;

	request = SOUP_REQUEST_HTTP (source_object);

	if (gdav_request_send_finish (request, result, &local_error))
		g_task_return_boolean (task, TRUE);
	else
		g_task_return_error (task, local_error);

	g_object_unref (task);
}

void
gdav_copy (SoupSession *session,
           SoupURI *uri,
           const gchar *destination,
           GDavCopyFlags flags,
           GCancellable *cancellable,
           GAsyncReadyCallback callback,
           gpointer user_data)
{
	GTask *task;
	SoupRequestHTTP *request;
	TaskData *task_data;
	GError *local_error = NULL;

	g_return_if_fail (SOUP_IS_SESSION (session));
	g_return_if_fail (uri != NULL);

	task_data = g_slice_new0 (TaskData);

	task = g_task_new (session, cancellable, callback, user_data);
	g_task_set_source_tag (task, gdav_copy);

	g_task_set_task_data (
		task, task_data, (GDestroyNotify) task_data_free);

	request = gdav_request_copy_uri (
		session, uri, destination, flags, &local_error);

	/* Sanity check */
	g_warn_if_fail (
		((request != NULL) && (local_error == NULL)) ||
		((request == NULL) && (local_error != NULL)));

	if (request != NULL) {
		task_data->message =
			soup_request_http_get_message (request);

		gdav_request_send (
			request, cancellable,
			gdav_copy_request_cb,
			g_object_ref (task));

		g_object_unref (request);
	} else {
		g_task_return_error (task, local_error);
	}

	g_object_unref (task);
}

gboolean
gdav_copy_finish (SoupSession *session,
                  GAsyncResult *result,
                  SoupMessage **out_message,
                  GError **error)
{
	TaskData *task_data;

	g_return_val_if_fail (
		g_task_is_valid (result, session), FALSE);
	g_return_val_if_fail (
		g_async_result_is_tagged (result, gdav_copy), FALSE);

	task_data = g_task_get_task_data (G_TASK (result));

	/* SoupMessage is set even in case of error for uses
	 * like calling soup_message_get_https_status() when
	 * SSL/TLS negotiation fails, though SoupMessage may
	 * be NULL if the Request-URI was invalid. */
	if (out_message != NULL) {
		*out_message = task_data->message;
		task_data->message = NULL;
	}

	return g_task_propagate_boolean (G_TASK (result), error);
}

gboolean
gdav_move_sync (SoupSession *session,
                SoupURI *uri,
                const gchar *destination,
                GDavMoveFlags flags,
                SoupMessage **out_message,
                GCancellable *cancellable,
                GError **error)
{
	SoupRequestHTTP *request;
	SoupMessage *message;
	gboolean success;

	g_return_val_if_fail (SOUP_IS_SESSION (session), FALSE);
	g_return_val_if_fail (uri != NULL, FALSE);

	request = gdav_request_move_uri (
		session, uri, destination, flags, error);

	if (request == NULL) {
		if (out_message != NULL)
			*out_message = NULL;
		return FALSE;
	}

	message = soup_request_http_get_message (request);
	success = gdav_request_send_sync (request, cancellable, error);

	/* SoupMessage is set even in case of error for uses
	 * like calling soup_message_get_https_status() when
	 * SSL/TLS negotiation fails. */
	if (out_message != NULL)
		*out_message = g_object_ref (message);

	g_object_unref (message);
	g_object_unref (request);

	return success;
}

static void
gdav_move_request_cb (GObject *source_object,
                      GAsyncResult *result,
                      gpointer user_data)
{
	SoupRequestHTTP *request;
	GTask *task = G_TASK (user_data);
	GError *local_error = NULL;

	request = SOUP_REQUEST_HTTP (source_object);

	if (gdav_request_send_finish (request, result, &local_error))
		g_task_return_boolean (task, TRUE);
	else
		g_task_return_error (task, local_error);

	g_object_unref (task);
}

void
gdav_move (SoupSession *session,
           SoupURI *uri,
           const gchar *destination,
           GDavMoveFlags flags,
           GCancellable *cancellable,
           GAsyncReadyCallback callback,
           gpointer user_data)
{
	GTask *task;
	SoupRequestHTTP *request;
	TaskData *task_data;
	GError *local_error = NULL;

	g_return_if_fail (SOUP_IS_SESSION (session));
	g_return_if_fail (uri != NULL);

	task_data = g_slice_new0 (TaskData);

	task = g_task_new (session, cancellable, callback, user_data);
	g_task_set_source_tag (task, gdav_move);

	g_task_set_task_data (
		task, task_data, (GDestroyNotify) task_data_free);

	request = gdav_request_move_uri (
		session, uri, destination, flags, &local_error);

	/* Sanity check */
	g_warn_if_fail (
		((request != NULL) && (local_error == NULL)) ||
		((request == NULL) && (local_error != NULL)));

	if (request != NULL) {
		task_data->message =
			soup_request_http_get_message (request);

		gdav_request_send (
			request, cancellable,
			gdav_move_request_cb,
			g_object_ref (task));

		g_object_unref (request);
	} else {
		g_task_return_error (task, local_error);
	}

	g_object_unref (task);
}

gboolean
gdav_move_finish (SoupSession *session,
                  GAsyncResult *result,
                  SoupMessage **out_message,
                  GError **error)
{
	TaskData *task_data;

	g_return_val_if_fail (
		g_task_is_valid (result, session), FALSE);
	g_return_val_if_fail (
		g_async_result_is_tagged (result, gdav_move), FALSE);

	task_data = g_task_get_task_data (G_TASK (result));

	/* SoupMessage is set even in case of error for uses
	 * like calling soup_message_get_https_status() when
	 * SSL/TLS negotiation fails, though SoupMessage may
	 * be NULL if the Request-URI was invalid. */
	if (out_message != NULL) {
		*out_message = task_data->message;
		task_data->message = NULL;
	}

	return g_task_propagate_boolean (G_TASK (result), error);
}

static GDavLockDiscoveryProperty *
gdav_parse_lock_response (SoupMessage *message,
                          GError **error)
{
	GDavPropertySet *propset;
	GDavLockDiscoveryProperty *property = NULL;

	g_return_val_if_fail (SOUP_IS_MESSAGE (message), NULL);

	propset = gdav_parsable_new_from_data (
		GDAV_TYPE_PROPERTY_SET,
		soup_message_get_uri (message),
		message->response_body->data,
		message->response_body->length,
		error);

	if (propset != NULL) {
		GList *list;

		list = gdav_property_set_list (
			propset, GDAV_TYPE_LOCKDISCOVERY_PROPERTY);

		if (list != NULL) {
			property = g_object_ref (list->data);
		} else {
			g_set_error_literal (
				error, GDAV_PARSABLE_ERROR,
				GDAV_PARSABLE_ERROR_CONTENT_VIOLATION,
				_("The LOCK response body was missing a "
				  "required \"lockdiscovery\" property"));
		}

		g_list_free_full (list, (GDestroyNotify) g_object_unref);

		g_object_unref (propset);
	}

	return property;
}

GDavLockDiscoveryProperty *
gdav_lock_sync (SoupSession *session,
                SoupURI *uri,
                GDavLockScope lock_scope,
                GDavLockType lock_type,
                GDavLockFlags flags,
                const gchar *owner,
                gint timeout,
                SoupMessage **out_message,
                GCancellable *cancellable,
                GError **error)
{
	SoupRequestHTTP *request;
	SoupMessage *message;
	GDavLockDiscoveryProperty *property;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri != NULL, NULL);

	request = gdav_request_lock_uri (
		session, uri, lock_scope, lock_type,
		flags, owner, timeout, error);

	if (request == NULL) {
		if (out_message != NULL)
			*out_message = NULL;
		return NULL;
	}

	message = soup_request_http_get_message (request);

	if (gdav_request_send_sync (request, cancellable, error))
		property = gdav_parse_lock_response (message, error);

	/* SoupMessage is set even in case of error for uses
	 * like calling soup_message_get_https_status() when
	 * SSL/TLS negotiation fails. */
	if (out_message != NULL)
		*out_message = g_object_ref (message);

	g_object_unref (message);
	g_object_unref (request);

	return property;
}

static void
gdav_lock_request_cb (GObject *source_object,
                      GAsyncResult *result,
                      gpointer user_data)
{
	SoupRequestHTTP *request;
	GTask *task = G_TASK (user_data);
	SoupMessage *message;
	GDavLockDiscoveryProperty *property;
	GError *local_error = NULL;

	request = SOUP_REQUEST_HTTP (source_object);
	message = soup_request_http_get_message (request);

	if (gdav_request_send_finish (request, result, &local_error))
		property = gdav_parse_lock_response (message, &local_error);

	/* Sanity check */
	g_warn_if_fail (
		((property != NULL) && (local_error == NULL)) ||
		((property == NULL) && (local_error != NULL)));

	if (property != NULL)
		g_task_return_pointer (task, property, g_object_unref);
	else
		g_task_return_error (task, local_error);

	g_object_unref (message);
	g_object_unref (task);
}

void
gdav_lock (SoupSession *session,
           SoupURI *uri,
           GDavLockScope lock_scope,
           GDavLockType lock_type,
           GDavLockFlags flags,
           const gchar *owner,
           gint timeout,
           GCancellable *cancellable,
           GAsyncReadyCallback callback,
           gpointer user_data)
{
	GTask *task;
	SoupRequestHTTP *request;
	TaskData *task_data;
	GError *local_error = NULL;

	g_return_if_fail (SOUP_IS_SESSION (session));
	g_return_if_fail (uri != NULL);

	task_data = g_slice_new0 (TaskData);

	task = g_task_new (session, cancellable, callback, user_data);
	g_task_set_source_tag (task, gdav_lock);

	g_task_set_task_data (
		task, task_data, (GDestroyNotify) task_data_free);

	request = gdav_request_lock_uri (
		session, uri, lock_scope, lock_type,
		flags, owner, timeout, &local_error);

	/* Sanity check */
	g_warn_if_fail (
		((request != NULL) && (local_error == NULL)) ||
		((request == NULL) && (local_error != NULL)));

	if (request != NULL) {
		task_data->message =
			soup_request_http_get_message (request);

		gdav_request_send (
			request, cancellable,
			gdav_lock_request_cb,
			g_object_ref (task));

		g_object_unref (request);
	} else {
		g_task_return_error (task, local_error);
	}

	g_object_unref (task);
}

GDavLockDiscoveryProperty *
gdav_lock_finish (SoupSession *session,
                  GAsyncResult *result,
                  SoupMessage **out_message,
                  GError **error)
{
	TaskData *task_data;

	g_return_val_if_fail (
		g_task_is_valid (result, session), NULL);
	g_return_val_if_fail (
		g_async_result_is_tagged (result, gdav_lock), NULL);

	task_data = g_task_get_task_data (G_TASK (result));

	/* SoupMessage is set even in case of error for uses
	 * like calling soup_message_get_https_status() when
	 * SSL/TLS negotiation fails, though SoupMessage may
	 * be NULL if the Request-URI was invalid. */
	if (out_message != NULL) {
		*out_message = task_data->message;
		task_data->message = NULL;
	}

	return g_task_propagate_pointer (G_TASK (result), error);
}

GDavLockDiscoveryProperty *
gdav_lock_refresh_sync (SoupSession *session,
                        SoupURI *uri,
                        const gchar *lock_token,
                        gint timeout,
                        SoupMessage **out_message,
                        GCancellable *cancellable,
                        GError **error)
{
	SoupRequestHTTP *request;
	SoupMessage *message;
	GDavLockDiscoveryProperty *property;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri != NULL, NULL);

	request = gdav_request_lock_refresh_uri (
		session, uri, lock_token, timeout, error);

	if (request == NULL) {
		if (out_message != NULL)
			*out_message = NULL;
		return NULL;
	}

	message = soup_request_http_get_message (request);

	if (gdav_request_send_sync (request, cancellable, error))
		property = gdav_parse_lock_response (message, error);

	/* SoupMessage is set even in case of error for uses
	 * like calling soup_message_get_https_status() when
	 * SSL/TLS negotiation fails. */
	if (out_message != NULL)
		*out_message = g_object_ref (message);

	g_object_unref (message);
	g_object_unref (request);

	return property;
}

static void
gdav_lock_refresh_request_cb (GObject *source_object,
                              GAsyncResult *result,
                              gpointer user_data)
{
	SoupRequestHTTP *request;
	GTask *task = G_TASK (user_data);
	SoupMessage *message;
	GDavLockDiscoveryProperty *property;
	GError *local_error = NULL;

	request = SOUP_REQUEST_HTTP (source_object);
	message = soup_request_http_get_message (request);

	if (gdav_request_send_finish (request, result, &local_error))
		property = gdav_parse_lock_response (message, &local_error);

	/* Sanity check */
	g_warn_if_fail (
		((property != NULL) && (local_error == NULL)) ||
		((property == NULL) && (local_error != NULL)));

	if (property != NULL)
		g_task_return_pointer (task, property, g_object_unref);
	else
		g_task_return_error (task, local_error);

	g_object_unref (message);
	g_object_unref (task);
}

void
gdav_lock_refresh (SoupSession *session,
                   SoupURI *uri,
                   const gchar *lock_token,
                   gint timeout,
                   GCancellable *cancellable,
                   GAsyncReadyCallback callback,
                   gpointer user_data)
{
	GTask *task;
	SoupRequestHTTP *request;
	TaskData *task_data;
	GError *local_error = NULL;

	g_return_if_fail (SOUP_IS_SESSION (session));
	g_return_if_fail (uri != NULL);
	g_return_if_fail (lock_token != NULL);

	task_data = g_slice_new0 (TaskData);

	task = g_task_new (session, cancellable, callback, user_data);
	g_task_set_source_tag (task, gdav_lock_refresh);

	g_task_set_task_data (
		task, task_data, (GDestroyNotify) task_data_free);

	request = gdav_request_lock_refresh_uri (
		session, uri, lock_token, timeout, &local_error);

	g_warn_if_fail (
		((request != NULL) && (local_error == NULL)) ||
		((request == NULL) && (local_error != NULL)));

	if (request != NULL) {
		task_data->message =
			soup_request_http_get_message (request);

		gdav_request_send (
			request, cancellable,
			gdav_lock_refresh_request_cb,
			g_object_ref (task));

		g_object_unref (request);
	} else {
		g_task_return_error (task, local_error);
	}

	g_object_unref (task);
}

GDavLockDiscoveryProperty *
gdav_lock_refresh_finish (SoupSession *session,
                          GAsyncResult *result,
                          SoupMessage **out_message,
                          GError **error)
{
	TaskData *task_data;

	g_return_val_if_fail (
		g_task_is_valid (result, session), NULL);
	g_return_val_if_fail (
		g_async_result_is_tagged (result, gdav_lock_refresh), NULL);

	task_data = g_task_get_task_data (G_TASK (result));

	/* SoupMessage is set even in case of error for uses
	 * like calling soup_message_get_https_status() when
	 * SSL/TLS negotiation fails, though SoupMessage may
	 * be NULL if the Request-URI was invalid. */
	if (out_message != NULL) {
		*out_message = task_data->message;
		task_data->message = NULL;
	}

	return g_task_propagate_pointer (G_TASK (result), error);
}

gboolean
gdav_unlock_sync (SoupSession *session,
                  SoupURI *uri,
                  const gchar *lock_token,
                  SoupMessage **out_message,
                  GCancellable *cancellable,
                  GError **error)
{
	SoupRequestHTTP *request;
	SoupMessage *message;
	gboolean success;

	g_return_val_if_fail (SOUP_IS_SESSION (session), FALSE);
	g_return_val_if_fail (uri != NULL, FALSE);

	request = gdav_request_unlock_uri (session, uri, lock_token, error);

	if (request == NULL) {
		if (out_message != NULL)
			*out_message = NULL;
		return FALSE;
	}

	message = soup_request_http_get_message (request);
	success = gdav_request_send_sync (request, cancellable, error);

	/* SoupMessage is set even in case of error for uses
	 * like calling soup_message_get_https_status() when
	 * SSL/TLS negotiation fails. */
	if (out_message != NULL)
		*out_message = g_object_ref (message);

	g_object_unref (message);
	g_object_unref (request);

	return success;
}

static void
gdav_unlock_request_cb (GObject *source_object,
                        GAsyncResult *result,
                        gpointer user_data)
{
	SoupRequestHTTP *request;
	GTask *task = G_TASK (user_data);
	GError *local_error = NULL;

	request = SOUP_REQUEST_HTTP (source_object);

	if (gdav_request_send_finish (request, result, &local_error))
		g_task_return_boolean (task, TRUE);
	else
		g_task_return_error (task, local_error);

	g_object_unref (task);
}

void
gdav_unlock (SoupSession *session,
             SoupURI *uri,
             const gchar *lock_token,
             GCancellable *cancellable,
             GAsyncReadyCallback callback,
             gpointer user_data)
{
	GTask *task;
	SoupRequestHTTP *request;
	TaskData *task_data;
	GError *local_error = NULL;

	g_return_if_fail (SOUP_IS_SESSION (session));
	g_return_if_fail (uri != NULL);
	g_return_if_fail (lock_token != NULL);

	task_data = g_slice_new0 (TaskData);

	task = g_task_new (session, cancellable, callback, user_data);
	g_task_set_source_tag (task, gdav_unlock);

	g_task_set_task_data (
		task, task_data, (GDestroyNotify) task_data_free);

	request = gdav_request_unlock_uri (
		session, uri, lock_token, &local_error);

	g_warn_if_fail (
		((request != NULL) && (local_error == NULL)) ||
		((request == NULL) && (local_error != NULL)));

	if (request != NULL) {
		task_data->message =
			soup_request_http_get_message (request);

		gdav_request_send (
			request, cancellable,
			gdav_unlock_request_cb,
			g_object_ref (task));

		g_object_unref (request);
	} else {
		g_task_return_error (task, local_error);
	}

	g_object_unref (task);
}

gboolean
gdav_unlock_finish (SoupSession *session,
                    GAsyncResult *result,
                    SoupMessage **out_message,
                    GError **error)
{
	TaskData *task_data;

	g_return_val_if_fail (
		g_task_is_valid (result, session), NULL);
	g_return_val_if_fail (
		g_async_result_is_tagged (result, gdav_unlock), NULL);

	task_data = g_task_get_task_data (G_TASK (result));

	/* SoupMessage is set even in case of error for uses
	 * like calling soup_message_get_https_status() when
	 * SSL/TLS negotiation fails, though SoupMessage may
	 * be NULL if the Request-URI was invalid. */
	if (out_message != NULL) {
		*out_message = task_data->message;
		task_data->message = NULL;
	}

	return g_task_propagate_boolean (G_TASK (result), error);
}


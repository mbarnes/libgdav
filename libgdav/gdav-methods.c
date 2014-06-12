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

typedef struct _AsyncContext AsyncContext;

struct _AsyncContext {
	SoupMessage *message;
	GDavAllow allow;
	GDavOptions options;
};

static void
async_context_free (AsyncContext *async_context)
{
	g_clear_object (&async_context->message);

	g_slice_free (AsyncContext, async_context);
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

	if (local_error != NULL) {
		g_task_return_error (task, local_error);
	} else {
		gdav_request_apply_response (message, output_stream);
		g_task_return_boolean (task, TRUE);
	}

	g_object_unref (message);
	g_object_unref (task);
}

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

static gboolean
gdav_request_send_sync (SoupRequestHTTP *request,
                        GCancellable *cancellable,
                        GError **error)
{
	GInputStream *input_stream;
	gssize n_bytes = -1;

	/* This is an internal wrapper for soup_request_send().
	 * The input stream contents are written to the SoupMessage
	 * response body to ensure a SoupLogger sees it. */

	input_stream = soup_request_send (
		SOUP_REQUEST (request), cancellable, error);

	if (input_stream != NULL) {
		GOutputStream *output_stream;

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
			g_object_unref (message);
		}

		g_object_unref (output_stream);
		g_object_unref (input_stream);
	}

	return (n_bytes >= 0);
}

static GDavMultiStatus *
gdav_parse_multi_status (SoupMessage *message,
                         GError **error)
{
	SoupURI *base_uri;

	g_return_val_if_fail (SOUP_IS_MESSAGE (message), NULL);

	base_uri = soup_message_get_uri (message);

	if (message->status_code != SOUP_STATUS_MULTI_STATUS) {
		g_set_error (
			error, GDAV_PARSABLE_ERROR,
			GDAV_PARSABLE_ERROR_INTERNAL,
			_("Expected status %u (%s), but got %u (%s)"),
			SOUP_STATUS_MULTI_STATUS,
			soup_status_get_phrase (SOUP_STATUS_MULTI_STATUS),
			message->status_code,
			message->reason_phrase);
		return NULL;
	}

	return gdav_parsable_new_from_data (
		GDAV_TYPE_MULTI_STATUS, base_uri,
		message->response_body->data,
		message->response_body->length,
		error);
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
	gboolean success = FALSE;

	g_return_val_if_fail (SOUP_IS_SESSION (session), FALSE);
	g_return_val_if_fail (uri != NULL, FALSE);

	request = gdav_request_options_uri (session, uri, error);

	if (request == NULL)
		return FALSE;

	message = soup_request_http_get_message (request);

	if (gdav_request_send_sync (request, cancellable, error)) {
		if (out_allow != NULL) {
			*out_allow = gdav_allow_from_headers (
				message->response_headers);
		}

		if (out_options != NULL) {
			*out_options = gdav_options_from_headers (
				message->response_headers);
		}

		success = TRUE;
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
	GTask *task = G_TASK (user_data);
	AsyncContext *async_context;
	GError *local_error = NULL;

	request = SOUP_REQUEST_HTTP (source_object);
	async_context = g_task_get_task_data (task);

	gdav_request_send_finish (request, result, &local_error);

	if (local_error == NULL) {
		async_context->allow =
			gdav_allow_from_headers (
			async_context->message->response_headers);
		async_context->options =
			gdav_options_from_headers (
			async_context->message->response_headers);
		g_task_return_boolean (task, TRUE);
	} else {
		g_task_return_error (task, local_error);
	}

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
	AsyncContext *async_context;
	GError *local_error = NULL;

	g_return_if_fail (SOUP_IS_SESSION (session));
	g_return_if_fail (uri != NULL);

	async_context = g_slice_new0 (AsyncContext);

	task = g_task_new (session, cancellable, callback, user_data);
	g_task_set_source_tag (task, gdav_options);

	g_task_set_task_data (
		task, async_context, (GDestroyNotify) async_context_free);

	request = gdav_request_options_uri (session, uri, &local_error);

	/* Sanity check */
	g_return_if_fail (
		((request != NULL) && (local_error == NULL)) ||
		((request == NULL) && (local_error != NULL)));

	if (request != NULL) {
		async_context->message =
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
	AsyncContext *async_context;

	g_return_val_if_fail (
		g_task_is_valid (result, session), FALSE);
	g_return_val_if_fail (
		g_async_result_is_tagged (result, gdav_options), FALSE);

	async_context = g_task_get_task_data (G_TASK (result));

	if (!g_task_had_error (G_TASK (result))) {
		if (out_allow != NULL)
			*out_allow = async_context->allow;
		if (out_options != NULL)
			*out_options = async_context->options;
	}

	/* SoupMessage is set even in case of error for uses
	 * like calling soup_message_get_https_status() when
	 * SSL/TLS negotiation fails, though SoupMessage may
	 * be NULL if the Request-URI was invalid. */
	if (out_message != NULL) {
		*out_message = async_context->message;
		async_context->message = NULL;
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

	if (request == NULL)
		return NULL;

	message = soup_request_http_get_message (request);

	if (gdav_request_send_sync (request, cancellable, error))
		multi_status = gdav_parse_multi_status (message, error);

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
	gpointer parsable;
	GError *local_error = NULL;

	request = SOUP_REQUEST_HTTP (source_object);
	message = soup_request_http_get_message (request);

	gdav_request_send_finish (request, result, &local_error);

	if (local_error != NULL)
		goto exit;

	parsable = gdav_parse_multi_status (message, &local_error);

	/* Sanity check */
	g_warn_if_fail (
		((parsable != NULL) && (local_error == NULL)) ||
		((parsable == NULL) && (local_error != NULL)));

	if (parsable != NULL) {
		g_warn_if_fail (GDAV_IS_MULTI_STATUS (parsable));
		g_task_return_pointer (task, parsable, g_object_unref);
	}

exit:
	if (local_error != NULL)
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
	AsyncContext *async_context;
	GError *local_error = NULL;

	g_return_if_fail (SOUP_IS_SESSION (session));
	g_return_if_fail (uri != NULL);

	async_context = g_slice_new0 (AsyncContext);

	task = g_task_new (session, cancellable, callback, user_data);
	g_task_set_source_tag (task, gdav_propfind);

	g_task_set_task_data (
		task, async_context, (GDestroyNotify) async_context_free);

	request = gdav_request_propfind_uri (
		session, uri, type, prop, depth, &local_error);

	/* Sanity check */
	g_warn_if_fail (
		((request != NULL) && (local_error == NULL)) ||
		((request == NULL) && (local_error != NULL)));

	if (request != NULL) {
		async_context->message =
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
	AsyncContext *async_context;

	g_return_val_if_fail (
		g_task_is_valid (result, session), NULL);
	g_return_val_if_fail (
		g_async_result_is_tagged (result, gdav_propfind), FALSE);

	async_context = g_task_get_task_data (G_TASK (result));

	/* SoupMessage is set even in case of error for uses
	 * like calling soup_message_get_https_status() when
	 * SSL/TLS negotiation fails, though SoupMessage may
	 * be NULL if the Request-URI was invalid. */
	if (out_message != NULL) {
		*out_message = async_context->message;
		async_context->message = NULL;
	}

	return g_task_propagate_pointer (G_TASK (result), error);
}


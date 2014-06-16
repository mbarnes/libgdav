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
gdav_request_splice_cb (GObject *source_object,
                        GAsyncResult *result,
                        gpointer user_data)
{
	SoupMessage *message;
	GTask *task = G_TASK (user_data);
	GError *local_error = NULL;

	message = g_task_get_task_data (task);

	g_output_stream_splice_finish (
		G_OUTPUT_STREAM (source_object), result, &local_error);

	if (local_error != NULL) {
		g_task_return_error (task, local_error);

	/* XXX That the input stream's content is not automatically
	 *     copied to the SoupMessage's response_body is a known
	 *     libsoup bug which may be fixed in a future release.
	 *     Check that the response body is empty so we don't
	 *     accidentally duplicate the body. */
	} else if (message->response_body->data == NULL) {
		GMemoryOutputStream *output_stream;
		gpointer data;
		gsize size;

		output_stream = G_MEMORY_OUTPUT_STREAM (source_object);
		size = g_memory_output_stream_get_data_size (output_stream);
		data = g_memory_output_stream_steal_data (output_stream);

		soup_message_body_append_take (
			message->response_body, data, size);
		soup_message_body_flatten (message->response_body);
		soup_message_finished (message);

		g_task_return_boolean (task, TRUE);
	} else {
		g_task_return_boolean (task, TRUE);
	}

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

		cancellable = g_task_get_cancellable (task);

		output_stream = g_memory_output_stream_new_resizable ();

		/* Don't close the input stream here, we'll
		 * need to perform some post-processing first. */
		g_output_stream_splice_async (
			output_stream, input_stream,
			G_OUTPUT_STREAM_SPLICE_CLOSE_TARGET,
			G_PRIORITY_DEFAULT, cancellable,
			gdav_request_splice_cb,
			g_object_ref (task));

		g_object_unref (output_stream);
	}

	if (local_error != NULL) {
		g_task_return_error (task, local_error);
	}

	g_object_unref (task);
}

static void
gdav_request_send (SoupRequestHTTP *request,
                   GCancellable *cancellable,
                   GAsyncReadyCallback callback,
                   gpointer user_data)
{
	GTask *task;
	SoupMessage *message;

	/* This is an internal wrapper for soup_request_send_async().
	 * The input stream contents are written to the SoupMessage
	 * response body to ensure a SoupLogger sees it. */

	task = g_task_new (request, cancellable, callback, user_data);

	message = soup_request_http_get_message (request);
	g_task_set_task_data (task, message, g_object_unref);

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
	GDavAsyncClosure *closure;
	GAsyncResult *result;
	gboolean success;

	g_return_val_if_fail (SOUP_IS_SESSION (session), FALSE);
	g_return_val_if_fail (uri != NULL, FALSE);

	closure = gdav_async_closure_new ();

	gdav_options (
		session, uri, cancellable,
		gdav_async_closure_callback, closure);

	result = gdav_async_closure_wait (closure);

	success = gdav_options_finish (
		session, result, out_allow,
		out_options, out_message, error);

	gdav_async_closure_free (closure);

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
			gdav_options_from_headers (
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
	GDavAsyncClosure *closure;
	GAsyncResult *result;
	GDavMultiStatus *multi_status;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri != NULL, NULL);

	closure = gdav_async_closure_new ();

	gdav_propfind (
		session, uri, type, prop, depth, cancellable,
		gdav_async_closure_callback, closure);

	result = gdav_async_closure_wait (closure);

	multi_status = gdav_propfind_finish (
		session, result, out_message, error);

	gdav_async_closure_free (closure);

	return multi_status;
}

static void
gdav_propfind_request_cb (GObject *source_object,
                          GAsyncResult *result,
                          gpointer user_data)
{
	SoupRequestHTTP *request;
	GTask *task = G_TASK (user_data);
	SoupURI *base_uri;
	gpointer parsable;
	guint status_code;
	AsyncContext *async_context;
	GError *local_error = NULL;

	request = SOUP_REQUEST_HTTP (source_object);
	async_context = g_task_get_task_data (task);
	status_code = async_context->message->status_code;

	gdav_request_send_finish (request, result, &local_error);

	if (local_error != NULL)
		goto exit;

	if (status_code != SOUP_STATUS_MULTI_STATUS) {
		g_task_return_new_error (
			task, GDAV_PARSABLE_ERROR,
			GDAV_PARSABLE_ERROR_INTERNAL,
			_("Expected status %u (%s), but got (%u) (%s)"),
			SOUP_STATUS_MULTI_STATUS,
			soup_status_get_phrase (SOUP_STATUS_MULTI_STATUS),
			async_context->message->status_code,
			async_context->message->reason_phrase);
		goto exit;
	}

	base_uri = soup_message_get_uri (async_context->message);

	parsable = gdav_parsable_new_from_data (
		GDAV_TYPE_MULTI_STATUS, base_uri,
		async_context->message->response_body->data,
		async_context->message->response_body->length,
		&local_error);

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


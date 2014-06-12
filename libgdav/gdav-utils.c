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

#include "gdav-utils.h"

#include <glib/gi18n-lib.h>

#include "gdav-parsable.h"

static gpointer
init_multi_status_is_error (gpointer unused)
{
	GHashTable *hash_table;

	hash_table = g_hash_table_new (NULL, NULL);

	/* A 207 Multi-Status response for these
	 * methods indicates a partial failure. */

	g_hash_table_add (hash_table, (gpointer) SOUP_METHOD_DELETE);
	g_hash_table_add (hash_table, (gpointer) SOUP_METHOD_COPY);
	g_hash_table_add (hash_table, (gpointer) SOUP_METHOD_MOVE);
	g_hash_table_add (hash_table, (gpointer) SOUP_METHOD_LOCK);

	return hash_table;
}

static gboolean
multi_status_is_error (const gchar *method)
{
	static GOnce multi_status_is_error = G_ONCE_INIT;

	g_return_val_if_fail (method != NULL, FALSE);

	/* Note, the method string must be the canonical representation
	 * by way of g_intern_string().  SoupMessage's method string is
	 * already canonicalized. */

	g_once (&multi_status_is_error, init_multi_status_is_error, NULL);

	return g_hash_table_contains (multi_status_is_error.retval, method);
}

GDavAllow
gdav_allow_from_headers (SoupMessageHeaders *headers)
{
	GDavAllow allow = 0;
	const gchar *hdr;

	g_return_val_if_fail (headers != NULL, 0);

	hdr = soup_message_headers_get_list (headers, "Allow");

	if (hdr != NULL) {
		if (soup_header_contains (hdr, "ACL"))
			allow |= GDAV_ALLOW_ACL;

		if (soup_header_contains (hdr, "COPY"))
			allow |= GDAV_ALLOW_COPY;

		if (soup_header_contains (hdr, "DELETE"))
			allow |= GDAV_ALLOW_DELETE;

		if (soup_header_contains (hdr, "GET"))
			allow |= GDAV_ALLOW_GET;

		if (soup_header_contains (hdr, "HEAD"))
			allow |= GDAV_ALLOW_HEAD;

		if (soup_header_contains (hdr, "LOCK"))
			allow |= GDAV_ALLOW_LOCK;

		if (soup_header_contains (hdr, "MKCALENDAR"))
			allow |= GDAV_ALLOW_MKCALENDAR;

		if (soup_header_contains (hdr, "MKCOL"))
			allow |= GDAV_ALLOW_MKCOL;

		if (soup_header_contains (hdr, "MOVE"))
			allow |= GDAV_ALLOW_MOVE;

		if (soup_header_contains (hdr, "OPTIONS"))
			allow |= GDAV_ALLOW_OPTIONS;

		if (soup_header_contains (hdr, "POST"))
			allow |= GDAV_ALLOW_POST;

		if (soup_header_contains (hdr, "PROPFIND"))
			allow |= GDAV_ALLOW_PROPFIND;

		if (soup_header_contains (hdr, "PROPPATCH"))
			allow |= GDAV_ALLOW_PROPPATCH;

		if (soup_header_contains (hdr, "PUT"))
			allow |= GDAV_ALLOW_PUT;

		if (soup_header_contains (hdr, "REPORT"))
			allow |= GDAV_ALLOW_REPORT;

		if (soup_header_contains (hdr, "UNLOCK"))
			allow |= GDAV_ALLOW_UNLOCK;
	}

	return allow;
}

GDavOptions
gdav_options_from_headers (SoupMessageHeaders *headers)
{
	GDavOptions options = 0;
	const gchar *hdr;

	g_return_val_if_fail (headers != NULL, 0);

	hdr = soup_message_headers_get_list (headers, "DAV");

	if (hdr != NULL) {
		if (soup_header_contains (hdr, "1"))
			options |= GDAV_OPTIONS_COMPLIANCE_CLASS_1;

		if (soup_header_contains (hdr, "2"))
			options |= GDAV_OPTIONS_COMPLIANCE_CLASS_2;

		if (soup_header_contains (hdr, "3"))
			options |= GDAV_OPTIONS_COMPLIANCE_CLASS_3;

		if (soup_header_contains (hdr, "access-control"))
			options |= GDAV_OPTIONS_ACCESS_CONTROL;

		if (soup_header_contains (hdr, "redirectrefs"))
			options |= GDAV_OPTIONS_REDIRECT_REFS;

		if (soup_header_contains (hdr, "version-control"))
			options |= GDAV_OPTIONS_VERSION_CONTROL;

		if (soup_header_contains (hdr, "calendar-access"))
			options |= GDAV_OPTIONS_CALENDAR_ACCESS;

		if (soup_header_contains (hdr, "calendar-schedule"))
			options |= GDAV_OPTIONS_CALENDAR_SCHEDULE;

		if (soup_header_contains (hdr, "calendar-auto-schedule"))
			options |= GDAV_OPTIONS_CALENDAR_AUTO_SCHEDULE;

		if (soup_header_contains (hdr, "calendar-proxy"))
			options |= GDAV_OPTIONS_CALENDAR_PROXY;

		if (soup_header_contains (hdr, "addressbook"))
			options |= GDAV_OPTIONS_ADDRESSBOOK;
	}

	return options;
}

gboolean
gdav_message_is_successful (SoupMessage *message,
                            GError **error)
{
	guint status_code;
	gboolean success;

	g_return_val_if_fail (SOUP_IS_MESSAGE (message), FALSE);

	/* XXX This doesn't take into account informational (1xx)
	 *     status codes, but I think libsoup handles those
	 *     automatically so it shouldn't be an issue. */

	status_code = message->status_code;
	success = SOUP_STATUS_IS_SUCCESSFUL (status_code);

	/* For certain methods a 207 Multi-Status response indicates
	 * a partial failure, which this function treats as an error. */
	if (status_code == SOUP_STATUS_MULTI_STATUS) {
		if (multi_status_is_error (message->method))
			success = FALSE;
	}

	if (!success) {
		const gchar *reason_phrase;

		/* XXX Does libsoup already ensure this is set? */
		if (message->reason_phrase != NULL)
			reason_phrase = message->reason_phrase;
		else
			reason_phrase = soup_status_get_phrase (status_code);

		g_set_error (
			error, SOUP_HTTP_ERROR, status_code,
			"%d %s", status_code, reason_phrase);
	}

	return success;
}

static gchar *
gdav_print_element (xmlNode *element)
{
	GString *string;

	/* XXX Probably oversimplified, but good enough for now. */

	string = g_string_sized_new (64);

	g_string_append_c (string, '<');

	if (element->ns != NULL && element->ns->prefix != NULL) {
		g_string_append (string, (gchar *) element->ns->prefix);
		g_string_append_c (string, ':');
	}

	g_string_append (string, (gchar *) element->name);
	g_string_append_c (string, '>');

	return g_string_free (string, FALSE);
}

gboolean
gdav_error_missing_content (xmlNode *element,
                            GError **error)
{
	gchar *element_string;

	g_return_val_if_fail (element != NULL, FALSE);

	element_string = gdav_print_element (element);

	g_set_error (
		error, GDAV_PARSABLE_ERROR,
		GDAV_PARSABLE_ERROR_CONTENT_VIOLATION,
		_("A %s element was missing required content"),
		element_string);

	g_free (element_string);

	return FALSE;
}

gboolean
gdav_error_unknown_content (xmlNode *element,
                            const gchar *actual_content,
                            GError **error)
{
	gchar *element_string;

	g_return_val_if_fail (element != NULL, FALSE);
	g_return_val_if_fail (actual_content != NULL, FALSE);

	element_string = gdav_print_element (element);

	g_set_error (
		error, GDAV_PARSABLE_ERROR,
		GDAV_PARSABLE_ERROR_CONTENT_VIOLATION,
		_("The content of a %s element (\"%s\") was unknown"),
		element_string, actual_content);

	g_free (element_string);

	return FALSE;
}


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


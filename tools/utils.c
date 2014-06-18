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

#include "utils.h"

#include <ctype.h>
#include <stdio.h>
#include <unistd.h>

#include <glib/gi18n.h>

void
open_connection (GlobalState *state,
                 const gchar *uri_string)
{
	SoupURI *uri;
	SoupMessage *message = NULL;
	GError *local_error = NULL;

	g_return_if_fail (state != NULL);
	g_return_if_fail (uri_string != NULL);

	close_connection (state);

	uri = soup_uri_new (uri_string);

	if (uri == NULL) {
		g_print (_("Could not parse URI '%s'\n"), uri_string);
		return;
	}

retry:
	gdav_options_sync (
		state->session, uri,
		&state->allow,
		&state->options,
		&message,
		NULL, &local_error);

	if (accept_bad_certificate (message)) {
		g_clear_object (&message);
		g_clear_error (&local_error);
		soup_session_abort (state->session);

		g_object_set (
			state->session,
			SOUP_SESSION_SSL_STRICT, FALSE, NULL);

		goto retry;
	}

	if (local_error == NULL) {
		state->base_uri = uri;  /* takes ownership */
		state->connected = TRUE;

		/* This may alter the URI's path. */
		if (!set_path (state, uri))
			close_connection (state);
	} else {
		print_error (local_error);
		g_error_free (local_error);
		soup_uri_free (uri);
	}

	g_clear_object (&message);
}

void
close_connection (GlobalState *state)
{
	if (state->base_uri != NULL) {
		g_print (
			_("Connection to '%s' closed.\n"),
			state->base_uri->host);
		soup_uri_free (state->base_uri);
		state->base_uri = NULL;
	}

	if (state->last_uri != NULL) {
		soup_uri_free (state->last_uri);
		state->last_uri = NULL;
	}

	soup_session_abort (state->session);

	g_object_set (
		state->session,
		SOUP_SESSION_SSL_STRICT, TRUE, NULL);

	state->connected = FALSE;
	state->allow = 0;
	state->options = 0;
}

gboolean
set_path (GlobalState *state,
          SoupURI *uri)
{
	GDavResourceType resource_type;
	GError *local_error = NULL;

	g_return_val_if_fail (state != NULL, FALSE);
	g_return_val_if_fail (uri != NULL, FALSE);

	/* This may alter the URI's path. */
	resource_type = get_resource_type (state->session, uri, &local_error);

	if (local_error != NULL) {
		g_print (
			_("Could not access %s (not WebDAV-enabled?):"),
			uri->path);
		g_print ("\n%s\n", local_error->message);
		g_error_free (local_error);
		state->isdav = FALSE;

	} else if ((resource_type & GDAV_RESOURCE_TYPE_COLLECTION) == 0) {
		g_print (
			_("Could not access %s (not a collection)"),
			uri->path);
		g_print ("\n");
		state->isdav = FALSE;

	} else {
		state->isdav = TRUE;
	}

	return state->isdav;
}

static gboolean
yesno (void)
{
	gchar buf[128];
	gint len = 0;
	gchar c;

	while ((c = getchar ()) != EOF && c != '\n') {
		if ((len > 0 && len < 127) || (len == 0 && !isspace (c)))
			buf[len++] = c;
	}
	buf[len] = '\0';

	return (buf[0] == 'y' || buf[0] == 'Y');
}

gboolean
accept_bad_certificate (SoupMessage *message)
{
	GTlsCertificateFlags errors;
	gboolean accept_it = FALSE;

	g_return_val_if_fail (SOUP_IS_MESSAGE (message), FALSE);

	if (message->status_code != SOUP_STATUS_SSL_FAILED)
		return FALSE;

	if (!soup_message_get_https_status (message, NULL, &errors))
		return FALSE;

	g_print ("%s\n", _("WARNING: Untrusted server certificate presented:"));

	/* XXX These are copied straight out of GTlsCertificateFlags docs.
	 *     Lame, I know. */

	if (errors & G_TLS_CERTIFICATE_UNKNOWN_CA)
		g_print ("* %s\n", _("The signing certificate authority is not known."));

	if (errors & G_TLS_CERTIFICATE_BAD_IDENTITY)
		g_print ("* %s\n", _("The certificate does not match the expected identity of the site that it was retrieved from."));

	if (errors & G_TLS_CERTIFICATE_NOT_ACTIVATED)
		g_print ("* %s\n", _("The certificate's activation time is still in the future."));

	if (errors & G_TLS_CERTIFICATE_EXPIRED)
		g_print ("* %s\n", _("The certificate has expired."));

	if (errors & G_TLS_CERTIFICATE_REVOKED)
		g_print ("* %s\n", _("The certificate has been revoked."));

	if (errors & G_TLS_CERTIFICATE_INSECURE)
		g_print ("* %s\n", _("The certificate's algorithm is considered insecure."));

	if (isatty (STDIN_FILENO)) {
		g_print (_("Do you wish to accept the certificate? (y/n) "));
		accept_it = yesno ();
	} else {
		g_print ("%s\n", _("Certificate rejected."));
	}

	return accept_it;
}

void
print_error (GError *error)
{
	if (error != NULL) {
		if (error->domain == SOUP_HTTP_ERROR)
			g_print ("(%d) %s\n", error->code, error->message);
		else
			g_print ("%s\n", error->message);
	}
}

GDavResourceType
get_resource_type (SoupSession *session,
                   SoupURI *uri,
                   GError **error)
{
	GDavPropertySet *prop;
	GDavMultiStatus *multi_status;
	GDavResponse *response;
	GDavResourceType resource_type = 0;
	GValue value = G_VALUE_INIT;
	GList *hrefs;
	guint status;

	g_return_val_if_fail (SOUP_IS_SESSION (session), 0);
	g_return_val_if_fail (uri != NULL, 0);

	prop = gdav_property_set_new ();
	gdav_property_set_add_type (prop, GDAV_TYPE_RESOURCETYPE_PROPERTY);

	multi_status = gdav_propfind_sync (
		session, uri, GDAV_PROPFIND_PROP, prop,
		GDAV_DEPTH_0, NULL, NULL, error);

	g_object_unref (prop);

	if (multi_status == NULL)
		return 0;

	response = gdav_multi_status_get_response (multi_status, 0);
	g_return_val_if_fail (response != NULL, 0);

	/* Canonicalize the URI path according to the response. */
	hrefs = gdav_response_list_hrefs (response);
	if (hrefs != NULL) {
		const gchar *path;
		path = soup_uri_get_path (hrefs->data);
		soup_uri_set_path (uri, path);
	}

	status = gdav_response_find_property (
		response, GDAV_TYPE_RESOURCETYPE_PROPERTY, &value, NULL);

	if (status == SOUP_STATUS_OK) {
		resource_type = g_value_get_flags (&value);
		g_value_unset (&value);
	}

	g_object_unref (multi_status);

	return resource_type;
}

/* Helper for get_property_type() */
static GType
get_property_type_rec (GType parent_type,
                       const gchar *element_name,
                       const gchar *element_namespace)
{
	GType *children;
	guint n_children, ii;
	GType type = G_TYPE_INVALID;

	children = g_type_children (parent_type, &n_children);

	for (ii = 0; ii < n_children; ii++) {
		GDavParsableClass *class;
		GType child_type;
		gboolean match = FALSE;

		child_type = children[ii];

		/* Recurse over the child's children. */
		type = get_property_type_rec (
			child_type, element_name, element_namespace);

		if (type != G_TYPE_INVALID)
			break;

		if (G_TYPE_IS_ABSTRACT (child_type))
			continue;

		class = g_type_class_ref (child_type);

		/* element_namespace can be NULL. */
		match = (class->element_name != NULL) &&
			(class->element_namespace != NULL) &&
			g_str_equal (element_name, class->element_name) &&
			(element_namespace == NULL ||
			 g_str_equal (
				element_namespace,
				class->element_namespace));

		g_type_class_unref (class);

		if (match) {
			type = child_type;
			break;
		}
	}

	g_free (children);

	return type;
}

GType
get_property_type (const gchar *prop_name)
{
	const gchar *colon;
	const gchar *xmlns = NULL;

	g_return_val_if_fail (prop_name != NULL, 0);

	colon = strchr (prop_name, ':');
	if (colon != NULL) {
		gchar *xmlns_prefix;

		xmlns_prefix = g_strndup (prop_name, colon - prop_name);
		prop_name = colon + 1;

		xmlns = gdav_xmlns_from_prefix (xmlns_prefix);

		g_free (xmlns_prefix);

		/* Return invalid for an unknown prefix. */
		if (xmlns == NULL)
			return G_TYPE_INVALID;
	}

	/* XXX We could almost use gdav_parsable_lookup_type() except that
	 *     requires a namespace and we want to try to cope without one.
	 *     Also I don't want to deal with xmlNode from here. */
	return get_property_type_rec (GDAV_TYPE_PROPERTY, prop_name, xmlns);
}

static gint
compare_resources (gconstpointer a,
                   gconstpointer b,
                   gpointer unused)
{
	const Resource *resource_a = a;
	const Resource *resource_b = b;

	/* Sort errors first, then collections, then alphabetically. */

	if (!SOUP_STATUS_IS_SUCCESSFUL (resource_a->status))
		return -1;

	if (!SOUP_STATUS_IS_SUCCESSFUL (resource_b->status))
		return 1;

	if (resource_a->type & GDAV_RESOURCE_TYPE_COLLECTION) {
		if (resource_b->type & GDAV_RESOURCE_TYPE_COLLECTION) {
			return g_strcmp0 (
				resource_a->href->path,
				resource_b->href->path);
		} else {
			return -1;
		}
	}

	if (resource_b->type & GDAV_RESOURCE_TYPE_COLLECTION) {
		return 1;
	} else {
		return g_strcmp0 (
			resource_a->href->path,
			resource_b->href->path);
	}
}

static void
process_response (GDavResponse *response,
                  GQueue *out_resources)
{
	GList *list, *link;

	list = gdav_response_list_hrefs (response);

	for (link = list; link != NULL; link = g_list_next (link)) {
		Resource *resource;
		GType property_type;
		GValue value = G_VALUE_INIT;
		gchar *reason_phrase = NULL;

		resource = g_slice_new0 (Resource);
		resource->href = (SoupURI *) link->data;
		link->data = NULL;

		g_queue_push_tail (out_resources, resource);

		/* For the purpose of listing resources, if any
		 * error is encountered then the whole resource
		 * indicates failure. */

		property_type = GDAV_TYPE_RESOURCETYPE_PROPERTY;
		resource->status = gdav_response_find_property (
			response, property_type, &value, &reason_phrase);
		if (SOUP_STATUS_IS_SUCCESSFUL (resource->status)) {
			resource->type = g_value_get_flags (&value);
		} else {
			resource->reason_phrase = reason_phrase;
			continue;
		}

		g_value_unset (&value);
		g_free (reason_phrase);

		property_type = GDAV_TYPE_GETCONTENTLENGTH_PROPERTY;
		resource->status = gdav_response_find_property (
			response, property_type, &value, &reason_phrase);
		if (SOUP_STATUS_IS_SUCCESSFUL (resource->status)) {
			resource->content_length = g_value_get_uint64 (&value);
		} else {
			resource->reason_phrase = reason_phrase;
			continue;
		}

		g_value_unset (&value);
		g_free (reason_phrase);

		property_type = GDAV_TYPE_GETLASTMODIFIED_PROPERTY;
		resource->status = gdav_response_find_property (
			response, property_type, &value, &reason_phrase);
		if (SOUP_STATUS_IS_SUCCESSFUL (resource->status)) {
			resource->last_modified = g_value_dup_boxed (&value);
		} else {
			resource->reason_phrase = reason_phrase;
			continue;
		}

		g_value_unset (&value);
		g_free (reason_phrase);
	}

	g_list_free (list);

}

gboolean
get_resource_list (SoupSession *session,
                   SoupURI *uri,
                   GDavDepth depth,
                   GQueue *out_resources,
                   GError **error)
{
	GDavPropertySet *prop;
	GDavMultiStatus *multi_status;
	guint ii, n_responses;

	g_return_if_fail (SOUP_IS_SESSION (session));
	g_return_if_fail (uri != NULL);
	g_return_if_fail (out_resources != NULL);

	prop = gdav_property_set_new ();
	gdav_property_set_add_type (prop, GDAV_TYPE_RESOURCETYPE_PROPERTY);
	gdav_property_set_add_type (prop, GDAV_TYPE_GETCONTENTLENGTH_PROPERTY);
	gdav_property_set_add_type (prop, GDAV_TYPE_GETLASTMODIFIED_PROPERTY);

	multi_status = gdav_propfind_sync (
		session, uri, GDAV_PROPFIND_PROP, prop,
		GDAV_DEPTH_1, NULL, NULL, error);

	g_object_unref (prop);

	if (multi_status == NULL)
		return FALSE;

	n_responses = gdav_multi_status_get_n_responses (multi_status);

	for (ii = 0; ii < n_responses; ii++) {
		GDavResponse *response;

		response = gdav_multi_status_get_response (multi_status, ii);
		process_response (response, out_resources);
	}

	g_object_unref (multi_status);

	g_queue_sort (out_resources, compare_resources, NULL);

	return TRUE;
}

void
free_resource (Resource *resource)
{
	if (resource != NULL) {
		if (resource->href != NULL)
			soup_uri_free (resource->href);

		g_free (resource->reason_phrase);

		if (resource->last_modified != NULL)
			soup_date_free (resource->last_modified);

		g_slice_free (Resource, resource);
	}
}

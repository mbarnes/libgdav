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

#include "gdav-requests.h"

#define XC_ALLPROP		(BAD_CAST "allprop")
#define XC_EXCLUSIVE		(BAD_CAST "exclusive")
#define XC_HREF			(BAD_CAST "href")
#define XC_LOCKINFO		(BAD_CAST "lockinfo")
#define XC_LOCKSCOPE		(BAD_CAST "lockscope")
#define XC_LOCKTYPE		(BAD_CAST "locktype")
#define XC_MKREDIRECTREF	(BAD_CAST "mkredirectref")
#define XC_OWNER		(BAD_CAST "owner")
#define XC_PERMANENT		(BAD_CAST "permanent")
#define XC_PROPERTYUPDATE	(BAD_CAST "propertyupdate")
#define XC_PROPFIND		(BAD_CAST "propfind")
#define XC_PROPNAME		(BAD_CAST "propname")
#define XC_REDIRECT_LIFETIME	(BAD_CAST "redirect-lifetime")
#define XC_REFTARGET		(BAD_CAST "reftarget")
#define XC_SHARED		(BAD_CAST "shared")
#define XC_TEMPORARY		(BAD_CAST "temporary")
#define XC_UPDATEREDIRECTREF	(BAD_CAST "updateredirectref")
#define XC_WRITE		(BAD_CAST "write")

static xmlNs *
gdav_nsdav_new (xmlNode *root)
{
	const gchar *prefix;

	prefix = gdav_get_xmlns_prefix (GDAV_XMLNS_DAV);
	g_warn_if_fail (prefix != NULL);

	return xmlNewNs (root, BAD_CAST GDAV_XMLNS_DAV, BAD_CAST prefix);
}

static GHashTable *
gdav_namespaces_ht_new (xmlNode *root)
{
	GHashTable *namespaces;
	xmlNs *ns;

	namespaces = g_hash_table_new (g_str_hash, g_str_equal);

	/* Ensure the "DAV:" namespace is always present. */
	ns = gdav_nsdav_new (root);
	g_hash_table_replace (namespaces, (gpointer) ns->href, ns);

	return namespaces;
}

/* Helper for gdav_namespaces_ht_collect() */
static void
gdav_maybe_add_namespace (GHashTable *namespaces,
                          GDavParsableClass *class,
                          xmlNode *root)
{
	xmlNs *ns;
	const gchar *ns_href;
	const gchar *ns_prefix;

	ns_href = class->element_namespace;

	if (ns_href == NULL) {
		g_warning (
			"No element_namespace in %sClass",
			G_OBJECT_CLASS_NAME (class));
		return;
	}

	if (g_hash_table_contains (namespaces, ns_href))
		return;

	ns_prefix = gdav_get_xmlns_prefix (ns_href);

	if (ns_prefix == NULL) {
		g_warning ("No prefix for namespace '%s'", ns_href);
		return;
	}

	ns = xmlNewNs (root, BAD_CAST ns_href, BAD_CAST ns_prefix);
	g_hash_table_replace (namespaces, (gpointer) ns->href, ns);
}

static void
gdav_namespaces_ht_collect (GHashTable *namespaces,
                            GDavParsable *parsable,
                            xmlNode *root)
{
	GHashTable *parsable_types;
	GHashTableIter iter;
	gpointer key;

	/* XXX Memory management of xmlNs structures is confusing.
	 *     The libxml2 docs don't say so, but xmlNode functions
	 *     apparently take ownership of xmlNs arguments.  So WE
	 *     allocate them and xmlNode frees them.  Therefore, we
	 *     only want to allocate xmlNs structures we know we'll
	 *     use per HTTP request so we don't leak memory. */

	parsable_types = g_hash_table_new (NULL, NULL);
	gdav_parsable_collect_types (parsable, parsable_types);

	g_hash_table_iter_init (&iter, parsable_types);

	while (g_hash_table_iter_next (&iter, &key, NULL)) {
		GType type = GPOINTER_TO_SIZE (key);
		GDavParsableClass *class;

		if (!g_type_is_a (type, GDAV_TYPE_PARSABLE)) {
			g_warning (
				"Non-parsable type %s collected from %s",
				g_type_name (type),
				G_OBJECT_TYPE_NAME (parsable));
			continue;
		}

		class = g_type_class_ref (type);
		gdav_maybe_add_namespace (namespaces, class, root);
		g_type_class_unref (class);
	}

	g_hash_table_destroy (parsable_types);
}

static void
gdav_init_basic_request (SoupRequestHTTP *request)
{
	SoupMessage *message;

	message = soup_request_http_get_message (request);

	/* Add headers common to all requests. */

	/* See RFC 4918 (WebDAV) Section 10.4.5 */
	soup_message_headers_replace (
		message->request_headers,
		"Cache-Control", "no-cache");
	soup_message_headers_replace (
		message->request_headers,
		"Pragma", "no-cache");

	g_object_unref (message);
}

static void
gdav_request_headers_add_depth (SoupMessage *message,
                                GDavDepth depth)
{
	switch (depth) {
		case GDAV_DEPTH_0:
			soup_message_headers_replace (
				message->request_headers, "Depth", "0");
			break;
		case GDAV_DEPTH_1:
			soup_message_headers_replace (
				message->request_headers, "Depth", "1");
			break;
		case GDAV_DEPTH_INFINITY:
			soup_message_headers_replace (
				message->request_headers, "Depth", "infinity");
			break;
		default:
			g_warn_if_reached ();
	}
}

static void
gdav_request_headers_add_destination (SoupMessage *message,
                                      const gchar *destination)
{
	soup_message_headers_replace (
		message->request_headers, "Destination", destination);
}

static void
gdav_request_headers_add_lock_token (SoupMessage *message,
                                     const gchar *lock_token)
{
	gchar *coded_url;

	coded_url = g_strdup_printf ("<%s>", lock_token);
	soup_message_headers_replace (
		message->request_headers, "Lock-Token", coded_url);
	g_free (coded_url);
}

static void
gdav_request_headers_add_overwrite (SoupMessage *message,
                                    gboolean overwrite)
{
	soup_message_headers_replace (
		message->request_headers,
		"Overwrite", overwrite ? "T" : "F");
}

static void
gdav_request_headers_add_timeout (SoupMessage *message,
                                  gint timeout)
{
	if (timeout < 0) {
		soup_message_headers_replace (
			message->request_headers,
			"Timeout", "Infinite");
	} else {
		gchar *seconds;

		timeout = CLAMP (timeout, 0, G_MAXINT32);
		seconds = g_strdup_printf ("Second-%d", timeout);
		soup_message_headers_replace (
			message->request_headers,
			"Timeout", seconds);
		g_free (seconds);
	}
}

static void
gdav_request_write_reftarget (SoupURI *target,
                              xmlNode *parent,
                              xmlNs *nsdav)
{
	if (target != NULL) {
		xmlNode *node;
		gchar *uri_string;

		node = xmlNewTextChild (parent, nsdav, XC_REFTARGET, NULL);

		uri_string = soup_uri_to_string (target, FALSE);
		xmlNewTextChild (node, nsdav, XC_HREF, BAD_CAST uri_string);
		g_free (uri_string);
	}
}

static void
gdav_request_write_redirect_lifetime (GDavRedirectLifetime lifetime,
                                      xmlNode *parent,
                                      xmlNs *nsdav)
{
	xmlNode *node;
	xmlChar *name = NULL;

	switch (lifetime) {
		case GDAV_REDIRECT_LIFETIME_PERMANENT:
			name = XC_PERMANENT;
			break;
		case GDAV_REDIRECT_LIFETIME_TEMPORARY:
			name = XC_TEMPORARY;
			break;
		default:
			/* leave it unspecified */
			return;
	}

	node = xmlNewTextChild (parent, nsdav, XC_REDIRECT_LIFETIME, NULL);
	xmlNewTextChild (node, nsdav, name, NULL);
}

static void
gdav_request_write_body (SoupMessage *message,
                         xmlDoc *doc,
                         xmlNode *root)
{
	xmlOutputBuffer *buffer;
	gconstpointer content;
	gsize size;

	buffer = xmlAllocOutputBuffer (NULL);
	xmlNodeDumpOutput (buffer, doc, root, 0, 1, NULL);
	xmlOutputBufferFlush (buffer);

#ifdef LIBXML2_NEW_BUFFER
	content = xmlOutputBufferGetContent (buffer);
	size = xmlOutputBufferGetSize (buffer);
#else
	content = buffer->buffer->content;
	size = buffer->buffer->use;
#endif

	soup_message_set_request (
		message, "application/xml", SOUP_MEMORY_COPY, content, size);

	xmlOutputBufferClose (buffer);
}

SoupRequestHTTP *
gdav_request_options (SoupSession *session,
                      const gchar *uri_string,
                      GError **error)
{
	SoupRequestHTTP *request;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri_string != NULL, NULL);

	request = soup_session_request_http (
		session, SOUP_METHOD_OPTIONS, uri_string, error);

	if (request != NULL)
		gdav_init_basic_request (request);

	return request;
}

SoupRequestHTTP *
gdav_request_options_uri (SoupSession *session,
                          SoupURI *uri,
                          GError **error)
{
	SoupRequestHTTP *request;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri != NULL, NULL);

	request = soup_session_request_http_uri (
		session, SOUP_METHOD_OPTIONS, uri, error);

	if (request != NULL)
		gdav_init_basic_request (request);

	return request;
}

static gboolean
gdav_init_propfind_request (SoupRequestHTTP *request,
                            GDavPropFindType type,
                            GDavPropertySet *prop,
                            GDavDepth depth,
                            GError **error)
{
	SoupMessage *message;
	GHashTable *namespaces;
	xmlDoc *doc;
	xmlNode *root;
	xmlNs *nsdav;
	gboolean success = TRUE;

	/* Gracefully resolve type/prop disagreement. */
	if (type == GDAV_PROPFIND_PROP && prop == NULL)
		type = GDAV_PROPFIND_ALLPROP;

	gdav_init_basic_request (request);

	message = soup_request_http_get_message (request);

	gdav_request_headers_add_depth (message, depth);

	doc = xmlNewDoc (BAD_CAST "1.0");
	root = xmlNewDocNode (doc, NULL, XC_PROPFIND, NULL);
	xmlDocSetRootElement (doc, root);

	namespaces = gdav_namespaces_ht_new (root);
	nsdav = g_hash_table_lookup (namespaces, GDAV_XMLNS_DAV);
	xmlSetNs (root, nsdav);

	switch (type) {
		case GDAV_PROPFIND_PROP:
			gdav_namespaces_ht_collect (
				namespaces, GDAV_PARSABLE (prop), root);
			gdav_property_set_set_names_only (prop, TRUE);
			success = gdav_parsable_serialize (
				GDAV_PARSABLE (prop),
				namespaces, doc, root, error);
			break;

		case GDAV_PROPFIND_ALLPROP:
			xmlNewTextChild (root, nsdav, XC_ALLPROP, NULL);
			break;

		case GDAV_PROPFIND_PROPNAME:
			xmlNewTextChild (root, nsdav, XC_PROPNAME, NULL);
			break;

		default:
			g_warn_if_reached ();
	}

	if (success)
		gdav_request_write_body (message, doc, root);

	g_hash_table_destroy (namespaces);

	xmlFreeDoc (doc);

	g_object_unref (message);

	return success;
}

SoupRequestHTTP *
gdav_request_propfind (SoupSession *session,
                       const gchar *uri_string,
                       GDavPropFindType type,
                       GDavPropertySet *prop,
                       GDavDepth depth,
                       GError **error)
{
	SoupRequestHTTP *request;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri_string != NULL, NULL);
	g_return_val_if_fail (
		prop == NULL || GDAV_IS_PROPERTY_SET (prop), NULL);

	request = soup_session_request_http (
		session, SOUP_METHOD_PROPFIND, uri_string, error);

	if (request != NULL) {
		if (!gdav_init_propfind_request (request, type, prop, depth, error))
			g_clear_object (&request);
	}

	return request;
}

SoupRequestHTTP *
gdav_request_propfind_uri (SoupSession *session,
                           SoupURI *uri,
                           GDavPropFindType type,
                           GDavPropertySet *prop,
                           GDavDepth depth,
                           GError **error)
{
	SoupRequestHTTP *request;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri != NULL, NULL);
	g_return_val_if_fail (
		prop == NULL || GDAV_IS_PROPERTY_SET (prop), NULL);

	request = soup_session_request_http_uri (
		session, SOUP_METHOD_PROPFIND, uri, error);

	if (request != NULL) {
		if (!gdav_init_propfind_request (request, type, prop, depth, error))
			g_clear_object (&request);
	}

	return request;
}

static gboolean
gdav_init_proppatch_request (SoupRequestHTTP *request,
                             GDavPropertyUpdate *update,
                             GError **error)
{
	SoupMessage *message;
	GHashTable *namespaces;
	xmlDoc *doc;
	xmlNode *root;
	xmlNs *nsdav;
	gboolean success;

	gdav_init_basic_request (request);

	message = soup_request_http_get_message (request);

	/* XXX GDavPropertyUpdate could produce the "propertyupdate"
	 *     element itself, but we need a root xmlNode to pass to
	 *     gdav_parsable_serialize().  Minor hack. */

	doc = xmlNewDoc (BAD_CAST "1.0");
	root = xmlNewDocNode (doc, NULL, XC_PROPERTYUPDATE, NULL);
	xmlDocSetRootElement (doc, root);

	namespaces = gdav_namespaces_ht_new (root);
	nsdav = g_hash_table_lookup (namespaces, GDAV_XMLNS_DAV);
	xmlSetNs (root, nsdav);

	gdav_namespaces_ht_collect (namespaces, GDAV_PARSABLE (update), root);

	success = gdav_parsable_serialize (
		GDAV_PARSABLE (update), namespaces, doc, root, error);

	if (success)
		gdav_request_write_body (message, doc, root);

	g_hash_table_destroy (namespaces);

	xmlFreeDoc (doc);

	g_object_unref (message);

	return success;
}

SoupRequestHTTP *
gdav_request_proppatch (SoupSession *session,
                        const gchar *uri_string,
                        GDavPropertyUpdate *update,
                        GError **error)
{
	SoupRequestHTTP *request;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri_string != NULL, NULL);
	g_return_val_if_fail (GDAV_IS_PROPERTY_UPDATE (update), NULL);

	request = soup_session_request_http (
		session, SOUP_METHOD_PROPPATCH, uri_string, error);

	if (request != NULL) {
		if (!gdav_init_proppatch_request (request, update, error))
			g_clear_object (&request);
	}

	return request;
}

SoupRequestHTTP *
gdav_request_proppatch_uri (SoupSession *session,
                            SoupURI *uri,
                            GDavPropertyUpdate *update,
                            GError **error)
{
	SoupRequestHTTP *request;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri != NULL, NULL);
	g_return_val_if_fail (GDAV_IS_PROPERTY_UPDATE (update), NULL);

	request = soup_session_request_http_uri (
		session, SOUP_METHOD_PROPPATCH, uri, error);

	if (request != NULL) {
		if (!gdav_init_proppatch_request (request, update, error))
			g_clear_object (&request);
	}

	return request;
}

SoupRequestHTTP *
gdav_request_mkcol (SoupSession *session,
                    const gchar *uri_string,
                    GError **error)
{
	SoupRequestHTTP *request;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri_string != NULL, NULL);

	request = soup_session_request_http (
		session, SOUP_METHOD_MKCOL, uri_string, error);

	if (request != NULL)
		gdav_init_basic_request (request);

	return request;
}

SoupRequestHTTP *
gdav_request_mkcol_uri (SoupSession *session,
                        SoupURI *uri,
                        GError **error)
{
	SoupRequestHTTP *request;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri != NULL, NULL);

	request = soup_session_request_http_uri (
		session, SOUP_METHOD_MKCOL, uri, error);

	if (request != NULL)
		gdav_init_basic_request (request);

	return request;
}

SoupRequestHTTP *
gdav_request_delete (SoupSession *session,
                     const gchar *uri_string,
                     GError **error)
{
	SoupRequestHTTP *request;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri_string != NULL, NULL);

	request = soup_session_request_http (
		session, SOUP_METHOD_DELETE, uri_string, error);

	if (request != NULL)
		gdav_init_basic_request (request);

	return request;
}

SoupRequestHTTP *
gdav_request_delete_uri (SoupSession *session,
                         SoupURI *uri,
                         GError **error)
{
	SoupRequestHTTP *request;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri != NULL, NULL);

	request = soup_session_request_http_uri (
		session, SOUP_METHOD_DELETE, uri, error);

	if (request != NULL)
		gdav_init_basic_request (request);

	return request;
}

static void
gdav_init_copy_request (SoupRequestHTTP *request,
                        const gchar *destination,
                        GDavCopyFlags flags)
{
	SoupMessage *message;

	gdav_init_basic_request (request);

	message = soup_request_http_get_message (request);

	gdav_request_headers_add_destination (message, destination);

	if (flags & GDAV_COPY_FLAGS_NO_OVERWRITE)
		gdav_request_headers_add_overwrite (message, FALSE);

	if (flags & GDAV_COPY_FLAGS_COLLECTION_ONLY)
		gdav_request_headers_add_depth (message, GDAV_DEPTH_0);

	g_object_unref (message);
}

SoupRequestHTTP *
gdav_request_copy (SoupSession *session,
                   const gchar *uri_string,
                   const gchar *destination,
                   GDavCopyFlags flags,
                   GError **error)
{
	SoupRequestHTTP *request;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri_string != NULL, NULL);
	g_return_val_if_fail (destination != NULL, NULL);

	request = soup_session_request_http (
		session, SOUP_METHOD_COPY, uri_string, error);

	if (request != NULL)
		gdav_init_copy_request (request, destination, flags);

	return request;
}

SoupRequestHTTP *
gdav_request_copy_uri (SoupSession *session,
                       SoupURI *uri,
                       const gchar *destination,
                       GDavCopyFlags flags,
                       GError **error)
{
	SoupRequestHTTP *request;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri != NULL, NULL);
	g_return_val_if_fail (destination != NULL, NULL);

	request = soup_session_request_http_uri (
		session, SOUP_METHOD_COPY, uri, error);

	if (request != NULL)
		gdav_init_copy_request (request, destination, flags);

	return request;
}

static void
gdav_init_move_request (SoupRequestHTTP *request,
                        const gchar *destination,
                        GDavMoveFlags flags)
{
	SoupMessage *message;

	gdav_init_basic_request (request);

	message = soup_request_http_get_message (request);

	gdav_request_headers_add_destination (message, destination);

	if (flags & GDAV_MOVE_FLAGS_NO_OVERWRITE)
		gdav_request_headers_add_overwrite (message, FALSE);

	g_object_unref (message);
}

SoupRequestHTTP *
gdav_request_move (SoupSession *session,
                   const gchar *uri_string,
                   const gchar *destination,
                   GDavMoveFlags flags,
                   GError **error)
{
	SoupRequestHTTP *request;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri_string != NULL, NULL);
	g_return_val_if_fail (destination != NULL, NULL);

	request = soup_session_request_http (
		session, SOUP_METHOD_MOVE, uri_string, error);

	if (request != NULL)
		gdav_init_move_request (request, destination, flags);

	return request;
}

SoupRequestHTTP *
gdav_request_move_uri (SoupSession *session,
                       SoupURI *uri,
                       const gchar *destination,
                       GDavMoveFlags flags,
                       GError **error)
{
	SoupRequestHTTP *request;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri != NULL, NULL);
	g_return_val_if_fail (destination != NULL, NULL);

	request = soup_session_request_http_uri (
		session, SOUP_METHOD_MOVE, uri, error);

	if (request != NULL)
		gdav_init_move_request (request, destination, flags);

	return request;
}

static void
gdav_init_lock_request (SoupRequestHTTP *request,
                        GDavLockScope lock_scope,
                        GDavLockType lock_type,
                        GDavLockFlags flags,
                        const gchar *owner,
                        gint timeout)
{
	SoupMessage *message;
	xmlDoc *doc;
	xmlNode *root;   /* depth=0 */
	xmlNode *node1;  /* depth=1 */
	xmlNs *nsdav;

	gdav_init_basic_request (request);

	message = soup_request_http_get_message (request);

	gdav_request_headers_add_timeout (message, timeout);

	if (flags & GDAV_LOCK_FLAGS_NON_RECURSIVE)
		gdav_request_headers_add_depth (message, GDAV_DEPTH_0);

	doc = xmlNewDoc (BAD_CAST "1.0");
	root = xmlNewDocNode (doc, NULL, XC_LOCKINFO, NULL);
	xmlDocSetRootElement (doc, root);

	nsdav = gdav_nsdav_new (root);
	xmlSetNs (root, nsdav);

	node1 = xmlNewTextChild (root, nsdav, XC_LOCKSCOPE, NULL);

	switch (lock_scope) {
		case GDAV_LOCK_SCOPE_EXCLUSIVE:
		default:  /* fallback for invalid values */
			xmlNewTextChild (node1, nsdav, XC_EXCLUSIVE, NULL);
			break;
		case GDAV_LOCK_SCOPE_SHARED:
			xmlNewTextChild (node1, nsdav, XC_SHARED, NULL);
			break;
	}

	/* XXX Just hard-code this since there's only one valid locktype.
	 *     We can rewrite it if GDavLockType ever grows new values. */
	node1 = xmlNewTextChild (root, nsdav, XC_LOCKTYPE, NULL);
	xmlNewTextChild (node1, nsdav, XC_WRITE, NULL);

	if (owner != NULL) {
		SoupURI *uri = NULL;

		if (flags & GDAV_LOCK_FLAGS_OWNER_IS_URI)
			uri = soup_uri_new (owner);

		if (uri != NULL) {
			node1 = xmlNewTextChild (root, nsdav, XC_OWNER, NULL);
			xmlNewTextChild (node1, nsdav, XC_HREF, BAD_CAST owner);
			soup_uri_free (uri);
		} else {
			xmlNewTextChild (root, nsdav, XC_OWNER, BAD_CAST owner);
		}
	}

	gdav_request_write_body (message, doc, root);

	xmlFreeDoc (doc);

	g_object_unref (message);
}

SoupRequestHTTP *
gdav_request_lock (SoupSession *session,
                   const gchar *uri_string,
                   GDavLockScope lock_scope,
                   GDavLockType lock_type,
                   GDavLockFlags flags,
                   const gchar *owner,
                   gint timeout,
                   GError **error)
{
	SoupRequestHTTP *request;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri_string != NULL, NULL);

	request = soup_session_request_http (
		session, SOUP_METHOD_LOCK, uri_string, error);

	if (request != NULL) {
		gdav_init_lock_request (
			request, lock_scope, lock_type,
			flags, owner, timeout);
	}

	return request;
}

SoupRequestHTTP *
gdav_request_lock_uri (SoupSession *session,
                       SoupURI *uri,
                       GDavLockScope lock_scope,
                       GDavLockType lock_type,
                       GDavLockFlags flags,
                       const gchar *owner,
                       gint timeout,
                       GError **error)
{
	SoupRequestHTTP *request;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri != NULL, NULL);

	request = soup_session_request_http_uri (
		session, SOUP_METHOD_LOCK, uri, error);

	if (request != NULL) {
		gdav_init_lock_request (
			request, lock_scope, lock_type,
			flags, owner, timeout);
	}

	return request;
}

static void
gdav_init_lock_refresh_request (SoupRequestHTTP *request,
                                const gchar *lock_token,
                                gint timeout)
{
	SoupMessage *message;
	gchar *condition;

	gdav_init_basic_request (request);

	message = soup_request_http_get_message (request);

	gdav_request_headers_add_timeout (message, timeout);

	condition = g_strdup_printf ("(<%s>)", lock_token);
	soup_message_headers_replace (
		message->request_headers, "If", condition);
	g_free (condition);

	g_object_unref (message);
}

SoupRequestHTTP *
gdav_request_lock_refresh (SoupSession *session,
                           const gchar *uri_string,
                           const gchar *lock_token,
                           gint timeout,
                           GError **error)
{
	SoupRequestHTTP *request;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri_string != NULL, NULL);
	g_return_val_if_fail (lock_token != NULL, NULL);

	request = soup_session_request_http (
		session, SOUP_METHOD_LOCK, uri_string, error);

	if (request != NULL)
		gdav_init_lock_refresh_request (request, lock_token, timeout);

	return request;
}

SoupRequestHTTP *
gdav_request_lock_refresh_uri (SoupSession *session,
                               SoupURI *uri,
                               const gchar *lock_token,
                               gint timeout,
                               GError **error)
{
	SoupRequestHTTP *request;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri != NULL, NULL);
	g_return_val_if_fail (lock_token != NULL, NULL);

	request = soup_session_request_http_uri (
		session, SOUP_METHOD_LOCK, uri, error);

	if (request != NULL)
		gdav_init_lock_refresh_request (request, lock_token, timeout);

	return request;
}

static void
gdav_init_unlock_request (SoupRequestHTTP *request,
                          const gchar *lock_token)
{
	SoupMessage *message;

	gdav_init_basic_request (request);

	message = soup_request_http_get_message (request);

	gdav_request_headers_add_lock_token (message, lock_token);

	g_object_unref (message);
}

SoupRequestHTTP *
gdav_request_unlock (SoupSession *session,
                     const gchar *uri_string,
                     const gchar *lock_token,
                     GError **error)
{
	SoupRequestHTTP *request;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri_string != NULL, NULL);
	g_return_val_if_fail (lock_token != NULL, NULL);

	request = soup_session_request_http (
		session, SOUP_METHOD_UNLOCK, uri_string, error);

	if (request != NULL)
		gdav_init_unlock_request (request, lock_token);

	return request;
}

SoupRequestHTTP *
gdav_request_unlock_uri (SoupSession *session,
                         SoupURI *uri,
                         const gchar *lock_token,
                         GError **error)
{
	SoupRequestHTTP *request;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri != NULL, NULL);
	g_return_val_if_fail (lock_token != NULL, NULL);

	request = soup_session_request_http_uri (
		session, SOUP_METHOD_UNLOCK, uri, error);

	if (request != NULL)
		gdav_init_unlock_request (request, lock_token);

	return request;
}

static void
gdav_init_mkredirectref_request (SoupRequestHTTP *request,
                                 SoupURI *target,
                                 GDavRedirectLifetime lifetime)
{
	SoupMessage *message;
	xmlDoc *doc;
	xmlNode *root;
	xmlNs *nsdav;

	gdav_init_basic_request (request);

	message = soup_request_http_get_message (request);

	doc = xmlNewDoc (BAD_CAST "1.0");
	root = xmlNewDocNode (doc, NULL, XC_MKREDIRECTREF, NULL);
	xmlDocSetRootElement (doc, root);

	nsdav = gdav_nsdav_new (root);
	xmlSetNs (root, nsdav);

	gdav_request_write_reftarget (target, root, nsdav);
	gdav_request_write_redirect_lifetime (lifetime, root, nsdav);

	gdav_request_write_body (message, doc, root);

	xmlFreeDoc (doc);

	g_object_unref (message);
}

SoupRequestHTTP *
gdav_request_mkredirectref (SoupSession *session,
                            const gchar *uri_string,
                            SoupURI *target,
                            GDavRedirectLifetime lifetime,
                            GError **error)
{
	SoupRequestHTTP *request;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri_string != NULL, NULL);
	g_return_val_if_fail (target != NULL, NULL);

	request = soup_session_request_http (
		session, "MKREDIRECTREF", uri_string, error);

	if (request != NULL)
		gdav_init_mkredirectref_request (request, target, lifetime);

	return request;
}

SoupRequestHTTP *
gdav_request_mkredirectref_uri (SoupSession *session,
                                SoupURI *uri,
                                SoupURI *target,
                                GDavRedirectLifetime lifetime,
                                GError **error)
{
	SoupRequestHTTP *request;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri != NULL, NULL);
	g_return_val_if_fail (target != NULL, NULL);

	request = soup_session_request_http_uri (
		session, "MKREDIRECTREF", uri, error);

	if (request != NULL)
		gdav_init_mkredirectref_request (request, target, lifetime);

	return request;
}

static void
gdav_init_updateredirectref_request (SoupRequestHTTP *request,
                                     SoupURI *target,
                                     GDavRedirectLifetime lifetime)
{
	SoupMessage *message;
	xmlDoc *doc;
	xmlNode *root;
	xmlNs *nsdav;

	gdav_init_basic_request (request);

	gdav_request_apply_to_redirect_ref (request, TRUE);

	message = soup_request_http_get_message (request);

	doc = xmlNewDoc (BAD_CAST "1.0");
	root = xmlNewDocNode (doc, NULL, XC_UPDATEREDIRECTREF, NULL);
	xmlDocSetRootElement (doc, root);

	nsdav = gdav_nsdav_new (root);
	xmlSetNs (root, nsdav);

	gdav_request_write_reftarget (target, root, nsdav);
	gdav_request_write_redirect_lifetime (lifetime, root, nsdav);

	gdav_request_write_body (message, doc, root);

	xmlFreeDoc (doc);

	g_object_unref (message);
}

SoupRequestHTTP *
gdav_request_updateredirectref (SoupSession *session,
                                const gchar *uri_string,
                                SoupURI *target,
                                GDavRedirectLifetime lifetime,
                                GError **error)
{
	SoupRequestHTTP *request;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri_string != NULL, NULL);
	/* target can be NULL */

	request = soup_session_request_http (
		session, "UPDATEREDIRECTREF", uri_string, error);

	if (request != NULL)
		gdav_init_updateredirectref_request (request, target, lifetime);

	return request;
}

SoupRequestHTTP *
gdav_request_updateredirectref_uri (SoupSession *session,
                                    SoupURI *uri,
                                    SoupURI *target,
                                    GDavRedirectLifetime lifetime,
                                    GError **error)
{
	SoupRequestHTTP *request;

	g_return_val_if_fail (SOUP_IS_SESSION (session), NULL);
	g_return_val_if_fail (uri != NULL, NULL);
	/* target can be NULL */

	request = soup_session_request_http_uri (
		session, "UPDATEREDIRECTREF", uri, error);

	if (request != NULL)
		gdav_init_updateredirectref_request (request, target, lifetime);

	return request;
}

void
gdav_request_add_lock_token (SoupRequestHTTP *request,
                             const gchar *resource_tag,
                             const gchar *lock_token)
{
	SoupMessage *message;
	SoupMessageHeaders *headers;
	GString *tagged_list;
	const gchar *value;

	g_return_if_fail (SOUP_IS_REQUEST_HTTP (request));
	g_return_if_fail (lock_token != NULL);

	message = soup_request_http_get_message (request);
	headers = message->request_headers;

	value = soup_message_headers_get_one (headers, "If");

	/* g_string_new() accepts NULL */
	tagged_list = g_string_new (value);
	if (tagged_list->len > 0)
		g_string_append_c (tagged_list, ' ');

	if (resource_tag != NULL) {
		g_string_append_printf (tagged_list, "<%s>", resource_tag);
	} else {
		SoupURI *uri;
		gchar *uri_string;

		uri = soup_request_get_uri (SOUP_REQUEST (request));
		uri_string = soup_uri_to_string (uri, FALSE);
		g_string_append_printf (tagged_list, "<%s>", uri_string);
		g_free (uri_string);
	}

	g_string_append_printf (tagged_list, " (<%s>)", lock_token);

	soup_message_headers_replace (headers, "If", tagged_list->str);

	g_string_free (tagged_list, TRUE);

	g_object_unref (message);
}

void
gdav_request_apply_to_redirect_ref (SoupRequestHTTP *request,
                                    gboolean true_or_false)
{
	SoupMessage *message;
	SoupMessageHeaders *headers;

	g_return_if_fail (SOUP_IS_REQUEST_HTTP (request));

	message = soup_request_http_get_message (request);
	headers = message->request_headers;

	soup_message_headers_replace (
		headers, "Apply-To-Redirect-Ref",
		true_or_false ? "T" : "F");
}

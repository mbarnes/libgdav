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

#include "gdav-parsable.h"

#include <glib/gi18n-lib.h>

/* Include everything for GType registrations. */
#include <libgdav/gdav.h>

#define GDAV_PARSABLE_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE \
	((obj), GDAV_TYPE_PARSABLE, GDavParsablePrivate))

struct _GDavParsablePrivate {
	gint placeholder;
};

G_DEFINE_ABSTRACT_TYPE (GDavParsable, gdav_parsable, G_TYPE_OBJECT)

G_DEFINE_QUARK (gdav-parsable-error-quark, gdav_parsable_error)

static gboolean
gdav_parsable_real_deserialize (GDavParsable *parsable,
                                SoupURI *base_uri,
                                xmlDoc *doc,
                                xmlNode *node,
                                GError **error)
{
	/* FIXME Store off unhandled xmlNode. */
	return TRUE;
}

static void
gdav_parsable_class_init (GDavParsableClass *class)
{
	g_type_class_add_private (class, sizeof (GDavParsablePrivate));

	class->deserialize = gdav_parsable_real_deserialize;

	/* Set up libxml to use GLib memory allocation functions.
	 * Not only does this cut down on g_strdup() calls, but we
	 * also don't have to deal with memory allocation failures
	 * in libxml functions since GLib will abort. */
	xmlMemSetup (
		(xmlFreeFunc) g_free,
		(xmlMallocFunc) g_malloc,
		(xmlReallocFunc) g_realloc,
		(xmlStrdupFunc) g_strdup);

	/* Register all GDavParsable subtypes. */

	/* Containers */
	g_type_ensure (GDAV_TYPE_ACTIVE_LOCK);
	g_type_ensure (GDAV_TYPE_ERROR);
	g_type_ensure (GDAV_TYPE_LOCK_ENTRY);
	g_type_ensure (GDAV_TYPE_MULTI_STATUS);
	g_type_ensure (GDAV_TYPE_PROPERTY_SET);
	g_type_ensure (GDAV_TYPE_PROPERTY_UPDATE);
	g_type_ensure (GDAV_TYPE_PROP_STAT);
	g_type_ensure (GDAV_TYPE_RESPONSE);

	/* Properties */
	g_type_ensure (GDAV_TYPE_ALTERNATE_URI_SET_PROPERTY);
	g_type_ensure (GDAV_TYPE_CALENDAR_DESCRIPTION_PROPERTY);
	g_type_ensure (GDAV_TYPE_CALENDAR_TIMEZONE_PROPERTY);
	g_type_ensure (GDAV_TYPE_CREATIONDATE_PROPERTY);
	g_type_ensure (GDAV_TYPE_DISPLAYNAME_PROPERTY);
	g_type_ensure (GDAV_TYPE_GETCONTENTLANGUAGE_PROPERTY);
	g_type_ensure (GDAV_TYPE_GETCONTENTLENGTH_PROPERTY);
	g_type_ensure (GDAV_TYPE_GETETAG_PROPERTY);
	g_type_ensure (GDAV_TYPE_GETLASTMODIFIED_PROPERTY);
	g_type_ensure (GDAV_TYPE_GROUP_MEMBERSHIP_PROPERTY);
	g_type_ensure (GDAV_TYPE_GROUP_MEMBER_SET_PROPERTY);
	g_type_ensure (GDAV_TYPE_LOCKDISCOVERY_PROPERTY);
	g_type_ensure (GDAV_TYPE_MAX_RESOURCE_SIZE_PROPERTY);
	g_type_ensure (GDAV_TYPE_OWNER_PROPERTY);
	g_type_ensure (GDAV_TYPE_PRINCIPAL_URL_PROPERTY);
	g_type_ensure (GDAV_TYPE_REDIRECT_LIFETIME_PROPERTY);
	g_type_ensure (GDAV_TYPE_REFTARGET_PROPERTY);
	g_type_ensure (GDAV_TYPE_RESOURCETYPE_PROPERTY);
	g_type_ensure (GDAV_TYPE_SCHEDULE_INBOX_URL_PROPERTY);
	g_type_ensure (GDAV_TYPE_SCHEDULE_OUTBOX_URL_PROPERTY);
	g_type_ensure (GDAV_TYPE_SUPPORTED_CALENDAR_COMPONENT_SET_PROPERTY);
	g_type_ensure (GDAV_TYPE_SUPPORTED_CALENDAR_DATA_PROPERTY);
	g_type_ensure (GDAV_TYPE_SUPPORTEDLOCK_PROPERTY);
}

static void
gdav_parsable_init (GDavParsable *parsable)
{
	parsable->priv = GDAV_PARSABLE_GET_PRIVATE (parsable);
}

gboolean
gdav_parsable_is_a (xmlNode *node,
                    GType parsable_type)
{
	GDavParsableClass *class;
	gboolean match;

	g_return_val_if_fail (node != NULL, FALSE);
	g_return_val_if_fail (node->name != NULL, FALSE);
	g_return_val_if_fail (
		g_type_is_a (parsable_type, GDAV_TYPE_PARSABLE), FALSE);

	class = g_type_class_ref (parsable_type);

	g_warn_if_fail (class->element_name != NULL);
	g_warn_if_fail (class->element_namespace != NULL);

	/* xmlStrcmp() handles NULL arguments gracefully. */
	match = (gdav_is_xmlns (node, class->element_namespace)) &&
		(xmlStrcmp (node->name, BAD_CAST class->element_name) == 0);

	g_type_class_unref (class);

	return match;
}

/* Helper for gdav_parsable_type_from_node() */
static GType
parsable_type_from_node_rec (GType parent_type,
                             xmlNode *node)
{
	GType *children;
	guint n_children, ii;
	GType type = G_TYPE_INVALID;

	children = g_type_children (parent_type, &n_children);

	for (ii = 0; ii < n_children; ii++) {
		GType child_type;

		child_type = children[ii];

		/* Recurse over the child's children. */
		type = parsable_type_from_node_rec (child_type, node);

		if (type != G_TYPE_INVALID)
			break;

		if (G_TYPE_IS_ABSTRACT (child_type))
			continue;

		if (gdav_parsable_is_a (node, child_type)) {
			type = child_type;
			break;
		}
	}

	g_free (children);

	return type;
}

GType
gdav_parsable_lookup_type (xmlNode *node,
                           GError **error)
{
	GType type = G_TYPE_INVALID;

	g_return_val_if_fail (node != NULL, NULL);
	g_return_val_if_fail (node->name != NULL, NULL);

	if (node->ns == NULL || node->ns->href == NULL) {
		g_set_error (
			error, GDAV_PARSABLE_ERROR,
			GDAV_PARSABLE_ERROR_UNKNOWN_ELEMENT,
			_("No XML namespace for element <%s>"),
			node->name);
		return G_TYPE_INVALID;
	}

	type = parsable_type_from_node_rec (GDAV_TYPE_PARSABLE, node);

	if (!g_type_is_a (type, GDAV_TYPE_PARSABLE)) {
		g_set_error (
			error, GDAV_PARSABLE_ERROR,
			GDAV_PARSABLE_ERROR_UNKNOWN_ELEMENT,
			_("Unknown XML element <%s>"),
			node->name);
		return G_TYPE_INVALID;
	}

	return type;
}

gpointer
gdav_parsable_new_from_data (GType parsable_type,
                             SoupURI *base_uri,
                             gconstpointer data,
                             gsize data_size,
                             GError **error)
{
	xmlDoc *doc;
	xmlNode *root;
	gpointer parsable;

	g_return_val_if_fail (
		g_type_is_a (parsable_type, GDAV_TYPE_PARSABLE), NULL);
	g_return_val_if_fail (SOUP_URI_VALID_FOR_HTTP (base_uri), NULL);
	g_return_val_if_fail (data != NULL, NULL);

	doc = xmlReadMemory (data, data_size, "/dev/null", NULL, 0);

	if (doc == NULL) {
		xmlError *xml_error = xmlGetLastError ();
		const gchar *message = NULL;

		if (xml_error != NULL)
			message = xml_error->message;
		if (message == NULL) {
			/* Translators: This is a fallback in the event
			 * of an XML parsing error with no error message. */
			message = _("unspecified");
		}

		g_set_error (
			error, GDAV_PARSABLE_ERROR,
			GDAV_PARSABLE_ERROR_PARSER_FAILED,
			_("Error parsing XML: %s"), message);

		return NULL;
	}

	root = xmlDocGetRootElement (doc);

	if (root == NULL) {
		xmlFreeDoc (doc);

		g_set_error (
			error, GDAV_PARSABLE_ERROR,
			GDAV_PARSABLE_ERROR_EMPTY_DOCUMENT,
			_("Error parsing XML: %s"),
			_("Empty document"));

		return NULL;
	}

	parsable = gdav_parsable_new_from_xml_node (
		parsable_type, base_uri, doc, root, error);

	xmlFreeDoc (doc);

	return parsable;
}

gpointer
gdav_parsable_new_from_xml_node (GType parsable_type,
                                 SoupURI *base_uri,
                                 xmlDoc *doc,
                                 xmlNode *node,
                                 GError **error)
{
	GDavParsable *parsable;
	xmlNode *child;

	g_return_val_if_fail (
		g_type_is_a (parsable_type, GDAV_TYPE_PARSABLE), NULL);
	g_return_val_if_fail (SOUP_URI_VALID_FOR_HTTP (base_uri), NULL);
	g_return_val_if_fail (doc != NULL, NULL);
	g_return_val_if_fail (node != NULL, NULL);

	parsable = g_object_new (parsable_type, NULL);

	for (child = node->children; child != NULL; child = child->next) {
		gboolean success;

		success = gdav_parsable_deserialize (
			parsable, base_uri, doc, child, error);

		if (!success) {
			g_clear_object (&parsable);
			break;
		}
	}

	return parsable;
}

gboolean
gdav_parsable_serialize (GDavParsable *parsable,
                         GHashTable *namespaces,
                         xmlDoc *doc,
                         xmlNode *parent,
                         GError **error)
{
	GDavParsableClass *class;

	g_return_val_if_fail (GDAV_IS_PARSABLE (parsable), FALSE);
	g_return_val_if_fail (namespaces != NULL, FALSE);
	g_return_val_if_fail (doc != NULL, FALSE);
	g_return_val_if_fail (parent != NULL, FALSE);

	class = GDAV_PARSABLE_GET_CLASS (parsable);
	g_return_val_if_fail (class->serialize != NULL, FALSE);
	g_return_val_if_fail (class->element_name != NULL, FALSE);
	g_return_val_if_fail (class->element_namespace != NULL, FALSE);

	return class->serialize (parsable, namespaces, doc, parent, error);
}

gboolean
gdav_parsable_deserialize (GDavParsable *parsable,
                           SoupURI *base_uri,
                           xmlDoc *doc,
                           xmlNode *node,
                           GError **error)
{
	GDavParsableClass *class;

	g_return_val_if_fail (GDAV_IS_PARSABLE (parsable), FALSE);
	g_return_val_if_fail (SOUP_URI_VALID_FOR_HTTP (base_uri), NULL);
	g_return_val_if_fail (doc != NULL, FALSE);
	g_return_val_if_fail (node != NULL, FALSE);

	class = GDAV_PARSABLE_GET_CLASS (parsable);
	g_return_val_if_fail (class->deserialize != NULL, FALSE);

	return class->deserialize (parsable, base_uri, doc, node, error);
}

void
gdav_parsable_collect_types (GDavParsable *parsable,
                             GHashTable *parsable_types)
{
	GDavParsableClass *class;

	g_return_if_fail (GDAV_IS_PARSABLE (parsable));
	g_return_if_fail (parsable_types != NULL);

	g_hash_table_add (
		parsable_types,
		GSIZE_TO_POINTER (G_OBJECT_TYPE (parsable)));

	class = GDAV_PARSABLE_GET_CLASS (parsable);

	if (class->collect_types != NULL)
		class->collect_types (parsable, parsable_types);
}

xmlNode *
gdav_parsable_new_text_child (GType parsable_type,
                              GHashTable *namespaces,
                              xmlNode *parent,
                              const xmlChar *content)
{
	GDavParsableClass *class;
	const xmlChar *name;
	xmlNode *node;
	xmlNs *ns = NULL;

	g_return_val_if_fail (
		g_type_is_a (parsable_type, GDAV_TYPE_PARSABLE), NULL);
	g_return_val_if_fail (namespaces != NULL, NULL);

	class = g_type_class_ref (parsable_type);

	if (class->element_namespace != NULL) {
		ns = g_hash_table_lookup (
			namespaces, class->element_namespace);
	}

	name = BAD_CAST class->element_name;
	node = xmlNewTextChild (parent, ns, name, content);

	g_type_class_unref (class);

	return node;
}

SoupURI *
gdav_parsable_deserialize_href (GDavParsable *parsable,
                                SoupURI *base_uri,
                                xmlDoc *doc,
                                xmlNode *href,
                                GError **error)
{
	xmlChar *text;
	SoupURI *uri;

	g_return_val_if_fail (GDAV_IS_PARSABLE (parsable), NULL);
	g_return_val_if_fail (base_uri != NULL, NULL);
	g_return_val_if_fail (doc != NULL, NULL);
	g_return_val_if_fail (href != NULL, NULL);

	text = xmlNodeListGetString (doc, href->children, TRUE);

	uri = soup_uri_new_with_base (base_uri, (gchar *) text);

	if (!SOUP_URI_VALID_FOR_HTTP (uri)) {
		g_set_error (
			error, GDAV_PARSABLE_ERROR,
			GDAV_PARSABLE_ERROR_INTERNAL,
			_("Invalid href value '%s'"),
			(gchar *) text);
		if (uri != NULL) {
			soup_uri_free (uri);
			uri = NULL;
		}
	}

	xmlFree (text);

	return uri;
}


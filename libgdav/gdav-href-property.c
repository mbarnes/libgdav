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

#include "gdav-href-property.h"

G_DEFINE_ABSTRACT_TYPE (
	GDavHRefProperty,
	gdav_href_property,
	GDAV_TYPE_PROPERTY)

static gboolean
gdav_href_property_serialize (GDavParsable *parsable,
                              GHashTable *namespaces,
                              xmlDoc *doc,
                              xmlNode *parent,
                              GError **error)
{
	GDavProperty *property;
	GValue value = G_VALUE_INIT;
	SoupURI *uri;
	xmlNs *ns;

	ns = g_hash_table_lookup (namespaces, GDAV_XMLNS_DAV);
	g_return_val_if_fail (ns != NULL, FALSE);

	property = GDAV_PROPERTY (parsable);
	gdav_property_get_value (property, &value);
	uri = g_value_get_boxed (&value);

	if (uri != NULL) {
		xmlNode *node;
		gchar *uri_string;

		node = gdav_parsable_new_text_child (
			G_OBJECT_TYPE (parsable), namespaces, parent, NULL);

		uri_string = soup_uri_to_string (uri, FALSE);
		xmlNewTextChild (
			node, ns,
			BAD_CAST "href",
			BAD_CAST uri_string);
		g_free (uri_string);
	}

	g_value_unset (&value);

	return TRUE;
}

static gboolean
gdav_href_property_deserialize (GDavParsable *parsable,
                                SoupURI *base_uri,
                                xmlDoc *doc,
                                xmlNode *node,
                                GError **error)
{
	if (gdav_is_xmlns (node, GDAV_XMLNS_DAV)) {
		if (xmlStrcmp (node->name, BAD_CAST "href") == 0) {
			GDavProperty *property;
			GValue value = G_VALUE_INIT;
			SoupURI *uri;

			property = GDAV_PROPERTY (parsable);

			uri = gdav_parsable_deserialize_href (
				parsable, base_uri, doc, node, error);

			if (uri == NULL)
				return FALSE;

			g_value_init (&value, SOUP_TYPE_URI);
			g_value_take_boxed (&value, uri);
			gdav_property_set_value (property, &value);
			g_value_unset (&value);

			return TRUE;
		}
	}

	/* Chain up to parent's deserialize() method. */
	return GDAV_PARSABLE_CLASS (gdav_href_property_parent_class)->
		deserialize (parsable, base_uri, doc, node, error);
}

static void
gdav_href_property_class_init (GDavPropertyClass *class)
{
	GDavParsableClass *parsable_class;

	parsable_class = GDAV_PARSABLE_CLASS (class);
	parsable_class->serialize = gdav_href_property_serialize;
	parsable_class->deserialize = gdav_href_property_deserialize;

	class->value_type = SOUP_TYPE_URI;
}

static void
gdav_href_property_init (GDavProperty *property)
{
}


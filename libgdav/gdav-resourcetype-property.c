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

#include "gdav-resourcetype-property.h"

#include <glib/gi18n-lib.h>

#include "gdav-enumtypes.h"

G_DEFINE_TYPE (
	GDavResourceTypeProperty,
	gdav_resourcetype_property,
	GDAV_TYPE_PROPERTY)

static struct {
	xmlChar *name;
	xmlChar *ns_href;
	GDavResourceType type;
} property_map[] = {

	/* Prioritize these; highest first. */

	{ BAD_CAST "collection",
	  BAD_CAST GDAV_XMLNS_DAV,
	  GDAV_RESOURCE_TYPE_COLLECTION }
};

static gboolean
gdav_resourcetype_property_serialize (GDavParsable *parsable,
                                      GHashTable *namespaces,
                                      xmlDoc *doc,
                                      xmlNode *parent,
                                      GError **error)
{
	GValue value = G_VALUE_INIT;
	GDavResourceType resource_type;
	xmlNode *node;
	xmlNs *ns;
	gint ii;

	gdav_property_get_value (GDAV_PROPERTY (parsable), &value);
	resource_type = g_value_get_flags (&value);
	g_value_unset (&value);

	ns = g_hash_table_lookup (namespaces, GDAV_XMLNS_DAV);
	node = xmlNewTextChild (parent, ns, BAD_CAST "resourcetype", NULL);

	for (ii = 0; ii < G_N_ELEMENTS (property_map); ii++) {
		if (resource_type & property_map[ii].type) {
			ns = g_hash_table_lookup (
				namespaces, property_map[ii].ns_href);
			xmlNewTextChild (
				node, ns, property_map[ii].name, NULL);
		}
	}

	return TRUE;
}

static gboolean
gdav_resourcetype_property_deserialize (GDavParsable *parsable,
                                        SoupURI *base_uri,
                                        xmlDoc *doc,
                                        xmlNode *node,
                                        GError **error)
{
	GArray *array;
	GDavResourceType resource_type;
	GValue value = G_VALUE_INIT;
	xmlNode *child;
	gint ii;

	if (node->ns == NULL)
		goto chainup;

	gdav_property_get_value (GDAV_PROPERTY (parsable), &value);
	resource_type = g_value_get_flags (&value);

	/* xmlStrcmp() handles NULL arguments gracefully. */

	for (ii = 0; ii < G_N_ELEMENTS (property_map); ii++) {
		if (xmlStrcmp (node->name, property_map[ii].name) != 0)
			continue;
		if (xmlStrcmp (node->ns->href, property_map[ii].ns_href) != 0)
			continue;

		resource_type |= property_map[ii].type;
		g_value_set_flags (&value, resource_type);
		gdav_property_set_value (GDAV_PROPERTY (parsable), &value);

		g_value_unset (&value);

		return TRUE;
	}

	g_value_unset (&value);

chainup:
	/* Chain up to parent's deserialize() method. */
	GDAV_PARSABLE_CLASS (gdav_resourcetype_property_parent_class)->
		deserialize (parsable, base_uri, doc, node, error);
}

static void
gdav_resourcetype_property_class_init (GDavResourceTypePropertyClass *class)
{
	GDavParsableClass *parsable_class;
	GDavPropertyClass *property_class;

	parsable_class = GDAV_PARSABLE_CLASS (class);
	parsable_class->element_name = "resourcetype";
	parsable_class->element_namespace = GDAV_XMLNS_DAV;
	parsable_class->serialize = gdav_resourcetype_property_serialize;
	parsable_class->deserialize = gdav_resourcetype_property_deserialize;

	property_class = GDAV_PROPERTY_CLASS (class);
	property_class->value_type = GDAV_TYPE_RESOURCE_TYPE;
}

static void
gdav_resourcetype_property_init (GDavResourceTypeProperty *property)
{
}

GDavProperty *
gdav_resourcetype_property_new (GDavResourceType prop_value)
{
	GDavProperty *property;
	GValue value = G_VALUE_INIT;

	g_value_init (&value, GDAV_TYPE_RESOURCE_TYPE);
	g_value_set_flags (&value, prop_value);

	property = g_object_new (
		GDAV_TYPE_RESOURCETYPE_PROPERTY,
		"value", &value, NULL);

	g_value_unset (&value);

	return property;
}


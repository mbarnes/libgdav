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

#include "gdav-redirect-lifetime-property.h"

#include <glib/gi18n-lib.h>

#include "gdav-enumtypes.h"

G_DEFINE_TYPE (
	GDavRedirectLifetimeProperty,
	gdav_redirect_lifetime_property,
	GDAV_TYPE_PROPERTY)

static struct {
	xmlChar *name;
	xmlChar *ns_href;
	GDavRedirectLifetime value;
} xml_map[] = {

	{ BAD_CAST "permanent",
	  BAD_CAST GDAV_XMLNS_DAV,
	  GDAV_REDIRECT_LIFETIME_PERMANENT },

	{ BAD_CAST "temporary",
	  BAD_CAST GDAV_XMLNS_DAV,
	  GDAV_REDIRECT_LIFETIME_TEMPORARY }
};

static gboolean
gdav_redirect_lifetime_property_serialize (GDavParsable *parsable,
                                           GHashTable *namespaces,
                                           xmlDoc *doc,
                                           xmlNode *parent,
                                           GError **error)
{
	GValue value = G_VALUE_INIT;
	GDavRedirectLifetime lifetime;
	gint ii;

	gdav_property_get_value (GDAV_PROPERTY (parsable), &value);
	lifetime = g_value_get_enum (&value);
	g_value_unset (&value);

	for (ii = 0; ii < G_N_ELEMENTS (xml_map); ii++) {
		if (lifetime == xml_map[ii].value) {
			xmlNode *node;
			xmlNs *ns;

			node = gdav_parsable_new_text_child (
				G_OBJECT_TYPE (parsable),
				namespaces, parent, NULL);

			ns = g_hash_table_lookup (
				namespaces, xml_map[ii].ns_href);
			xmlNewTextChild (node, ns, xml_map[ii].name, NULL);

			break;
		}
	}

	return TRUE;
}

static gboolean
gdav_redirect_lifetime_property_deserialize (GDavParsable *parsable,
                                             SoupURI *base_uri,
                                             xmlDoc *doc,
                                             xmlNode *node,
                                             GError **error)
{
	GValue value = G_VALUE_INIT;
	gint ii;

	if (node->ns == NULL)
		goto chainup;

	/* xmlStrcmp() handles NULL arguments gracefully. */

	for (ii = 0; ii < G_N_ELEMENTS (xml_map); ii++) {
		if (xmlStrcmp (node->name, xml_map[ii].name) != 0)
			continue;
		if (xmlStrcmp (node->ns->href, xml_map[ii].ns_href) != 0)
			continue;

		g_value_init (&value, GDAV_TYPE_REDIRECT_LIFETIME);
		g_value_set_enum (&value, xml_map[ii].value);
		gdav_property_set_value (GDAV_PROPERTY (parsable), &value);
		g_value_unset (&value);

		return TRUE;
	}

chainup:
	/* Chain up to parent's deserialize() method. */
	return GDAV_PARSABLE_CLASS (gdav_redirect_lifetime_property_parent_class)->
		deserialize (parsable, base_uri, doc, node, error);
}

static void
gdav_redirect_lifetime_property_class_init (GDavPropertyClass *class)
{
	GDavParsableClass *parsable_class;

	parsable_class = GDAV_PARSABLE_CLASS (class);
	parsable_class->element_name = "redirect-lifetime";
	parsable_class->element_namespace = GDAV_XMLNS_DAV;
	parsable_class->serialize = gdav_redirect_lifetime_property_serialize;
	parsable_class->deserialize = gdav_redirect_lifetime_property_deserialize;

	class->value_type = GDAV_TYPE_REDIRECT_LIFETIME;
}

static void
gdav_redirect_lifetime_property_init (GDavProperty *property)
{
}


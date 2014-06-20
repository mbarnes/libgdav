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

#include "gdav-pcdata-property.h"

#include "gdav-utils.h"

G_DEFINE_ABSTRACT_TYPE (
	GDavPCDataProperty,
	gdav_pcdata_property,
	GDAV_TYPE_PROPERTY)

static gboolean
gdav_pcdata_property_serialize (GDavParsable *parsable,
                                GHashTable *namespaces,
                                xmlDoc *doc,
                                xmlNode *parent,
                                GError **error)
{
	GDavPCDataProperty *property;
	gchar *data;

	property = GDAV_PCDATA_PROPERTY (parsable);
	data = gdav_pcdata_property_write_data (property);

	if (data != NULL) {
		gdav_parsable_new_text_child (
			G_OBJECT_TYPE (parsable),
			namespaces, parent, BAD_CAST data);
		g_free (data);
	}

	return TRUE;
}

static gboolean
gdav_pcdata_property_deserialize (GDavParsable *parsable,
                                  SoupURI *base_uri,
                                  xmlDoc *doc,
                                  xmlNode *node,
                                  GError **error)
{
	GDavPCDataPropertyClass *class;
	GDavPCDataProperty *property;
	GValue value = G_VALUE_INIT;
	const gchar *data;
	xmlChar *text;
	gboolean success = TRUE;

	if (!xmlNodeIsText (node))
		goto chainup;

	property = GDAV_PCDATA_PROPERTY (parsable);
	class = GDAV_PCDATA_PROPERTY_GET_CLASS (parsable);

	text = xmlNodeListGetString (doc, node, TRUE);
	if ((text == NULL || *text == '\0') && !class->allow_empty_data) {
		gdav_error_missing_content (node, error);
		xmlFree (text);
		return FALSE;
	}

	data = (text != NULL) ? (gchar *) text : "";

	if (gdav_pcdata_property_parse_data (property, data, &value)) {
		gdav_property_set_value (GDAV_PROPERTY (property), &value);
		g_value_unset (&value);
	} else {
		gdav_error_unknown_content (node, data, error);
		success = FALSE;
	}

	xmlFree (text);

	return success;

chainup:
	/* Chain up to parent's deserialize() method. */
	return GDAV_PARSABLE_CLASS (gdav_pcdata_property_parent_class)->
		deserialize (parsable, base_uri, doc, node, error);
}

static gboolean
gdav_pcdata_property_real_parse_data (GDavPCDataProperty *property,
                                      const gchar *data,
                                      GValue *value)
{
	GDavPropertyClass *class;
	GValue data_value = G_VALUE_INIT;
	GValue real_value = G_VALUE_INIT;
	gboolean success = FALSE;

	class = GDAV_PROPERTY_GET_CLASS (property);

	g_value_init (&data_value, G_TYPE_STRING);
	g_value_init (&real_value, class->value_type);

	g_value_set_string (&data_value, data);

	/* We want to leave the output GValue unset (zero-filled) if
	 * g_value_transform() fails, so first try transforming to a
	 * private GValue and then copy it if successful. */

	if (g_value_transform (&data_value, &real_value)) {
		g_value_init (value, class->value_type);
		g_value_copy (&real_value, value);
		success = TRUE;
	}

	g_value_unset (&data_value);
	g_value_unset (&real_value);

	return success;
}

static gchar *
gdav_pcdata_property_real_write_data (GDavPCDataProperty *property)
{
	GValue data_value = G_VALUE_INIT;
	GValue real_value = G_VALUE_INIT;
	gchar *data = NULL;

	g_value_init (&data_value, G_TYPE_STRING);
	gdav_property_get_value (GDAV_PROPERTY (property), &real_value);

	if (g_value_transform (&real_value, &data_value)) {
		data = g_value_dup_string (&data_value);
	} else {
		g_critical (
			"%s: %s is not transformable to a string",
			G_OBJECT_TYPE_NAME (property),
			G_VALUE_TYPE_NAME (&real_value));
	}

	g_value_unset (&data_value);
	g_value_unset (&real_value);

	return data;
}

static void
gdav_pcdata_property_class_init (GDavPCDataPropertyClass *class)
{
	GDavParsableClass *parsable_class;
	GDavPropertyClass *property_class;

	parsable_class = GDAV_PARSABLE_CLASS (class);
	parsable_class->serialize = gdav_pcdata_property_serialize;
	parsable_class->deserialize = gdav_pcdata_property_deserialize;

	property_class = GDAV_PROPERTY_CLASS (class);
	property_class->value_type = G_TYPE_STRING;

	class->allow_empty_data = FALSE;
	class->parse_data = gdav_pcdata_property_real_parse_data;
	class->write_data = gdav_pcdata_property_real_write_data;
}

static void
gdav_pcdata_property_init (GDavPCDataProperty *property)
{
}

GDavProperty *
gdav_pcdata_property_new_from_data (GType pcdata_property_type,
                                    const gchar *data)
{
	GDavPCDataProperty *property;
	GValue value = G_VALUE_INIT;

	g_return_val_if_fail (
		g_type_is_a (
			pcdata_property_type,
			GDAV_TYPE_PCDATA_PROPERTY), NULL);
	g_return_val_if_fail (
		!G_TYPE_IS_ABSTRACT (pcdata_property_type), NULL);
	g_return_val_if_fail (data != NULL, NULL);

	property = g_object_new (pcdata_property_type, NULL);

	if (gdav_pcdata_property_parse_data (property, data, &value)) {
		gdav_property_set_value (GDAV_PROPERTY (property), &value);
		g_value_unset (&value);
	} else {
		g_clear_object (&property);
	}

	/* Might be NULL so use a plain type cast. */
	return (GDavProperty *) property;
}

gboolean
gdav_pcdata_property_parse_data (GDavPCDataProperty *property,
                                 const gchar *data,
                                 GValue *value)
{
	GDavPCDataPropertyClass *class;

	g_return_val_if_fail (GDAV_IS_PCDATA_PROPERTY (property), FALSE);
	g_return_val_if_fail (data != NULL, FALSE);
	g_return_val_if_fail (value != NULL, FALSE);

	class = GDAV_PCDATA_PROPERTY_GET_CLASS (property);
	g_return_val_if_fail (class->parse_data != NULL, FALSE);

	return class->parse_data (property, data, value);
}

gchar *
gdav_pcdata_property_write_data (GDavPCDataProperty *property)
{
	GDavPCDataPropertyClass *class;

	g_return_val_if_fail (GDAV_IS_PCDATA_PROPERTY (property), NULL);

	class = GDAV_PCDATA_PROPERTY_GET_CLASS (property);
	g_return_val_if_fail (class->write_data != NULL, NULL);

	return class->write_data (property);
}


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

#include "gdav-prop-stat.h"

#include <glib/gi18n-lib.h>

#define GDAV_PROP_STAT_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE \
	((obj), GDAV_TYPE_PROP_STAT, GDavPropStatPrivate))

struct _GDavPropStatPrivate {
	GDavPropertySet *prop;
	GDavError *error;
	gchar *description;
	gchar *reason_phrase;
	guint status_code;
};

enum {
	PROP_0,
	PROP_DESCRIPTION,
	PROP_ERROR,
	PROP_PROP,
	PROP_STATUS
};

G_DEFINE_TYPE (GDavPropStat, gdav_prop_stat, GDAV_TYPE_PARSABLE)

static void
gdav_prop_stat_get_property (GObject *object,
                             guint property_id,
                             GValue *value,
                             GParamSpec *pspec)
{
	switch (property_id) {
		case PROP_DESCRIPTION:
			g_value_set_string (
				value,
				gdav_prop_stat_get_description (
				GDAV_PROP_STAT (value)));
			return;

		case PROP_ERROR:
			g_value_set_object (
				value,
				gdav_prop_stat_get_error (
				GDAV_PROP_STAT (value)));
			return;

		case PROP_PROP:
			g_value_set_object (
				value,
				gdav_prop_stat_get_prop (
				GDAV_PROP_STAT (value)));
			return;

		case PROP_STATUS:
			g_value_set_uint (
				value,
				gdav_prop_stat_get_status (
				GDAV_PROP_STAT (value), NULL));
			return;
	}

	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
gdav_prop_stat_dispose (GObject *object)
{
	GDavPropStatPrivate *priv;

	priv = GDAV_PROP_STAT_GET_PRIVATE (object);

	g_clear_object (&priv->prop);
	g_clear_object (&priv->error);

	/* Chain up to parent's dispose() method. */
	G_OBJECT_CLASS (gdav_prop_stat_parent_class)->dispose (object);
}

static void
gdav_prop_stat_finalize (GObject *object)
{
	GDavPropStatPrivate *priv;

	priv = GDAV_PROP_STAT_GET_PRIVATE (object);

	g_free (priv->description);
	g_free (priv->reason_phrase);

	/* Chain up to parent's finalize() method. */
	G_OBJECT_CLASS (gdav_prop_stat_parent_class)->finalize (object);
}

static gboolean
gdav_prop_stat_deserialize_dav (GDavParsable *parsable,
                                SoupURI *base_uri,
                                xmlDoc *doc,
                                xmlNode *node,
                                GError **error)
{
	GDavPropStatPrivate *priv;

	priv = GDAV_PROP_STAT_GET_PRIVATE (parsable);

	if (xmlStrcmp (node->name, BAD_CAST "prop") == 0) {
		GDavParsable *item;

		item = gdav_parsable_new_from_xml_node (
			GDAV_TYPE_PROPERTY_SET, base_uri, doc, node, error);
		if (item == NULL)
			return FALSE;

		g_clear_object (&priv->prop);
		priv->prop = GDAV_PROPERTY_SET (item);

		return TRUE;
	}

	if (xmlStrcmp (node->name, BAD_CAST "status") == 0) {
		xmlChar *text;
		gboolean success;

		text = xmlNodeListGetString (doc, node->children, TRUE);

		g_free (priv->reason_phrase);
		priv->reason_phrase = NULL;

		success = soup_headers_parse_status_line (
			(gchar *) text, NULL,
			&priv->status_code,
			&priv->reason_phrase);

		if (!success) {
			g_set_error (
				error, GDAV_PARSABLE_ERROR,
				GDAV_PARSABLE_ERROR_INTERNAL,
				_("Failed to parse status line '%s'"),
				(gchar *) text);
		}

		xmlFree (text);

		return success;
	}

	if (xmlStrcmp (node->name, BAD_CAST "error") == 0) {
		GDavParsable *item;

		item = gdav_parsable_new_from_xml_node (
			GDAV_TYPE_ERROR, base_uri, doc, node, error);
		if (item == NULL)
			return FALSE;

		g_clear_object (&priv->error);
		priv->error = GDAV_ERROR (item);

		return TRUE;
	}

	if (xmlStrcmp (node->name, BAD_CAST "responsedescription") == 0) {
		xmlChar *text;

		text = xmlNodeListGetString (doc, node->children, TRUE);

		g_free (priv->description);
		priv->description = (gchar *) text;

		return TRUE;
	}

	/* Chain up to parent's deserialize() method. */
	return GDAV_PARSABLE_CLASS (gdav_prop_stat_parent_class)->
		deserialize (parsable, base_uri, doc, node, error);
}

static gboolean
gdav_prop_stat_deserialize (GDavParsable *parsable,
                            SoupURI *base_uri,
                            xmlDoc *doc,
                            xmlNode *node,
                            GError **error)
{
	if (gdav_is_xmlns (node, GDAV_XMLNS_DAV)) {
		return gdav_prop_stat_deserialize_dav (
			parsable, base_uri, doc, node, error);
	}

	/* Chain up to parent's deserialize() method. */
	return GDAV_PARSABLE_CLASS (gdav_prop_stat_parent_class)->
		deserialize (parsable, base_uri, doc, node, error);
}

static void
gdav_prop_stat_collect_types (GDavParsable *parsable,
                              GHashTable *parsable_types)
{
	GDavPropStatPrivate *priv;

	priv = GDAV_PROP_STAT_GET_PRIVATE (parsable);

	if (priv->prop != NULL) {
		gdav_parsable_collect_types (
			GDAV_PARSABLE (priv->prop), parsable_types);
	}

	if (priv->error != NULL) {
		gdav_parsable_collect_types (
			GDAV_PARSABLE (priv->error), parsable_types);
	}
}

static void
gdav_prop_stat_class_init (GDavPropStatClass *class)
{
	GObjectClass *object_class;
	GDavParsableClass *parsable_class;

	g_type_class_add_private (class, sizeof (GDavPropStatPrivate));

	object_class = G_OBJECT_CLASS (class);
	object_class->get_property = gdav_prop_stat_get_property;
	object_class->dispose = gdav_prop_stat_dispose;
	object_class->finalize = gdav_prop_stat_finalize;

	parsable_class = GDAV_PARSABLE_CLASS (class);
	parsable_class->element_name = "propstat";
	parsable_class->element_namespace = GDAV_XMLNS_DAV;
	parsable_class->deserialize = gdav_prop_stat_deserialize;
	parsable_class->collect_types = gdav_prop_stat_collect_types;
}

static void
gdav_prop_stat_init (GDavPropStat *prop_stat)
{
	prop_stat->priv = GDAV_PROP_STAT_GET_PRIVATE (prop_stat);
}

GDavPropertySet *
gdav_prop_stat_get_prop (GDavPropStat *prop_stat)
{
	g_return_val_if_fail (GDAV_IS_PROP_STAT (prop_stat), NULL);

	return prop_stat->priv->prop;
}

guint
gdav_prop_stat_get_status (GDavPropStat *prop_stat,
                           gchar **reason_phrase)
{
	g_return_val_if_fail (GDAV_IS_PROP_STAT (prop_stat), 0);

	if (reason_phrase != NULL)
		*reason_phrase = g_strdup (prop_stat->priv->reason_phrase);

	return prop_stat->priv->status_code;
}

GDavError *
gdav_prop_stat_get_error (GDavPropStat *prop_stat)
{
	g_return_val_if_fail (GDAV_IS_PROP_STAT (prop_stat), NULL);

	return prop_stat->priv->error;
}

const gchar *
gdav_prop_stat_get_description (GDavPropStat *prop_stat)
{
	g_return_val_if_fail (GDAV_IS_PROP_STAT (prop_stat), NULL);

	return prop_stat->priv->description;
}


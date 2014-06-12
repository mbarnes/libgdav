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

#include "gdav-response.h"

#include <glib/gi18n-lib.h>

#define GDAV_RESPONSE_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE \
	((obj), GDAV_TYPE_RESPONSE, GDavResponsePrivate))

struct _GDavResponsePrivate {
	GPtrArray *hrefs;
	GPtrArray *propstats;
	GDavError *error;
	gchar *description;
	gchar *location;
	gchar *reason_phrase;
	guint status_code;
};

enum {
	PROP_0,
	PROP_DESCRIPTION,
	PROP_ERROR,
	PROP_LOCATION,
	PROP_STATUS
};

G_DEFINE_TYPE (GDavResponse, gdav_response, GDAV_TYPE_PARSABLE)

static void
gdav_response_get_property (GObject *object,
                            guint property_id,
                            GValue *value,
                            GParamSpec *pspec)
{
	switch (property_id) {
		case PROP_DESCRIPTION:
			g_value_set_string (
				value,
				gdav_response_get_description (
				GDAV_RESPONSE (object)));
			return;

		case PROP_ERROR:
			g_value_set_object (
				value,
				gdav_response_get_error (
				GDAV_RESPONSE (object)));
			return;

		case PROP_LOCATION:
			g_value_set_string (
				value,
				gdav_response_get_location (
				GDAV_RESPONSE (object)));
			return;

		case PROP_STATUS:
			g_value_set_uint (
				value,
				gdav_response_get_status (
				GDAV_RESPONSE (object), NULL));
			return;
	}

	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
gdav_response_dispose (GObject *object)
{
	GDavResponsePrivate *priv;

	priv = GDAV_RESPONSE_GET_PRIVATE (object);

	g_ptr_array_set_size (priv->hrefs, 0);
	g_ptr_array_set_size (priv->propstats, 0);
	g_clear_object (&priv->error);

	/* Chain up to parent's dispose() method. */
	G_OBJECT_CLASS (gdav_response_parent_class)->dispose (object);
}

static void
gdav_response_finalize (GObject *object)
{
	GDavResponsePrivate *priv;

	priv = GDAV_RESPONSE_GET_PRIVATE (object);

	g_ptr_array_free (priv->hrefs, TRUE);
	g_ptr_array_free (priv->propstats, TRUE);

	g_free (priv->description);
	g_free (priv->location);
	g_free (priv->reason_phrase);

	/* Chain up to parent's finalize() method. */
	G_OBJECT_CLASS (gdav_response_parent_class)->finalize (object);
}

static gboolean
gdav_response_deserialize_dav (GDavParsable *parsable,
                               SoupURI *base_uri,
                               xmlDoc *doc,
                               xmlNode *node,
                               GError **error)
{
	GDavResponsePrivate *priv;

	priv = GDAV_RESPONSE_GET_PRIVATE (parsable);

	/* Handle nodes in the GDAV_XMLNS_DAV namespace. */

	if (xmlStrcmp (node->name, BAD_CAST "href") == 0) {
		SoupURI *uri;

		uri = gdav_parsable_deserialize_href (
			parsable, base_uri, doc, node, error);

		if (uri == NULL)
			return FALSE;

		g_ptr_array_add (priv->hrefs, uri);

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

	if (xmlStrcmp (node->name, BAD_CAST "propstat") == 0) {
		GDavParsable *item;

		item = gdav_parsable_new_from_xml_node (
			GDAV_TYPE_PROP_STAT, base_uri, doc, node, error);
		if (item == NULL)
			return FALSE;

		g_ptr_array_add (priv->propstats, item);

		return TRUE;
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

	if (xmlStrcmp (node->name, BAD_CAST "location") == 0) {
		xmlChar *text;

		text = xmlNodeListGetString (doc, node->children, TRUE);

		g_free (priv->location);
		priv->location = (gchar *) text;

		return TRUE;
	}

	/* Chain up to parent's deserialize() method. */
	return GDAV_PARSABLE_CLASS (gdav_response_parent_class)->
		deserialize (parsable, base_uri, doc, node, error);
}

static gboolean
gdav_response_deserialize (GDavParsable *parsable,
                           SoupURI *base_uri,
                           xmlDoc *doc,
                           xmlNode *node,
                           GError **error)
{
	if (gdav_is_xmlns (node, GDAV_XMLNS_DAV)) {
		return gdav_response_deserialize_dav (
			parsable, base_uri, doc, node, error);
	}

	/* Chain up to parent's deserialize() method. */
	return GDAV_PARSABLE_CLASS (gdav_response_parent_class)->
		deserialize (parsable, base_uri, doc, node, error);
}

static void
gdav_response_collect_types (GDavParsable *parsable,
                             GHashTable *parsable_types)
{
	GDavResponsePrivate *priv;

	priv = GDAV_RESPONSE_GET_PRIVATE (parsable);

	g_ptr_array_foreach (
		priv->propstats,
		(GFunc) gdav_parsable_collect_types,
		parsable_types);

	if (priv->error != NULL) {
		gdav_parsable_collect_types (
			GDAV_PARSABLE (priv->error), parsable_types);
	}
}

static void
gdav_response_class_init (GDavResponseClass *class)
{
	GObjectClass *object_class;
	GDavParsableClass *parsable_class;

	g_type_class_add_private (class, sizeof (GDavResponsePrivate));

	object_class = G_OBJECT_CLASS (class);
	object_class->get_property = gdav_response_get_property;
	object_class->dispose = gdav_response_dispose;
	object_class->finalize = gdav_response_finalize;

	parsable_class = GDAV_PARSABLE_CLASS (class);
	parsable_class->element_name = "response";
	parsable_class->element_namespace = GDAV_XMLNS_DAV;
	parsable_class->deserialize = gdav_response_deserialize;
	parsable_class->collect_types = gdav_response_collect_types;
}

static void
gdav_response_init (GDavResponse *response)
{
	response->priv = GDAV_RESPONSE_GET_PRIVATE (response);

	response->priv->hrefs =
		g_ptr_array_new_with_free_func (
		(GDestroyNotify) soup_uri_free);

	response->priv->propstats =
		g_ptr_array_new_with_free_func (
		(GDestroyNotify) g_object_unref);
}

gboolean
gdav_response_has_href (GDavResponse *response,
                        SoupURI *uri)
{
	guint ii;

	g_return_val_if_fail (GDAV_IS_RESPONSE (response), FALSE);
	g_return_val_if_fail (uri != NULL, FALSE);

	for (ii = 0; ii < response->priv->hrefs->len; ii++) {
		if (soup_uri_equal (uri, response->priv->hrefs->pdata[ii]))
			return TRUE;
	}

	return FALSE;
}

GList *
gdav_response_list_hrefs (GDavResponse *response)
{
	GQueue queue = G_QUEUE_INIT;
	guint ii;

	g_return_val_if_fail (GDAV_IS_RESPONSE (response), NULL);

	for (ii = 0; ii < response->priv->hrefs->len; ii++) {
		SoupURI *href = response->priv->hrefs->pdata[ii];
		g_queue_push_tail (&queue, soup_uri_copy (href));
	}

	return g_queue_peek_head_link (&queue);
}

guint
gdav_response_get_status (GDavResponse *response,
                          gchar **reason_phrase)
{
	g_return_val_if_fail (GDAV_IS_RESPONSE (response), 0);

	if (reason_phrase != NULL)
		*reason_phrase = g_strdup (response->priv->reason_phrase);

	return response->priv->status_code;
}

GDavPropStat *
gdav_response_get_propstat (GDavResponse *response,
                            guint index)
{
	GDavPropStat *propstat = NULL;

	g_return_val_if_fail (GDAV_IS_RESPONSE (response), NULL);

	if (index < response->priv->propstats->len)
		propstat = response->priv->propstats->pdata[index];

	return propstat;
}

guint
gdav_response_get_n_propstats (GDavResponse *response)
{
	g_return_val_if_fail (GDAV_IS_RESPONSE (response), 0);

	return response->priv->propstats->len;
}

GDavError *
gdav_response_get_error (GDavResponse *response)
{
	g_return_val_if_fail (GDAV_IS_RESPONSE (response), NULL);

	return response->priv->error;
}

const gchar *
gdav_response_get_description (GDavResponse *response)
{
	g_return_val_if_fail (GDAV_IS_RESPONSE (response), NULL);

	return response->priv->description;
}

const gchar *
gdav_response_get_location (GDavResponse *response)
{
	g_return_val_if_fail (GDAV_IS_RESPONSE (response), NULL);

	return response->priv->location;
}

guint
gdav_response_find_property (GDavResponse *response,
                             GType property_type,
                             GValue *value,
                             gchar **reason_phrase)
{
	guint ii, n_propstats;

	g_return_val_if_fail (
		GDAV_IS_RESPONSE (response), SOUP_STATUS_NONE);
	g_return_val_if_fail (
		g_type_is_a (property_type, GDAV_TYPE_PROPERTY),
		SOUP_STATUS_NONE);

	n_propstats = gdav_response_get_n_propstats (response);

	for (ii = 0; ii < n_propstats; ii++) {
		GDavPropStat *propstat;
		GDavPropertySet *prop;
		GList *list = NULL;
		guint status;

		propstat = gdav_response_get_propstat (response, ii);
		prop = gdav_prop_stat_get_prop (propstat);

		if (!gdav_property_set_has_type (prop, property_type))
			continue;

		status = gdav_prop_stat_get_status (propstat, reason_phrase);

		if (status == SOUP_STATUS_OK && value != NULL)
			list = gdav_property_set_list (prop, property_type);

		if (list != NULL)
			gdav_property_get_value (list->data, value);

		g_list_free_full (list, (GDestroyNotify) g_object_unref);

		return status;
	}

	return SOUP_STATUS_NONE;
}


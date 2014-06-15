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

#include "gdav-multi-status.h"

#define GDAV_MULTI_STATUS_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE \
	((obj), GDAV_TYPE_MULTI_STATUS, GDavMultiStatusPrivate))

struct _GDavMultiStatusPrivate {
	GPtrArray *responses;
	gchar *description;
};

enum {
	PROP_0,
	PROP_DESCRIPTION
};

G_DEFINE_TYPE (GDavMultiStatus, gdav_multi_status, GDAV_TYPE_PARSABLE)

static void
gdav_multi_status_get_property (GObject *object,
                                guint property_id,
                                GValue *value,
                                GParamSpec *pspec)
{
	switch (property_id) {
		case PROP_DESCRIPTION:
			g_value_set_string (
				value,
				gdav_multi_status_get_description (
				GDAV_MULTI_STATUS (object)));
			return;
	}

	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
gdav_multi_status_dispose (GObject *object)
{
	GDavMultiStatusPrivate *priv;

	priv = GDAV_MULTI_STATUS_GET_PRIVATE (object);

	g_ptr_array_set_size (priv->responses, 0);

	/* Chain up to parent's dispose() method. */
	G_OBJECT_CLASS (gdav_multi_status_parent_class)->dispose (object);
}

static void
gdav_multi_status_finalize (GObject *object)
{
	GDavMultiStatusPrivate *priv;

	priv = GDAV_MULTI_STATUS_GET_PRIVATE (object);

	g_ptr_array_free (priv->responses, TRUE);
	g_free (priv->description);

	/* Chain up to parent's finalize() method. */
	G_OBJECT_CLASS (gdav_multi_status_parent_class)->finalize (object);
}

static gboolean
gdav_multi_status_deserialize_dav (GDavParsable *parsable,
                                   SoupURI *base_uri,
                                   xmlDoc *doc,
                                   xmlNode *node,
                                   GError **error)
{
	GDavMultiStatusPrivate *priv;

	priv = GDAV_MULTI_STATUS_GET_PRIVATE (parsable);

	/* Handle nodes in the GDAV_XMLNS_DAV namespace. */

	if (xmlStrcmp (node->name, BAD_CAST "response") == 0) {
		GDavParsable *item;

		item = gdav_parsable_new_from_xml_node (
			GDAV_TYPE_RESPONSE, base_uri, doc, node, error);
		if (item == NULL)
			return FALSE;

		g_ptr_array_add (priv->responses, item);

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
	return GDAV_PARSABLE_CLASS (gdav_multi_status_parent_class)->
		deserialize (parsable, base_uri, doc, node, error);
}

static gboolean
gdav_multi_status_deserialize (GDavParsable *parsable,
                               SoupURI *base_uri,
                               xmlDoc *doc,
                               xmlNode *node,
                               GError **error)
{
	if (gdav_is_xmlns (node, GDAV_XMLNS_DAV)) {
		return gdav_multi_status_deserialize_dav (
			parsable, base_uri, doc, node, error);
	}

	/* Chain up to parent's deserialize() method. */
	return GDAV_PARSABLE_CLASS (gdav_multi_status_parent_class)->
		deserialize (parsable, base_uri, doc, node, error);
}

static void
gdav_multi_status_collect_types (GDavParsable *parsable,
                                 GHashTable *parsable_types)
{
	GDavMultiStatusPrivate *priv;

	priv = GDAV_MULTI_STATUS_GET_PRIVATE (parsable);

	g_ptr_array_foreach (
		priv->responses,
		(GFunc) gdav_parsable_collect_types,
		parsable_types);
}

static void
gdav_multi_status_class_init (GDavMultiStatusClass *class)
{
	GObjectClass *object_class;
	GDavParsableClass *parsable_class;

	g_type_class_add_private (class, sizeof (GDavMultiStatusPrivate));

	object_class = G_OBJECT_CLASS (class);
	object_class->get_property = gdav_multi_status_get_property;
	object_class->dispose = gdav_multi_status_dispose;
	object_class->finalize = gdav_multi_status_finalize;

	parsable_class = GDAV_PARSABLE_CLASS (class);
	parsable_class->element_name = "multistatus";
	parsable_class->element_namespace = GDAV_XMLNS_DAV;
	parsable_class->deserialize = gdav_multi_status_deserialize;
	parsable_class->collect_types = gdav_multi_status_collect_types;
}

static void
gdav_multi_status_init (GDavMultiStatus *multi_status)
{
	multi_status->priv = GDAV_MULTI_STATUS_GET_PRIVATE (multi_status);

	multi_status->priv->responses =
		g_ptr_array_new_with_free_func (g_object_unref);
}

GDavMultiStatus *
gdav_multi_status_new_from_message (SoupMessage *message,
                                    GError **error)
{
	GDavMultiStatus *multi_status;

	g_return_val_if_fail (SOUP_IS_MESSAGE (message), NULL);

	if (message->status_code == SOUP_STATUS_MULTI_STATUS) {
		multi_status = gdav_parsable_new_from_data (
			GDAV_TYPE_MULTI_STATUS,
			soup_message_get_uri (message),
			message->response_body->data,
			message->response_body->length,
			error);
	} else {
		GDavResponse *response;

		multi_status = g_object_new (GDAV_TYPE_MULTI_STATUS, NULL);

		response = gdav_response_new_from_message (message);
		g_ptr_array_add (multi_status->priv->responses, response);
	}

	return multi_status;
}

gboolean
gdav_multi_status_has_errors (GDavMultiStatus *multi_status)
{
	guint ii, n_responses;

	g_return_val_if_fail (GDAV_IS_MULTI_STATUS (multi_status), FALSE);

	n_responses = gdav_multi_status_get_n_responses (multi_status);

	for (ii = 0; ii < n_responses; ii++) {
		GDavResponse *response;

		response = gdav_multi_status_get_response (multi_status, ii);

		if (gdav_response_has_errors (response))
			return TRUE;
	}

	return FALSE;
}

GDavResponse *
gdav_multi_status_get_response (GDavMultiStatus *multi_status,
                                guint index)
{
	GDavResponse *response = NULL;
	guint n_responses;

	g_return_val_if_fail (GDAV_IS_MULTI_STATUS (multi_status), NULL);

	n_responses = gdav_multi_status_get_n_responses (multi_status);

	if (index < n_responses)
		response = multi_status->priv->responses->pdata[index];

	return response;
}

GDavResponse *
gdav_multi_status_get_response_by_href (GDavMultiStatus *multi_status,
                                        SoupURI *uri)
{
	guint ii, n_responses;

	g_return_val_if_fail (GDAV_IS_MULTI_STATUS (multi_status), NULL);

	n_responses = gdav_multi_status_get_n_responses (multi_status);

	for (ii = 0; ii < n_responses; ii++) {
		GDavResponse *response;

		response = gdav_multi_status_get_response (multi_status, ii);
		if (gdav_response_has_href (response, uri))
			return response;
	}

	return NULL;
}

guint
gdav_multi_status_get_n_responses (GDavMultiStatus *multi_status)
{
	g_return_val_if_fail (GDAV_IS_MULTI_STATUS (multi_status), 0);

	return multi_status->priv->responses->len;
}

const gchar *
gdav_multi_status_get_description (GDavMultiStatus *multi_status)
{
	g_return_val_if_fail (GDAV_IS_MULTI_STATUS (multi_status), NULL);

	return multi_status->priv->description;
}


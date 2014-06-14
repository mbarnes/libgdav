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

#include "gdav-property-set.h"

#include "gdav-property.h"

#define GDAV_PROPERTY_SET_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE \
	((obj), GDAV_TYPE_PROPERTY_SET, GDavPropertySetPrivate))

struct _GDavPropertySetPrivate {
	GHashTable *property_types;
	GQueue property_values;
	gboolean names_only;
};

enum {
	PROP_0,
	PROP_NAMES_ONLY
};

G_DEFINE_TYPE (GDavPropertySet, gdav_property_set, GDAV_TYPE_PARSABLE)

static void
gdav_property_set_set_property (GObject *object,
                                guint property_id,
                                const GValue *value,
                                GParamSpec *pspec)
{
	switch (property_id) {
		case PROP_NAMES_ONLY:
			gdav_property_set_set_names_only (
				GDAV_PROPERTY_SET (object),
				g_value_get_boolean (value));
			return;
	}

	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
gdav_property_set_get_property (GObject *object,
                                guint property_id,
                                GValue *value,
                                GParamSpec *pspec)
{
	switch (property_id) {
		case PROP_NAMES_ONLY:
			g_value_set_boolean (
				value,
				gdav_property_set_get_names_only (
				GDAV_PROPERTY_SET (object)));
			return;
	}

	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
gdav_property_set_dispose (GObject *object)
{
	GDavPropertySetPrivate *priv;

	priv = GDAV_PROPERTY_SET_GET_PRIVATE (object);

	/* XXX GLib needs g_queue_clear_full() */
	while (!g_queue_is_empty (&priv->property_values))
		g_object_unref (g_queue_pop_head (&priv->property_values));

	/* Chain up to parent's dispose() method. */
	G_OBJECT_CLASS (gdav_property_set_parent_class)->dispose (object);
}

static void
gdav_property_set_finalize (GObject *object)
{
	GDavPropertySetPrivate *priv;

	priv = GDAV_PROPERTY_SET_GET_PRIVATE (object);

	g_hash_table_destroy (priv->property_types);

	/* Chain up to parent's finalize() method. */
	G_OBJECT_CLASS (gdav_property_set_parent_class)->finalize (object);
}

static gboolean
gdav_property_set_serialize (GDavParsable *parsable,
                             GHashTable *namespaces,
                             xmlDoc *doc,
                             xmlNode *parent,
                             GError **error)
{
	GDavPropertySetPrivate *priv;
	GHashTableIter iter;
	GList *list, *link;
	xmlNode *node;
	gpointer key;
	gboolean success = TRUE;

	priv = GDAV_PROPERTY_SET_GET_PRIVATE (parsable);

	node = gdav_parsable_new_text_child (
		G_OBJECT_TYPE (parsable), namespaces, parent, NULL);

	if (priv->names_only)
		goto names_only;

	list = g_queue_peek_head_link (&priv->property_values);

	for (link = list; link != NULL; link = g_list_next (link)) {
		success = gdav_parsable_serialize (
			link->data, namespaces, doc, node, error);
		if (!success)
			break;
	}

	return success;

names_only:

	g_hash_table_iter_init (&iter, priv->property_types);

	while (g_hash_table_iter_next (&iter, &key, NULL)) {
		GType type = GPOINTER_TO_SIZE (key);
		gdav_parsable_new_text_child (type, namespaces, node, NULL);
	}

	return TRUE;
}

static gboolean
gdav_property_set_deserialize (GDavParsable *parsable,
                               SoupURI *base_uri,
                               xmlDoc *doc,
                               xmlNode *node,
                               GError **error)
{
	GType type;

	/* Be lenient about unrecognized properties. */
	type = gdav_parsable_lookup_type (node, NULL);

	if (g_type_is_a (type, GDAV_TYPE_PROPERTY)) {
		GDavProperty *property;

		property = gdav_parsable_new_from_xml_node (
			type, base_uri, doc, node, error);
		if (property == NULL)
			return FALSE;

		gdav_property_set_add (
			GDAV_PROPERTY_SET (parsable), property);

		g_object_unref (property);

		return TRUE;
	}

	/* Chain up to parent's deserialize() method. */
	return GDAV_PARSABLE_CLASS (gdav_property_set_parent_class)->
		deserialize (parsable, base_uri, doc, node, error);
}

static void
gdav_property_set_collect_types (GDavParsable *parsable,
                                 GHashTable *parsable_types)
{
	GDavPropertySetPrivate *priv;
	GHashTableIter iter;
	gpointer key;

	priv = GDAV_PROPERTY_SET_GET_PRIVATE (parsable);

	/* When an item is added to 'property_values', its type
	 * is also added to 'property_types'.  So we don't need
	 * to iterate over 'property_values' here. */

	g_hash_table_iter_init (&iter, priv->property_types);

	while (g_hash_table_iter_next (&iter, &key, NULL))
		g_hash_table_add (parsable_types, key);
}

static void
gdav_property_set_class_init (GDavPropertySetClass *class)
{
	GObjectClass *object_class;
	GDavParsableClass *parsable_class;

	g_type_class_add_private (class, sizeof (GDavPropertySetPrivate));

	object_class = G_OBJECT_CLASS (class);
	object_class->set_property = gdav_property_set_set_property;
	object_class->get_property = gdav_property_set_get_property;
	object_class->dispose = gdav_property_set_dispose;
	object_class->finalize = gdav_property_set_finalize;

	parsable_class = GDAV_PARSABLE_CLASS (class);
	parsable_class->element_name = "prop";
	parsable_class->element_namespace = GDAV_XMLNS_DAV;
	parsable_class->serialize = gdav_property_set_serialize;
	parsable_class->deserialize = gdav_property_set_deserialize;
	parsable_class->collect_types = gdav_property_set_collect_types;

	g_object_class_install_property (
		object_class,
		PROP_NAMES_ONLY,
		g_param_spec_boolean (
			"names-only",
			"Names Only",
			"Whether to use only property "
			"names when serializing to XML",
			FALSE,
			G_PARAM_READWRITE |
			G_PARAM_CONSTRUCT |
			G_PARAM_STATIC_STRINGS));
}

static void
gdav_property_set_init (GDavPropertySet *propset)
{
	propset->priv = GDAV_PROPERTY_SET_GET_PRIVATE (propset);

	propset->priv->property_types = g_hash_table_new (NULL, NULL);
}

GDavPropertySet *
gdav_property_set_new (void)
{
	return g_object_new (GDAV_TYPE_PROPERTY_SET, NULL);
}

void
gdav_property_set_add_type (GDavPropertySet *propset,
                            GType property_type)
{
	g_return_if_fail (GDAV_IS_PROPERTY_SET (propset));
	g_return_if_fail (!G_TYPE_IS_ABSTRACT (property_type));
	g_return_if_fail (g_type_is_a (property_type, GDAV_TYPE_PROPERTY));

	g_hash_table_add (
		propset->priv->property_types,
		GSIZE_TO_POINTER (property_type));
}

gboolean
gdav_property_set_has_type (GDavPropertySet *propset,
                            GType property_type)
{
	gboolean has_type = FALSE;

	g_return_val_if_fail (GDAV_IS_PROPERTY_SET (propset), FALSE);

	if (g_type_is_a (property_type, GDAV_TYPE_PROPERTY)) {
		has_type = g_hash_table_contains (
			propset->priv->property_types,
			GSIZE_TO_POINTER (property_type));
	}

	return has_type;
}

void
gdav_property_set_add (GDavPropertySet *propset,
                       GDavProperty *property)
{
	g_return_if_fail (GDAV_IS_PROPERTY_SET (propset));
	g_return_if_fail (GDAV_IS_PROPERTY (property));

	gdav_property_set_add_type (propset, G_OBJECT_TYPE (property));

	g_queue_push_tail (
		&propset->priv->property_values,
		g_object_ref (property));
}

GList *
gdav_property_set_list (GDavPropertySet *propset,
                        GType property_type)
{
	GQueue matches = G_QUEUE_INIT;
	GList *list, *link;

	g_return_val_if_fail (GDAV_IS_PROPERTY_SET (propset), NULL);

	/* XXX Return new property references in case we want
	 *     to make this function thread-safe in the future. */

	list = g_queue_peek_head_link (&propset->priv->property_values);

	for (link = list; link != NULL; link = g_list_next (link)) {
		GDavProperty *property;

		property = GDAV_PROPERTY (link->data);

		if (g_type_is_a (G_OBJECT_TYPE (property), property_type)) {
			g_object_ref (property);
			g_queue_push_tail (&matches, property);
		}
	}

	return g_queue_peek_head_link (&matches);
}

GList *
gdav_property_set_list_all (GDavPropertySet *propset)
{
	GList *list;

	g_return_val_if_fail (GDAV_IS_PROPERTY_SET (propset), NULL);

	/* XXX Return new property references in case we want
	 *     to make this function thread-safe in the future. */

	list = g_queue_peek_head_link (&propset->priv->property_values);
	list = g_list_copy_deep (list, (GCopyFunc) g_object_ref, NULL);

	return list;
}

gboolean
gdav_property_set_get_names_only (GDavPropertySet *propset)
{
	g_return_val_if_fail (GDAV_IS_PROPERTY_SET (propset), FALSE);

	return propset->priv->names_only;
}

void
gdav_property_set_set_names_only (GDavPropertySet *propset,
                                  gboolean names_only)
{
	g_return_if_fail (GDAV_IS_PROPERTY_SET (propset));

	if (names_only != propset->priv->names_only) {
		propset->priv->names_only = names_only;
		g_object_notify (G_OBJECT (propset), "names-only");
	}
}


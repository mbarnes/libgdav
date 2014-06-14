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

#include "gdav-property-update.h"

#include "gdav-property-set.h"

#define GDAV_PROPERTY_UPDATE_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE \
	((obj), GDAV_TYPE_PROPERTY_UPDATE, GDavPropertyUpdatePrivate))

struct _GDavPropertyUpdatePrivate {
	GQueue propsets;
};

G_DEFINE_TYPE (GDavPropertyUpdate, gdav_property_update, GDAV_TYPE_PARSABLE)

static void
gdav_property_update_dispose (GObject *object)
{
	GDavPropertyUpdatePrivate *priv;

	priv = GDAV_PROPERTY_UPDATE_GET_PRIVATE (object);

	/* XXX GLib needs g_queue_clear_full() */
	while (!g_queue_is_empty (&priv->propsets))
		g_object_unref (g_queue_pop_head (&priv->propsets));

	/* Chain up to parent's dispose() method. */
	G_OBJECT_CLASS (gdav_property_update_parent_class)->dispose (object);
}

static gboolean
gdav_property_update_serialize (GDavParsable *parsable,
                                GHashTable *namespaces,
                                xmlDoc *doc,
                                xmlNode *parent,
                                GError **error)
{
	GDavPropertyUpdatePrivate *priv;
	GList *list, *link;
	xmlNs *nsdav;
	gboolean success = TRUE;

	/* The parent node is the <propertyupdate> element. */

	priv = GDAV_PROPERTY_UPDATE_GET_PRIVATE (parsable);

	nsdav = g_hash_table_lookup (namespaces, GDAV_XMLNS_DAV);

	list = g_queue_peek_head_link (&priv->propsets);

	for (link = list; link != NULL; link = g_list_next (link)) {
		GDavPropertySet *propset = link->data;
		xmlNode *node;
		xmlChar *name;

		/* names-only: TRUE means <remove>, FALSE means <set> */

		if (gdav_property_set_get_names_only (propset))
			name = BAD_CAST "remove";
		else
			name = BAD_CAST "set";

		node = xmlNewTextChild (parent, nsdav, name, NULL);

		success = gdav_parsable_serialize (
			GDAV_PARSABLE (propset),
			namespaces, doc, node, error);

		if (!success)
			break;
	}

	return success;
}

static void
gdav_property_update_collect_types (GDavParsable *parsable,
                                    GHashTable *parsable_types)
{
	GDavPropertyUpdatePrivate *priv;

	priv = GDAV_PROPERTY_UPDATE_GET_PRIVATE (parsable);

	g_queue_foreach (
		&priv->propsets,
		(GFunc) gdav_parsable_collect_types,
		parsable_types);
}

static void
gdav_property_update_class_init (GDavPropertyUpdateClass *class)
{
	GObjectClass *object_class;
	GDavParsableClass *parsable_class;

	g_type_class_add_private (class, sizeof (GDavPropertyUpdatePrivate));

	object_class = G_OBJECT_CLASS (class);
	object_class->dispose = gdav_property_update_dispose;

	parsable_class = GDAV_PARSABLE_CLASS (class);
	parsable_class->element_name = "propertyupdate";
	parsable_class->element_namespace = GDAV_XMLNS_DAV;
	parsable_class->serialize = gdav_property_update_serialize;
	parsable_class->collect_types = gdav_property_update_collect_types;
}

static void
gdav_property_update_init (GDavPropertyUpdate *update)
{
	update->priv = GDAV_PROPERTY_UPDATE_GET_PRIVATE (update);
}

GDavPropertyUpdate *
gdav_property_update_new (void)
{
	return g_object_new (GDAV_TYPE_PROPERTY_UPDATE, NULL);
}

void
gdav_property_update_set (GDavPropertyUpdate *update,
                          GDavProperty *property)
{
	GDavPropertySet *propset;
	gboolean need_new_propset;

	g_return_if_fail (GDAV_IS_PROPERTY_UPDATE (update));
	g_return_if_fail (GDAV_IS_PROPERTY (property));

	/* Add to the most recent GDavPropertySet, if we can. */

	propset = g_queue_peek_tail (&update->priv->propsets);

	/* names-only: TRUE means <remove>, FALSE means <set> */

	need_new_propset =
		(propset == NULL) ||
		gdav_property_set_get_names_only (propset);

	if (need_new_propset) {
		propset = gdav_property_set_new ();
		gdav_property_set_set_names_only (propset, FALSE);
		g_queue_push_tail (&update->priv->propsets, propset);
	}

	gdav_property_set_add (propset, property);
}

void
gdav_property_update_remove (GDavPropertyUpdate *update,
                             GType property_type)
{
	GDavPropertySet *propset;
	gboolean need_new_propset;

	g_return_if_fail (GDAV_IS_PROPERTY_UPDATE (update));
	g_return_if_fail (!G_TYPE_IS_ABSTRACT (property_type));
	g_return_if_fail (g_type_is_a (property_type, GDAV_TYPE_PROPERTY));

	/* Add to the most recent GDavPropertySet, if we can. */

	propset = g_queue_peek_tail (&update->priv->propsets);

	/* names-only: TRUE means <remove>, FALSE means <set> */

	need_new_propset =
		(propset == NULL) ||
		!gdav_property_set_get_names_only (propset);

	if (need_new_propset) {
		propset = gdav_property_set_new ();
		gdav_property_set_set_names_only (propset, TRUE);
		g_queue_push_tail (&update->priv->propsets, propset);
	}

	gdav_property_set_add_type (propset, property_type);
}


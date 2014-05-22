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

#define GDAV_PROPERTY_UPDATE_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE \
	((obj), GDAV_TYPE_PROPERTY_UPDATE, GDavPropertyUpdatePrivate))

struct _GDavPropertyUpdatePrivate {
	GQueue instructions;
};

typedef struct {
	GType remove;
	GDavProperty *set;
} GDavInstruction;

G_DEFINE_TYPE (GDavPropertyUpdate, gdav_property_update, GDAV_TYPE_PARSABLE)

static GDavInstruction *
gdav_instruction_new (void)
{
	return g_slice_new0 (GDavInstruction);
}

static void
gdav_instruction_free (GDavInstruction *instruction)
{
	g_clear_object (&instruction->set);
	g_slice_free (GDavInstruction, instruction);
}

static void
gdav_property_update_dispose (GObject *object)
{
	GDavPropertyUpdatePrivate *priv;

	priv = GDAV_PROPERTY_UPDATE_GET_PRIVATE (object);

	/* XXX GLib needs g_queue_clear_full() */
	while (!g_queue_is_empty (&priv->instructions)) {
		GDavInstruction *instruction;
		instruction = g_queue_pop_head (&priv->instructions);
		gdav_instruction_free (instruction);
	}

	/* Chain up to parent's dispose() method. */
	G_OBJECT_CLASS (gdav_property_update_parent_class)->dispose (object);
}

static void
gdav_property_update_collect_types (GDavParsable *parsable,
                                    GHashTable *parsable_types)
{
	GDavPropertyUpdatePrivate *priv;
	GList *list, *link;

	priv = GDAV_PROPERTY_UPDATE_GET_PRIVATE (parsable);

	list = g_queue_peek_head_link (&priv->instructions);

	for (link = list; link != NULL; link = g_list_next (link)) {
		GDavInstruction *instruction = link->data;

		if (instruction->set != NULL) {
			gdav_parsable_collect_types (
				GDAV_PARSABLE (instruction->set),
				parsable_types);
		} else {
			g_hash_table_add (
				parsable_types,
				GSIZE_TO_POINTER (instruction->remove));
		}
	}
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
	GDavInstruction *instruction;

	g_return_if_fail (GDAV_IS_PROPERTY_UPDATE (update));
	g_return_if_fail (GDAV_IS_PROPERTY (property));

	instruction = gdav_instruction_new ();
	instruction->set = g_object_ref (property);

	g_queue_push_tail (&update->priv->instructions, instruction);
}

void
gdav_property_update_remove (GDavPropertyUpdate *update,
                             GType property_type)
{
	GDavInstruction *instruction;

	g_return_if_fail (GDAV_IS_PROPERTY_UPDATE (update));
	g_return_if_fail (!G_TYPE_IS_ABSTRACT (property_type));
	g_return_if_fail (g_type_is_a (property_type, GDAV_TYPE_PROPERTY));

	instruction = gdav_instruction_new ();
	instruction->remove = property_type;

	g_queue_push_tail (&update->priv->instructions, instruction);
}


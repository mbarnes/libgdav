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

#include "gdav-list-property.h"

G_DEFINE_ABSTRACT_TYPE (
	GDavListProperty,
	gdav_list_property,
	GDAV_TYPE_PROPERTY)

static void
gdav_list_property_constructed (GObject *object)
{
	GArray *array;
	GValue value = G_VALUE_INIT;

	/* Chain up first, or gdav_property_set_value() won't work. */
	G_OBJECT_CLASS (gdav_list_property_parent_class)->constructed (object);

	array = g_array_new (FALSE, TRUE, sizeof (GValue));
	g_array_set_clear_func (array, (GDestroyNotify) g_value_unset);

	g_value_init (&value, G_TYPE_ARRAY);
	g_value_take_boxed (&value, array);
	gdav_property_set_value (GDAV_PROPERTY (object), &value);
	g_value_unset (&value);
}

static void
gdav_list_property_class_init (GDavListPropertyClass *class)
{
	GObjectClass *object_class;
	GDavPropertyClass *property_class;

	object_class = G_OBJECT_CLASS (class);
	object_class->constructed = gdav_list_property_constructed;

	property_class = GDAV_PROPERTY_CLASS (class);
	property_class->value_type = G_TYPE_ARRAY;
}

static void
gdav_list_property_init (GDavListProperty *property)
{
}

guint
gdav_list_property_length (GDavListProperty *property)
{
	GValue array_value = G_VALUE_INIT;
	GArray *array;
	guint length;

	g_return_val_if_fail (GDAV_IS_LIST_PROPERTY (property), 0);

	gdav_property_get_value (GDAV_PROPERTY (property), &array_value);
	array = g_value_get_boxed (&array_value);
	g_return_val_if_fail (array != NULL, 0);
	length = array->len;
	g_value_unset (&array_value);

	return length;
}

void
gdav_list_property_add_value (GDavListProperty *property,
                              const GValue *value)
{
	GDavListPropertyClass *class;
	GValue array_value = G_VALUE_INIT;
	GArray *array;

	g_return_if_fail (GDAV_IS_LIST_PROPERTY (property));
	g_return_if_fail (value != NULL);

	class = GDAV_LIST_PROPERTY_GET_CLASS (property);
	g_return_if_fail (G_VALUE_HOLDS (value, class->element_type));

	gdav_property_get_value (GDAV_PROPERTY (property), &array_value);
	array = g_value_get_boxed (&array_value);
	g_return_if_fail (array != NULL);
	g_array_append_val (array, value);
	g_value_unset (&array_value);
}

gboolean
gdav_list_property_get_value (GDavListProperty *property,
                              guint index,
                              GValue *value)
{
	GValue array_value = G_VALUE_INIT;
	GArray *array;
	gboolean success = FALSE;

	g_return_val_if_fail (GDAV_IS_LIST_PROPERTY (property), FALSE);
	g_return_val_if_fail (value != NULL, FALSE);

	gdav_property_get_value (GDAV_PROPERTY (property), &array_value);
	array = g_value_get_boxed (&array_value);
	g_return_val_if_fail (array != NULL, FALSE);
	if (index < array->len) {
		const GValue *src_value;

		src_value = &g_array_index (array, GValue, index);
		g_value_init (value, G_VALUE_TYPE (src_value));
		g_value_copy (src_value, value);

		success = TRUE;
	}
	g_value_unset (&array_value);

	return success;
}


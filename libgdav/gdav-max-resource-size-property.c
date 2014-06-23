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

#include "gdav-max-resource-size-property.h"

G_DEFINE_TYPE (
	GDavMaxResourceSizeProperty,
	gdav_max_resource_size_property,
	GDAV_TYPE_UINT_PROPERTY)

static void
gdav_max_resource_size_property_class_init (GDavUIntPropertyClass *class)
{
	GDavParsableClass *parsable_class;

	parsable_class = GDAV_PARSABLE_CLASS (class);
	parsable_class->element_name = "max-resource-size";
	parsable_class->element_namespace = GDAV_XMLNS_CALDAV;
}

static void
gdav_max_resource_size_property_init (GDavUIntProperty *property)
{
}

GDavProperty *
gdav_max_resource_size_property_new (guint64 prop_value)
{
	GDavProperty *property;
	GValue value = G_VALUE_INIT;

	g_value_init (&value, G_TYPE_UINT64);
	g_value_set_uint64 (&value, prop_value);

	property = g_object_new (
		GDAV_TYPE_MAX_RESOURCE_SIZE_PROPERTY,
		"value", &value, NULL);

	g_value_unset (&value);

	return property;
}


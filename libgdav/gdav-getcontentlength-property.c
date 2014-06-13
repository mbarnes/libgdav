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

#include "gdav-getcontentlength-property.h"

G_DEFINE_TYPE (
	GDavGetContentLengthProperty,
	gdav_getcontentlength_property,
	GDAV_TYPE_PROPERTY)

static void
gdav_getcontentlength_property_class_init (GDavPropertyClass *class)
{
	GDavParsableClass *parsable_class;

	parsable_class = GDAV_PARSABLE_CLASS (class);
	parsable_class->element_name = "getcontentlength";
	parsable_class->element_namespace = GDAV_XMLNS_DAV;

	class->value_type = G_TYPE_UINT64;
}

static void
gdav_getcontentlength_property_init (GDavProperty *property)
{
}

GDavProperty *
gdav_getcontentlength_property_new (guint64 prop_value)
{
	GDavProperty *property;
	GValue value = G_VALUE_INIT;

	g_value_init (&value, G_TYPE_UINT64);
	g_value_set_uint64 (&value, prop_value);

	property = g_object_new (
		GDAV_TYPE_GETCONTENTLENGTH_PROPERTY,
		"value", &value, NULL);

	g_value_unset (&value);

	return property;
}


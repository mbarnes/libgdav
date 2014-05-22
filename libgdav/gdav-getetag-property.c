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

#include "gdav-getetag-property.h"

G_DEFINE_TYPE (
	GDavGetETagProperty,
	gdav_getetag_property,
	GDAV_TYPE_PROPERTY)

static void
gdav_getetag_property_class_init (GDavGetETagPropertyClass *class)
{
	GDavParsableClass *parsable_class;
	GDavPropertyClass *property_class;

	parsable_class = GDAV_PARSABLE_CLASS (class);
	parsable_class->element_name = "getetag";
	parsable_class->element_namespace = GDAV_XMLNS_DAV;

	property_class = GDAV_PROPERTY_CLASS (class);
	property_class->value_type = G_TYPE_STRING;
}

static void
gdav_getetag_property_init (GDavGetETagProperty *property)
{
}

GDavProperty *
gdav_getetag_property_new (const gchar *prop_value)
{
	GDavProperty *property;
	GValue value = G_VALUE_INIT;

	g_value_init (&value, G_TYPE_STRING);
	g_value_set_string (&value, prop_value);

	property = g_object_new (
		GDAV_TYPE_GETETAG_PROPERTY,
		"value", &value, NULL);

	g_value_unset (&value);

	return property;
}


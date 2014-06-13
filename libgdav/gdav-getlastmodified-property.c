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

#include "gdav-getlastmodified-property.h"

G_DEFINE_TYPE (
	GDavGetLastModifiedProperty,
	gdav_getlastmodified_property,
	GDAV_TYPE_DATE_PROPERTY)

static void
gdav_getlastmodified_property_class_init (GDavDatePropertyClass *class)
{
	GDavParsableClass *parsable_class;

	parsable_class = GDAV_PARSABLE_CLASS (class);
	parsable_class->element_name = "getlastmodified";
	parsable_class->element_namespace = GDAV_XMLNS_DAV;

	class->format = SOUP_DATE_HTTP;
}

static void
gdav_getlastmodified_property_init (GDavDateProperty *property)
{
}

GDavProperty *
gdav_getlastmodified_property_new (SoupDate *prop_value)
{
	GDavProperty *property;
	GValue value = G_VALUE_INIT;

	g_return_val_if_fail (prop_value != NULL, NULL);

	g_value_init (&value, SOUP_TYPE_DATE);
	g_value_set_boxed (&value, prop_value);

	property = g_object_new (
		GDAV_TYPE_GETLASTMODIFIED_PROPERTY,
		"value", &value, NULL);

	g_value_unset (&value);

	return property;
}


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

#include "gdav-calendar-timezone-property.h"

G_DEFINE_TYPE (
	GDavCalendarTimeZoneProperty,
	gdav_calendar_timezone_property,
	GDAV_TYPE_PROPERTY)

static void
gdav_calendar_timezone_property_class_init (GDavPropertyClass *class)
{
	GDavParsableClass *parsable_class;

	parsable_class = GDAV_PARSABLE_CLASS (class);
	parsable_class->element_name = "calendar-timezone";
	parsable_class->element_namespace = GDAV_XMLNS_CALDAV;

	class->value_type = G_TYPE_STRING;
}

static void
gdav_calendar_timezone_property_init (GDavProperty *property)
{
}

GDavProperty *
gdav_calendar_timezone_property_new (const gchar *prop_value)
{
	GDavProperty *property;
	GValue value = G_VALUE_INIT;

	g_return_val_if_fail (prop_value != NULL, NULL);

	g_value_init (&value, G_TYPE_STRING);
	g_value_set_string (&value, prop_value);

	property = g_object_new (
		GDAV_TYPE_CALENDAR_TIMEZONE_PROPERTY,
		"value", &value, NULL);

	g_value_unset (&value);

	return property;
}


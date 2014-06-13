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

#include "gdav-date-property.h"

#include "gdav-utils.h"

G_DEFINE_ABSTRACT_TYPE (
	GDavDateProperty,
	gdav_date_property,
	GDAV_TYPE_PCDATA_PROPERTY)

static gboolean
gdav_date_property_parse_data (GDavPCDataProperty *property,
                               const gchar *data,
                               GValue *value)
{
	SoupDate *soup_date;
	gboolean success = FALSE;

	soup_date = soup_date_new_from_string (data);

	if (soup_date != NULL) {
		g_value_init (value, SOUP_TYPE_DATE);
		g_value_take_boxed (value, soup_date);
		success = TRUE;
	}

	return success;
}

static gchar *
gdav_date_property_write_data (GDavPCDataProperty *property)
{
	GValue value = G_VALUE_INIT;
	SoupDate *soup_date;
	gchar *data = NULL;

	gdav_property_get_value (GDAV_PROPERTY (property), &value);
	soup_date = g_value_get_boxed (&value);

	if (soup_date != NULL) {
		GDavDatePropertyClass *class;

		class = GDAV_DATE_PROPERTY_GET_CLASS (property);
		data = soup_date_to_string (soup_date, class->format);
	}

	g_value_unset (&value);

	return data;
}

static void
gdav_date_property_class_init (GDavDatePropertyClass *class)
{
	GDavPropertyClass *property_class;
	GDavPCDataPropertyClass *pcdata_property_class;

	property_class = GDAV_PROPERTY_CLASS (class);
	property_class->value_type = SOUP_TYPE_DATE;

	pcdata_property_class = GDAV_PCDATA_PROPERTY_CLASS (class);
	pcdata_property_class->parse_data = gdav_date_property_parse_data;
	pcdata_property_class->write_data = gdav_date_property_write_data;

	class->format = SOUP_DATE_ISO8601;
}

static void
gdav_date_property_init (GDavDateProperty *property)
{
}


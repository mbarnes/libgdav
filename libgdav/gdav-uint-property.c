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

#include <errno.h>

#include "gdav-uint-property.h"

G_DEFINE_ABSTRACT_TYPE (
	GDavUIntProperty,
	gdav_uint_property,
	GDAV_TYPE_PCDATA_PROPERTY)

static gboolean
gdav_uint_property_parse_data (GDavPCDataProperty *property,
                               const gchar *data,
                               GValue *value)
{
	guint64 uint64_value;
	gchar *end_ptr;
	gboolean success;

	errno = 0;
	uint64_value = g_ascii_strtoull (data, &end_ptr, 10);
	success = (errno == 0 && end_ptr > data);

	if (success) {
		GDavUIntPropertyClass *class;

		class = GDAV_UINT_PROPERTY_GET_CLASS (property);

		/* Apply min/max constraints. */
		uint64_value = CLAMP (
			uint64_value,
			class->minimum_value,
			class->maximum_value);

		g_value_init (value, G_TYPE_UINT64);
		g_value_set_uint64 (value, uint64_value);
	}

	return success;
}

static void
gdav_uint_property_class_init (GDavUIntPropertyClass *class)
{
	GDavPropertyClass *property_class;
	GDavPCDataPropertyClass *pcdata_property_class;

	property_class = GDAV_PROPERTY_CLASS (class);
	property_class->value_type = G_TYPE_UINT64;

	/* GLib already knows how to transform a G_TYPE_UINT64 value to a
	 * G_TYPE_STRING value, so we don't need to override write_data(). */
	pcdata_property_class = GDAV_PCDATA_PROPERTY_CLASS (class);
	pcdata_property_class->parse_data = gdav_uint_property_parse_data;

	class->minimum_value = 0;
	class->maximum_value = G_MAXUINT64;
}

static void
gdav_uint_property_init (GDavUIntProperty *property)
{
}


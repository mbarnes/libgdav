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

#include "gdav-schedule-outbox-url-property.h"

G_DEFINE_TYPE (
	GDavScheduleOutboxUrlProperty,
	gdav_schedule_outbox_url_property,
	GDAV_TYPE_HREF_PROPERTY)

static void
gdav_schedule_outbox_url_property_class_init (GDavPropertyClass *class)
{
	GDavParsableClass *parsable_class;

	parsable_class = GDAV_PARSABLE_CLASS (class);
	parsable_class->element_name = "schedule-outbox-URL";
	parsable_class->element_namespace = GDAV_XMLNS_CALDAV;
}

static void
gdav_schedule_outbox_url_property_init (GDavProperty *property)
{
}


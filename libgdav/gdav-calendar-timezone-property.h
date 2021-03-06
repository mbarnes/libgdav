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

#ifndef __GDAV_CALENDAR_TIMEZONE_PROPERTY_H__
#define __GDAV_CALENDAR_TIMEZONE_PROPERTY_H__

#include <libgdav/gdav-property.h>

/* Standard GObject macros */
#define GDAV_TYPE_CALENDAR_TIMEZONE_PROPERTY \
	(gdav_calendar_timezone_property_get_type ())
#define GDAV_CALENDAR_TIMEZONE_PROPERTY(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), GDAV_TYPE_CALENDAR_TIMEZONE_PROPERTY, GDavCalendarTimeZoneProperty))
#define GDAV_IS_CALENDAR_TIMEZONE_PROPERTY(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), GDAV_TYPE_CALENDAR_TIMEZONE_PROPERTY))

G_BEGIN_DECLS

typedef struct _GDavCalendarTimeZoneProperty GDavCalendarTimeZoneProperty;
typedef struct _GDavCalendarTimeZonePropertyClass GDavCalendarTimeZonePropertyClass;
typedef struct _GDavCalendarTimeZonePropertyPrivate GDavCalendarTimeZonePropertyPrivate;

struct _GDavCalendarTimeZoneProperty {
	GDavProperty parent;
	GDavCalendarTimeZonePropertyPrivate *priv;
};

struct _GDavCalendarTimeZonePropertyClass {
	GDavPropertyClass parent_class;
};

GType		gdav_calendar_timezone_property_get_type
					(void) G_GNUC_CONST;
GDavProperty *	gdav_calendar_timezone_property_new
					(const gchar *prop_value);

G_END_DECLS

#endif /* __GDAV_CALENDAR_TIMEZONE_PROPERTY_H__ */


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

#ifndef __GDAV_DATE_PROPERTY_H__
#define __GDAV_DATE_PROPERTY_H__

#include <libgdav/gdav-pcdata-property.h>

/* Standard GObject macros */
#define GDAV_TYPE_DATE_PROPERTY \
	(gdav_date_property_get_type ())
#define GDAV_DATE_PROPERTY(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), GDAV_TYPE_DATE_PROPERTY, GDavDateProperty))
#define GDAV_DATE_PROPERTY_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_CAST \
	((cls), GDAV_TYPE_DATE_PROPERTY, GDavDatePropertyClass))
#define GDAV_IS_DATE_PROPERTY(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), GDAV_TYPE_DATE_PROPERTY))
#define GDAV_IS_DATE_PROPERTY_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_TYPE \
	((cls), GDAV_TYPE_DATE_PROPERTY))
#define GDAV_DATE_PROPERTY_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS \
	((obj), GDAV_TYPE_DATE_PROPERTY, GDavDatePropertyClass))

G_BEGIN_DECLS

typedef struct _GDavDateProperty GDavDateProperty;
typedef struct _GDavDatePropertyClass GDavDatePropertyClass;

struct _GDavDateProperty {
	GDavPCDataProperty parent;
};

struct _GDavDatePropertyClass {
	GDavPCDataPropertyClass parent_class;

	SoupDateFormat format;
};

GType		gdav_date_property_get_type	(void) G_GNUC_CONST;

#endif /* __GDAV_DATE_PROPERTY_H__ */


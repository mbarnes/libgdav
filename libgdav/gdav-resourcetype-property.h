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

#ifndef __GDAV_RESOURCETYPE_PROPERTY_H__
#define __GDAV_RESOURCETYPE_PROPERTY_H__

#include <libgdav/gdav-enums.h>
#include <libgdav/gdav-property.h>

/* Standard GObject macros */
#define GDAV_TYPE_RESOURCETYPE_PROPERTY \
	(gdav_resourcetype_property_get_type ())
#define GDAV_RESOURCETYPE_PROPERTY(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), GDAV_TYPE_RESOURCETYPE_PROPERTY, GDavResourceTypeProperty))
#define GDAV_IS_RESOURCETYPE_PROPERTY(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), GDAV_TYPE_RESOURCETYPE_PROPERTY))

G_BEGIN_DECLS

typedef struct _GDavResourceTypeProperty GDavResourceTypeProperty;
typedef struct _GDavResourceTypePropertyClass GDavResourceTypePropertyClass;
typedef struct _GDavResourceTypePropertyPrivate GDavResourceTypePropertyPrivate;

struct _GDavResourceTypeProperty {
	GDavProperty parent;
	GDavResourceTypePropertyPrivate *priv;
};

struct _GDavResourceTypePropertyClass {
	GDavPropertyClass parent_class;
};

GType		gdav_resourcetype_property_get_type
					(void) G_GNUC_CONST;
GDavProperty *	gdav_resourcetype_property_new
					(GDavResourceType prop_value);

#endif /* __GDAV_RESOURCETYPE_PROPERTY_H__ */


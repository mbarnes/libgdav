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

#ifndef __GDAV_GETLASTMODIFIED_PROPERTY_H__
#define __GDAV_GETLASTMODIFIED_PROPERTY_H__

#include <libgdav/gdav-property.h>

/* Standard GObject macros */
#define GDAV_TYPE_GETLASTMODIFIED_PROPERTY \
	(gdav_getlastmodified_property_get_type ())
#define GDAV_GETLASTMODIFIED_PROPERTY(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), GDAV_TYPE_GETLASTMODIFIED_PROPERTY, GDavGetLastModifiedProperty))
#define GDAV_IS_GETLASTMODIFIED_PROPERTY(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), GDAV_TYPE_GETLASTMODIFIED_PROPERTY))

G_BEGIN_DECLS

typedef struct _GDavGetLastModifiedProperty GDavGetLastModifiedProperty;
typedef struct _GDavGetLastModifiedPropertyClass GDavGetLastModifiedPropertyClass;
typedef struct _GDavGetLastModifiedPropertyPrivate GDavGetLastModifiedPropertyPrivate;

struct _GDavGetLastModifiedProperty {
	GDavProperty parent;
	GDavGetLastModifiedPropertyPrivate *priv;
};

struct _GDavGetLastModifiedPropertyClass {
	GDavPropertyClass parent_class;
};

GType		gdav_getlastmodified_property_get_type
					(void) G_GNUC_CONST;
GDavProperty *	gdav_getlastmodified_property_new
					(GDateTime *prop_value);

G_END_DECLS

#endif /* __GDAV_GETLASTMODIFIED_PROPERTY_H__ */


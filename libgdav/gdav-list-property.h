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

#ifndef __GDAV_LIST_PROPERTY_H__
#define __GDAV_LIST_PROPERTY_H__

#include <libgdav/gdav-property.h>

/* Standard GObject macros */
#define GDAV_TYPE_LIST_PROPERTY \
	(gdav_list_property_get_type ())
#define GDAV_LIST_PROPERTY(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), GDAV_TYPE_LIST_PROPERTY, GDavListProperty))
#define GDAV_LIST_PROPERTY_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_CAST \
	((cls), GDAV_TYPE_LIST_PROPERTY, GDavListPropertyClass))
#define GDAV_IS_LIST_PROPERTY(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), GDAV_TYPE_LIST_PROPERTY))
#define GDAV_IS_LIST_PROPERTY_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_TYPE \
	((cls), GDAV_TYPE_LIST_PROPERTY))
#define GDAV_LIST_PROPERTY_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS \
	((obj), GDAV_TYPE_LIST_PROPERTY, GDavListPropertyClass))

G_BEGIN_DECLS

typedef struct _GDavListProperty GDavListProperty;
typedef struct _GDavListPropertyClass GDavListPropertyClass;
typedef struct _GDavListPropertyPrivate GDavListPropertyPrivate;

struct _GDavListProperty {
	GDavProperty parent;
	GDavListPropertyPrivate *priv;
};

struct _GDavListPropertyClass {
	GDavPropertyClass parent_class;

	GType element_type;
};

GType		gdav_list_property_get_type	(void) G_GNUC_CONST;
guint		gdav_list_property_length	(GDavListProperty *property);
void		gdav_list_property_add_value	(GDavListProperty *property,
						 const GValue *value);
gboolean	gdav_list_property_get_value	(GDavListProperty *property,
						 guint index,
						 GValue *value);

G_END_DECLS

#endif /* __GDAV_LIST_PROPRETY_H__ */


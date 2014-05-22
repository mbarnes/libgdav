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

#ifndef __GDAV_PROPERTY_SET_H__
#define __GDAV_PROPERTY_SET_H__

#include <gio/gio.h>
#include <libgdav/gdav-parsable.h>
#include <libgdav/gdav-property.h>

/* Standard GObject macros */
#define GDAV_TYPE_PROPERTY_SET \
	(gdav_property_set_get_type ())
#define GDAV_PROPERTY_SET(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), GDAV_TYPE_PROPERTY_SET, GDavPropertySet))
#define GDAV_PROPERTY_SET_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_CAST \
	((cls), GDAV_TYPE_PROPERTY_SET, GDavPropertySetClass))
#define GDAV_IS_PROPERTY_SET(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), GDAV_TYPE_PROPERTY_SET))
#define GDAV_IS_PROPERTY_SET_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_TYPE \
	((cls), GDAV_TYPE_PROPERTY_SET))
#define GDAV_PROPERTY_SET_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS \
	((obj), GDAV_TYPE_PROPERTY_SET, GDavPropertySetClass))

G_BEGIN_DECLS

typedef struct _GDavPropertySet GDavPropertySet;
typedef struct _GDavPropertySetClass GDavPropertySetClass;
typedef struct _GDavPropertySetPrivate GDavPropertySetPrivate;

struct _GDavPropertySet {
	GDavParsable parent;
	GDavPropertySetPrivate *priv;
};

struct _GDavPropertySetClass {
	GDavParsableClass parent_class;
};

GType		gdav_property_set_get_type	(void) G_GNUC_CONST;
GDavPropertySet *
		gdav_property_set_new		(void);
void		gdav_property_set_add_type	(GDavPropertySet *propset,
						 GType property_type);
gboolean	gdav_property_set_has_type	(GDavPropertySet *propset,
						 GType property_type);
void		gdav_property_set_add		(GDavPropertySet *propset,
						 GDavProperty *property);
GList *		gdav_property_set_list		(GDavPropertySet *propset,
						 GType property_type);
GList *		gdav_property_set_list_all	(GDavPropertySet *propset);
gboolean	gdav_property_set_get_names_only
						(GDavPropertySet *propset);
void		gdav_property_set_set_names_only
						(GDavPropertySet *propset,
						 gboolean names_only);

G_END_DECLS

#endif /* __GDAV_PROPERTY_SET_H__ */


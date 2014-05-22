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

#ifndef __GDAV_PROPERTY_UPDATE_H__
#define __GDAV_PROPERTY_UPDATE_H__

#include <gio/gio.h>
#include <libgdav/gdav-parsable.h>
#include <libgdav/gdav-property.h>

/* Standard GObject macros */
#define GDAV_TYPE_PROPERTY_UPDATE \
	(gdav_property_update_get_type ())
#define GDAV_PROPERTY_UPDATE(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), GDAV_TYPE_PROPERTY_UPDATE, GDavPropertyUpdate))
#define GDAV_PROPERTY_UPDATE_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_CAST \
	((cls), GDAV_TYPE_PROPERTY_UPDATE, GDavPropertyUpdateClass))
#define GDAV_IS_PROPERTY_UPDATE(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), GDAV_TYPE_PROPERTY_UPDATE))
#define GDAV_IS_PROPERTY_UPDATE_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_TYPE \
	((cls), GDAV_TYPE_PROPERTY_UPDATE))
#define GDAV_PROPERTY_UPDATE_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS \
	((obj), GDAV_TYPE_PROPERTY_UPDATE, GDavPropertyUpdateClass))

G_BEGIN_DECLS

typedef struct _GDavPropertyUpdate GDavPropertyUpdate;
typedef struct _GDavPropertyUpdateClass GDavPropertyUpdateClass;
typedef struct _GDavPropertyUpdatePrivate GDavPropertyUpdatePrivate;

struct _GDavPropertyUpdate {
	GDavParsable parent;
	GDavPropertyUpdatePrivate *priv;
};

struct _GDavPropertyUpdateClass {
	GDavParsableClass parent_class;
};

GType		gdav_property_update_get_type	(void) G_GNUC_CONST;
GDavPropertyUpdate *
		gdav_property_update_new	(void);
void		gdav_property_update_set	(GDavPropertyUpdate *update,
						 GDavProperty *property);
void		gdav_property_update_remove	(GDavPropertyUpdate *update,
						 GType property_type);

#endif /* __GDAV_PROPERTY_UPDATE_H__ */


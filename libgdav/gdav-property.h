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

#ifndef __GDAV_PROPERTY_H__
#define __GDAV_PROPERTY_H__

#include <libgdav/gdav-parsable.h>

/* Standard GObject macros */
#define GDAV_TYPE_PROPERTY \
	(gdav_property_get_type ())
#define GDAV_PROPERTY(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), GDAV_TYPE_PROPERTY, GDavProperty))
#define GDAV_PROPERTY_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_CAST \
	((cls), GDAV_TYPE_PROPERTY, GDavPropertyClass))
#define GDAV_IS_PROPERTY(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), GDAV_TYPE_PROPERTY))
#define GDAV_IS_PROPERTY_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_TYPE \
	((cls), GDAV_TYPE_PROPERTY))
#define GDAV_PROPERTY_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS \
	((obj), GDAV_TYPE_PROPERTY, GDavPropertyClass))

G_BEGIN_DECLS

typedef struct _GDavProperty GDavProperty;
typedef struct _GDavPropertyClass GDavPropertyClass;
typedef struct _GDavPropertyPrivate GDavPropertyPrivate;

struct _GDavProperty {
	GDavParsable parent;
	GDavPropertyPrivate *priv;
};

struct _GDavPropertyClass {
	GDavParsableClass parent_class;

	GType value_type;

	gboolean	(*parse_data)		(GDavProperty *property,
						 const gchar *data,
						 GValue *result,
						 GError **error);
};

GType		gdav_property_get_type		(void) G_GNUC_CONST;
void		gdav_property_get_value		(GDavProperty *property,
						 GValue *value);
gboolean	gdav_property_set_value		(GDavProperty *property,
						 const GValue *value);

#endif /* __GDAV_PROPERTY_H__ */


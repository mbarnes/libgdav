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

#ifndef __GDAV_PCDATA_PROPERTY_H__
#define __GDAV_PCDATA_PROPERTY_H__

#include <libgdav/gdav-property.h>

/* Standard GObject macros */
#define GDAV_TYPE_PCDATA_PROPERTY \
	(gdav_pcdata_property_get_type ())
#define GDAV_PCDATA_PROPERTY(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), GDAV_TYPE_PCDATA_PROPERTY, GDavPCDataProperty))
#define GDAV_PCDATA_PROPERTY_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_CAST \
	((cls), GDAV_TYPE_PCDATA_PROPERTY, GDavPCDataPropertyClass))
#define GDAV_IS_PCDATA_PROPERTY(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), GDAV_TYPE_PCDATA_PROPERTY))
#define GDAV_IS_PCDATA_PROPERTY_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_TYPE \
	((cls), GDAV_TYPE_PCDATA_PROPERTY))
#define GDAV_PCDATA_PROPERTY_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS \
	((obj), GDAV_TYPE_PCDATA_PROPERTY, GDavPCDataPropertyClass))

G_BEGIN_DECLS

typedef struct _GDavPCDataProperty GDavPCDataProperty;
typedef struct _GDavPCDataPropertyClass GDavPCDataPropertyClass;
typedef struct _GDavPCDataPropertyPrivate GDavPCDataPropertyPrivate;

struct _GDavPCDataProperty {
	GDavProperty parent;
	GDavPCDataPropertyPrivate *priv;
};

struct _GDavPCDataPropertyClass {
	GDavPropertyClass parent_class;

	/* Whether parsing should fail on empty character data. */
	gboolean allow_empty_data;

	gboolean	(*parse_data)		(GDavPCDataProperty *property,
						 const gchar *data,
						 GValue *result);
	gchar *		(*write_data)		(GDavPCDataProperty *property);
};

GType		gdav_pcdata_property_get_type	(void) G_GNUC_CONST;
GDavProperty *	gdav_pcdata_property_new_from_data
						(GType pcdata_property_type,
						 const gchar *data);
gboolean	gdav_pcdata_property_parse_data	(GDavPCDataProperty *property,
						 const gchar *data,
						 GValue *result);
gchar *		gdav_pcdata_property_write_data	(GDavPCDataProperty *property);

G_END_DECLS

#endif /* __GDAV_PCDATA_PROPERTY_H__ */


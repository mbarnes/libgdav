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

#ifndef __GDAV_PROP_STAT_H__
#define __GDAV_PROP_STAT_H__

#include <libgdav/gdav-error.h>
#include <libgdav/gdav-parsable.h>
#include <libgdav/gdav-property-set.h>

/* Standard GObject macros */
#define GDAV_TYPE_PROP_STAT \
	(gdav_prop_stat_get_type ())
#define GDAV_PROP_STAT(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), GDAV_TYPE_PROP_STAT, GDavPropStat))
#define GDAV_PROP_STAT_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_CAST \
	((cls), GDAV_TYPE_PROP_STAT, GDavPropStatClass))
#define GDAV_IS_PROP_STAT(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), GDAV_TYPE_PROP_STAT))
#define GDAV_IS_PROP_STAT_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_TYPE \
	((cls), GDAV_TYPE_PROP_STAT))
#define GDAV_PROP_STAT_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS \
	((obj), GDAV_TYPE_PROP_STAT, GDavPropStatClass))

G_BEGIN_DECLS

typedef struct _GDavPropStat GDavPropStat;
typedef struct _GDavPropStatClass GDavPropStatClass;
typedef struct _GDavPropStatPrivate GDavPropStatPrivate;

struct _GDavPropStat {
	GDavParsable parent;
	GDavPropStatPrivate *priv;
};

struct _GDavPropStatClass {
	GDavParsableClass parent_class;
};

GType		gdav_prop_stat_get_type		(void) G_GNUC_CONST;
GDavPropertySet *
		gdav_prop_stat_get_prop		(GDavPropStat *prop_stat);
guint		gdav_prop_stat_get_status	(GDavPropStat *prop_stat,
						 gchar **reason_phrase);
GDavError *	gdav_prop_stat_get_error	(GDavPropStat *prop_stat);
const gchar *	gdav_prop_stat_get_description	(GDavPropStat *prop_stat);

G_END_DECLS

#endif /* __GDAV_PROP_STAT_H__ */


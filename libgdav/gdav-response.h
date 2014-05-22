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

#ifndef __GDAV_RESPONSE_H__
#define __GDAV_RESPONSE_H__

#include <libgdav/gdav-error.h>
#include <libgdav/gdav-parsable.h>
#include <libgdav/gdav-prop-stat.h>

/* Standard GObject macros */
#define GDAV_TYPE_RESPONSE \
	(gdav_response_get_type ())
#define GDAV_RESPONSE(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), GDAV_TYPE_RESPONSE, GDavResponse))
#define GDAV_RESPONSE_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_CAST \
	((cls), GDAV_TYPE_RESPONSE, GDavResponseClass))
#define GDAV_IS_RESPONSE(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), GDAV_TYPE_RESPONSE))
#define GDAV_IS_RESPONSE_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_TYPE \
	((cls), GDAV_TYPE_RESPONSE))
#define GDAV_RESPONSE_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS \
	((obj), GDAV_TYPE_RESPONSE, GDavResponseClass))

G_BEGIN_DECLS

typedef struct _GDavResponse GDavResponse;
typedef struct _GDavResponseClass GDavResponseClass;
typedef struct _GDavResponsePrivate GDavResponsePrivate;

struct _GDavResponse {
	GDavParsable parent;
	GDavResponsePrivate *priv;
};

struct _GDavResponseClass {
	GDavParsableClass parent_class;
};

GType		gdav_response_get_type		(void) G_GNUC_CONST;
gboolean	gdav_response_has_href		(GDavResponse *response,
						 SoupURI *uri);
guint		gdav_response_get_status	(GDavResponse *response,
						 gchar **reason_phrase);
GDavPropStat *	gdav_response_get_propstat	(GDavResponse *response,
						 guint index);
guint		gdav_response_get_n_propstats	(GDavResponse *response);
GDavError *	gdav_response_get_error		(GDavResponse *response);
const gchar *	gdav_response_get_description	(GDavResponse *response);
const gchar *	gdav_response_get_location	(GDavResponse *response);

guint		gdav_response_find_property	(GDavResponse *response,
						 GType property_type,
						 GValue *value,
						 gchar **reason_phrase);

G_END_DECLS

#endif /* __GDAV_RESPONSE_H__ */


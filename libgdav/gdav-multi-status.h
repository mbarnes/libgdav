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

#ifndef __GDAV_MULTI_STATUS_H__
#define __GDAV_MULTI_STATUS_H__

#include <libgdav/gdav-parsable.h>
#include <libgdav/gdav-response.h>

/* Standard GObject macros */
#define GDAV_TYPE_MULTI_STATUS \
	(gdav_multi_status_get_type ())
#define GDAV_MULTI_STATUS(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), GDAV_TYPE_MULTI_STATUS, GDavMultiStatus))
#define GDAV_MULTI_STATUS_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_CAST \
	((cls), GDAV_TYPE_MULTI_STATUS, GDavMultiStatusClass))
#define GDAV_IS_MULTI_STATUS(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), GDAV_TYPE_MULTI_STATUS))
#define GDAV_IS_MULTI_STATUS_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_TYPE \
	((cls), GDAV_TYPE_MULTI_STATUS))
#define GDAV_MULTI_STATUS_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS \
	((obj), GDAV_TYPE_MULTI_STATUS, GDavMultiStatusClass))

G_BEGIN_DECLS

typedef struct _GDavMultiStatus GDavMultiStatus;
typedef struct _GDavMultiStatusClass GDavMultiStatusClass;
typedef struct _GDavMultiStatusPrivate GDavMultiStatusPrivate;

struct _GDavMultiStatus {
	GDavParsable parent;
	GDavMultiStatusPrivate *priv;
};

struct _GDavMultiStatusClass {
	GDavParsableClass parent_class;
};

GType		gdav_multi_status_get_type
					(void) G_GNUC_CONST;
GDavMultiStatus *
		gdav_multi_status_new_from_message
					(SoupMessage *message,
					 GError **error);
gboolean	gdav_multi_status_has_errors
					(GDavMultiStatus *multi_status);
GDavResponse *	gdav_multi_status_get_response
					(GDavMultiStatus *multi_status,
					 guint index);
GDavResponse *	gdav_multi_status_get_response_by_href
					(GDavMultiStatus *multi_status,
					 SoupURI *uri);
guint		gdav_multi_status_get_n_responses
					(GDavMultiStatus *multi_status);
const gchar *	gdav_multi_status_get_description
					(GDavMultiStatus *multi_status);

G_END_DECLS

#endif /* __GDAV_MULTI_STATUS_H__ */


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

#ifndef __GDAV_METHODS_H__
#define __GDAV_METHODS_H__

#include <libgdav/gdav-multi-status.h>
#include <libgdav/gdav-requests.h>

G_BEGIN_DECLS

gboolean	gdav_options_sync		(SoupSession *session,
						 SoupURI *uri,
						 GDavAllow *out_allow,
						 GDavOptions *out_options,
						 SoupMessage **out_message,
						 GCancellable *cancellable,
						 GError **error);
void		gdav_options			(SoupSession *session,
						 SoupURI *uri,
						 GCancellable *cancellable,
						 GAsyncReadyCallback callback,
						 gpointer user_data);
gboolean	gdav_options_finish		(SoupSession *session,
						 GAsyncResult *result,
						 GDavAllow *out_allow,
						 GDavOptions *out_options,
						 SoupMessage **out_message,
						 GError **error);

GDavMultiStatus *
		gdav_propfind_sync		(SoupSession *session,
						 SoupURI *uri,
						 GDavPropFindType type,
						 GDavPropertySet *prop,
						 GDavDepth depth,
						 SoupMessage **out_message,
						 GCancellable *cancellable,
						 GError **error);
void		gdav_propfind			(SoupSession *session,
						 SoupURI *uri,
						 GDavPropFindType type,
						 GDavPropertySet *prop,
						 GDavDepth depth,
						 GCancellable *cancellable,
						 GAsyncReadyCallback callback,
						 gpointer user_data);
GDavMultiStatus *
		gdav_propfind_finish		(SoupSession *session,
						 GAsyncResult *result,
						 SoupMessage **out_message,
						 GError **error);

G_END_DECLS

#endif /* __GDAV_METHODS_H__ */


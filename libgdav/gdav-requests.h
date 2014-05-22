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

#ifndef __GDAV_REQUESTS_H__
#define __GDAV_REQUESTS_H__

#include <libgdav/gdav-enums.h>
#include <libgdav/gdav-property-set.h>
#include <libgdav/gdav-property-update.h>

G_BEGIN_DECLS

SoupRequestHTTP *
		gdav_request_options		(SoupSession *session,
						 const gchar *uri_string,
						 GError **error);
SoupRequestHTTP *
		gdav_request_options_uri	(SoupSession *session,
						 SoupURI *uri,
						 GError **error);

SoupRequestHTTP *
		gdav_request_propfind		(SoupSession *session,
						 const gchar *uri_string,
						 GDavPropFindType type,
						 GDavPropertySet *prop,
						 GDavDepth depth,
						 GError **error);
SoupRequestHTTP *
		gdav_request_propfind_uri	(SoupSession *session,
						 SoupURI *uri,
						 GDavPropFindType type,
						 GDavPropertySet *prop,
						 GDavDepth depth,
						 GError **error);

SoupRequestHTTP *
		gdav_request_proppatch		(SoupSession *session,
						 const gchar *uri_string,
						 GDavPropertyUpdate *update,
						 GError **error);
SoupRequestHTTP *
		gdav_request_proppatch_uri	(SoupSession *session,
						 SoupURI *uri,
						 GDavPropertyUpdate *update,
						 GError **error);

SoupRequestHTTP *
		gdav_request_mkcol		(SoupSession *session,
						 const gchar *uri_string,
						 GError **error);
SoupRequestHTTP *
		gdav_request_mkcol_uri		(SoupSession *session,
						 SoupURI *uri,
						 GError **error);

SoupRequestHTTP *
		gdav_request_delete		(SoupSession *session,
						 const gchar *uri_string,
						 GError **error);
SoupRequestHTTP *
		gdav_request_delete_uri		(SoupSession *session,
						 SoupURI *uri,
						 GError **error);

SoupRequestHTTP *
		gdav_request_copy		(SoupSession *session,
						 const gchar *uri_string,
						 const gchar *destination,
						 GDavCopyFlags flags,
						 GError **error);
SoupRequestHTTP *
		gdav_request_copy_uri		(SoupSession *session,
						 SoupURI *uri,
						 const gchar *destination,
						 GDavCopyFlags flags,
						 GError **error);

SoupRequestHTTP *
		gdav_request_move		(SoupSession *session,
						 const gchar *uri_string,
						 const gchar *destination,
						 GDavMoveFlags flags,
						 GError **error);
SoupRequestHTTP *
		gdav_request_move_uri		(SoupSession *session,
						 SoupURI *uri,
						 const gchar *destination,
						 GDavMoveFlags flags,
						 GError **error);

SoupRequestHTTP *
		gdav_request_lock		(SoupSession *session,
						 const gchar *uri_string,
						 GDavLockScope lock_scope,
						 GDavLockType lock_type,
						 GDavLockFlags flags,
						 const gchar *owner,
						 gint timeout,
						 GError **error);
SoupRequestHTTP *
		gdav_request_lock_uri		(SoupSession *session,
						 SoupURI *uri,
						 GDavLockScope lock_scope,
						 GDavLockType lock_type,
						 GDavLockFlags flags,
						 const gchar *owner,
						 gint timeout,
						 GError **error);

SoupRequestHTTP *
		gdav_request_lock_refresh	(SoupSession *session,
						 const gchar *uri_string,
						 const gchar *lock_token,
						 gint timeout,
						 GError **error);
SoupRequestHTTP *
		gdav_request_lock_refresh_uri	(SoupSession *session,
						 SoupURI *uri,
						 const gchar *lock_token,
						 gint timeout,
						 GError **error);

SoupRequestHTTP *
		gdav_request_unlock		(SoupSession *session,
						 const gchar *uri_string,
						 const gchar *lock_token,
						 GError **error);
SoupRequestHTTP *
		gdav_request_unlock_uri		(SoupSession *session,
						 SoupURI *uri,
						 const gchar *lock_token,
						 GError **error);

void		gdav_request_add_lock_token	(SoupRequestHTTP *request,
						 const gchar *resource_tag,
						 const gchar *lock_token);

G_END_DECLS

#endif /* __GDAV_REQUESTS_H__ */


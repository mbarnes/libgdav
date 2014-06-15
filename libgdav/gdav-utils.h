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

#ifndef __GDAV_UTILS_H__
#define __GDAV_UTILS_H__

#include <libxml/tree.h>
#include <libsoup/soup.h>
#include <libgdav/gdav-enums.h>

#define GDAV_STATUS_IS_ERROR(status) \
	(SOUP_STATUS_IS_TRANSPORT_ERROR (status) || \
	 SOUP_STATUS_IS_CLIENT_ERROR (status) || \
	 SOUP_STATUS_IS_SERVER_ERROR (status))

G_BEGIN_DECLS

GDavAllow	gdav_allow_from_headers		(SoupMessageHeaders *headers);
GDavOptions	gdav_options_from_headers	(SoupMessageHeaders *headers);

gboolean	gdav_error_missing_content	(xmlNode *element,
						 GError **error);
gboolean	gdav_error_unknown_content	(xmlNode *element,
						 const gchar *actual_content,
						 GError **error);

G_END_DECLS

#endif /* __GDAV_UTILS_H__ */


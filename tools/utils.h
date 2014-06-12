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

#ifndef __UTILS_H__
#define __UTILS_H__

#include <libgdav/gdav.h>

#include "structs.h"

void		open_connection			(GlobalState *state,
						 const gchar *uri_string);
void		close_connection		(GlobalState *state);
gboolean	set_path			(GlobalState *state,
						 SoupURI *uri);

gboolean	accept_bad_certificate		(SoupMessage *message);
void		print_error			(GError *error);

GDavResourceType
		get_resource_type		(SoupSession *session,
						 SoupURI *uri,
						 GError **error);

gboolean	get_resource_list		(SoupSession *session,
						 SoupURI *uri,
						 GDavDepth depth,
						 GQueue *out_resources,
						 GError **error);
void		free_resource			(Resource *resource);

#endif /* __UTILS_H__ */


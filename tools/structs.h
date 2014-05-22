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

#ifndef __STRUCTS_H__
#define __STRUCTS_H__

#include <histedit.h>

#include <libgdav/gdav.h>
#include <libsoup/soup.h>

#include "enums.h"

typedef struct {
	SoupURI *base_uri;
	SoupSession *session;
	gboolean connected;
	gboolean isdav;
	GDavAllow allow;
	GDavOptions options;
	gboolean prompt_username;
} GlobalState;

typedef void	(*CommandHandler)		(GlobalState *state,
						 gint argc,
						 const gchar **argv);

typedef struct {
	GDavCommand id;
	CommandHandler handler;
	gboolean needs_connection;
	gint min_args, max_args;
	const gchar *usage;
	const gchar *blurb;
} Command;

#endif /* __STRUCTS_H__ */


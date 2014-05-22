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

#include "config.h"

#include "gdav-error.h"

#define GDAV_ERROR_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE \
	((obj), GDAV_TYPE_ERROR, GDavErrorPrivate))

struct _GDavErrorPrivate {
	gint placeholder;
};

G_DEFINE_TYPE (GDavError, gdav_error, GDAV_TYPE_PARSABLE)

static void
gdav_error_class_init (GDavErrorClass *class)
{
	GDavParsableClass *parsable_class;

	g_type_class_add_private (class, sizeof (GDavErrorPrivate));

	parsable_class = GDAV_PARSABLE_CLASS (class);
	parsable_class->element_name = "error";
	parsable_class->element_namespace = GDAV_XMLNS_DAV;
}

static void
gdav_error_init (GDavError *error)
{
	error->priv = GDAV_ERROR_GET_PRIVATE (error);
}


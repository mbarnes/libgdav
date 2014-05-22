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

#ifndef __GDAV_ERROR_H__
#define __GDAV_ERROR_H__

#include <libgdav/gdav-parsable.h>

/* Standard GObject macros */
#define GDAV_TYPE_ERROR \
	(gdav_error_get_type ())
#define GDAV_ERROR(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), GDAV_TYPE_ERROR, GDavError))
#define GDAV_ERROR_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_CAST \
	((cls), GDAV_TYPE_ERROR, GDavErrorClass))
#define GDAV_IS_ERROR(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), GDAV_TYPE_ERROR))
#define GDAV_IS_ERROR_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_TYPE \
	((cls), GDAV_TYPE_ERROR))
#define GDAV_ERROR_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS \
	((obj), GDAV_TYPE_ERROR, GDavErrorClass))

G_BEGIN_DECLS

typedef struct _GDavError GDavError;
typedef struct _GDavErrorClass GDavErrorClass;
typedef struct _GDavErrorPrivate GDavErrorPrivate;

struct _GDavError {
	GDavParsable parent;
	GDavErrorPrivate *priv;
};

struct _GDavErrorClass {
	GDavParsableClass parent_class;
};

GType		gdav_error_get_type		(void) G_GNUC_CONST;

#endif /* __GDAV_ERROR_H__ */


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

#ifndef __GDAV_ACTIVE_LOCK_H__
#define __GDAV_ACTIVE_LOCK_H__

#include <libgdav/gdav-enums.h>
#include <libgdav/gdav-parsable.h>

/* Standard GObject macros */
#define GDAV_TYPE_ACTIVE_LOCK \
	(gdav_active_lock_get_type ())
#define GDAV_ACTIVE_LOCK(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), GDAV_TYPE_ACTIVE_LOCK, GDavActiveLock))
#define GDAV_ACTIVE_LOCK_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_CAST \
	((cls), GDAV_TYPE_ACTIVE_LOCK, GDavActiveLockClass))
#define GDAV_IS_ACTIVE_LOCK(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), GDAV_TYPE_ACTIVE_LOCK))
#define GDAV_IS_ACTIVE_LOCK_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_TYPE \
	((cls), GDAV_TYPE_ACTIVE_LOCK))
#define GDAV_ACTIVE_LOCK_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS \
	((obj), GDAV_TYPE_ACTIVE_LOCK, GDavActiveLockClass))

G_BEGIN_DECLS

typedef struct _GDavActiveLock GDavActiveLock;
typedef struct _GDavActiveLockClass GDavActiveLockClass;
typedef struct _GDavActiveLockPrivate GDavActiveLockPrivate;

struct _GDavActiveLock {
	GDavParsable parent;
	GDavActiveLockPrivate *priv;
};

struct _GDavActiveLockClass {
	GDavParsableClass parent_class;
};

GType		gdav_active_lock_get_type	(void) G_GNUC_CONST;
GDavActiveLock *
		gdav_active_lock_new		(GDavLockScope lock_scope,
						 GDavLockType lock_type,
						 GDavDepth depth,
						 const gchar *lock_root);
GDavLockScope	gdav_active_lock_get_lock_scope	(GDavActiveLock *active_lock);
void		gdav_active_lock_set_lock_scope	(GDavActiveLock *active_lock,
						 GDavLockScope lock_scope);
GDavLockType	gdav_active_lock_get_lock_type	(GDavActiveLock *active_lock);
void		gdav_active_lock_set_lock_type	(GDavActiveLock *active_lock,
						 GDavLockType lock_type);
GDavDepth	gdav_active_lock_get_depth	(GDavActiveLock *active_lock);
void		gdav_active_lock_set_depth	(GDavActiveLock *active_lock,
						 GDavDepth depth);
const gchar *	gdav_active_lock_get_owner	(GDavActiveLock *active_lock);
void		gdav_active_lock_set_owner	(GDavActiveLock *active_lock,
						 const gchar *owner);
gint		gdav_active_lock_get_timeout	(GDavActiveLock *active_lock);
void		gdav_active_lock_set_timeout	(GDavActiveLock *active_lock,
						 gint timeout);
const gchar *	gdav_active_lock_get_lock_token	(GDavActiveLock *active_lock);
void		gdav_active_lock_set_lock_token	(GDavActiveLock *active_lock,
						 const gchar *lock_token);
const gchar *	gdav_active_lock_get_lock_root	(GDavActiveLock *active_lock);
void		gdav_active_lock_set_lock_root	(GDavActiveLock *active_lock,
						 const gchar *lock_root);

G_END_DECLS

#endif /* __GDAV_ACTIVE_LOCK_H__ */


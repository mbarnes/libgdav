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

#ifndef __GDAV_LOCK_ENTRY_H__
#define __GDAV_LOCK_ENTRY_H__

#include <libgdav/gdav-enums.h>
#include <libgdav/gdav-parsable.h>

/* Standard GObject macros */
#define GDAV_TYPE_LOCK_ENTRY \
	(gdav_lock_entry_get_type ())
#define GDAV_LOCK_ENTRY(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), GDAV_TYPE_LOCK_ENTRY, GDavLockEntry))
#define GDAV_LOCK_ENTRY_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_CAST \
	((cls), GDAV_TYPE_LOCK_ENTRY, GDavLockEntryClass))
#define GDAV_IS_LOCK_ENTRY(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), GDAV_TYPE_LOCK_ENTRY))
#define GDAV_IS_LOCK_ENTRY_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_TYPE \
	((cls), GDAV_TYPE_LOCK_ENTRY))
#define GDAV_LOCK_ENTRY_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS \
	((obj), GDAV_TYPE_LOCK_ENTRY, GDavLockEntryClass))

G_BEGIN_DECLS

typedef struct _GDavLockEntry GDavLockEntry;
typedef struct _GDavLockEntryClass GDavLockEntryClass;
typedef struct _GDavLockEntryPrivate GDavLockEntryPrivate;

struct _GDavLockEntry {
	GDavParsable parent;
	GDavLockEntryPrivate *priv;
};

struct _GDavLockEntryClass {
	GDavParsableClass parent_class;
};

GType		gdav_lock_entry_get_type	(void) G_GNUC_CONST;
GDavLockEntry *	gdav_lock_entry_new		(GDavLockScope lock_scope,
						 GDavLockType lock_type);
GDavLockScope	gdav_lock_entry_get_lock_scope	(GDavLockEntry *lock_entry);
GDavLockType	gdav_lock_entry_get_lock_type	(GDavLockEntry *lock_entry);

G_END_DECLS

#endif /* __GDAV_LOCK_ENTRY_H__ */


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

#include "gdav-lock-entry.h"

#include "gdav-enumtypes.h"

#define GDAV_LOCK_ENTRY_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE \
	((obj), GDAV_TYPE_LOCK_ENTRY, GDavLockEntryPrivate))

struct _GDavLockEntryPrivate {
	GDavLockScope lock_scope;
	GDavLockType lock_type;
};

enum {
	PROP_0,
	PROP_LOCK_SCOPE,
	PROP_LOCK_TYPE
};

G_DEFINE_TYPE (GDavLockEntry, gdav_lock_entry, GDAV_TYPE_PARSABLE)

static void
gdav_lock_entry_set_lock_scope (GDavLockEntry *lock_entry,
                                GDavLockScope lock_scope)
{
	lock_entry->priv->lock_scope = lock_scope;
}

static void
gdav_lock_entry_set_lock_type (GDavLockEntry *lock_entry,
                               GDavLockType lock_type)
{
	lock_entry->priv->lock_type = lock_type;
}

static void
gdav_lock_entry_set_property (GObject *object,
                              guint property_id,
                              const GValue *value,
                              GParamSpec *pspec)
{
	switch (property_id) {
		case PROP_LOCK_SCOPE:
			gdav_lock_entry_set_lock_scope (
				GDAV_LOCK_ENTRY (object),
				g_value_get_enum (value));
			return;

		case PROP_LOCK_TYPE:
			gdav_lock_entry_set_lock_type (
				GDAV_LOCK_ENTRY (object),
				g_value_get_enum (value));
			return;
	}

	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
gdav_lock_entry_get_property (GObject *object,
                              guint property_id,
                              GValue *value,
                              GParamSpec *pspec)
{
	switch (property_id) {
		case PROP_LOCK_SCOPE:
			g_value_set_enum (
				value,
				gdav_lock_entry_get_lock_scope (
				GDAV_LOCK_ENTRY (object)));
			return;

		case PROP_LOCK_TYPE:
			g_value_set_enum (
				value,
				gdav_lock_entry_get_lock_type (
				GDAV_LOCK_ENTRY (object)));
			return;
	}

	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
gdav_lock_entry_class_init (GDavLockEntryClass *class)
{
	GObjectClass *object_class;
	GDavParsableClass *parsable_class;

	g_type_class_add_private (class, sizeof (GDavLockEntryPrivate));

	object_class = G_OBJECT_CLASS (class);
	object_class->set_property = gdav_lock_entry_set_property;
	object_class->get_property = gdav_lock_entry_get_property;

	parsable_class = GDAV_PARSABLE_CLASS (class);
	parsable_class->element_name = "lockentry";
	parsable_class->element_namespace = GDAV_XMLNS_DAV;

	g_object_class_install_property (
		object_class,
		PROP_LOCK_SCOPE,
		g_param_spec_enum (
			"lock-scope",
			"Lock Scope",
			"Exclusive or shared lock",
			GDAV_TYPE_LOCK_SCOPE,
			GDAV_LOCK_SCOPE_EXCLUSIVE,
			G_PARAM_READWRITE |
			G_PARAM_CONSTRUCT_ONLY |
			G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (
		object_class,
		PROP_LOCK_TYPE,
		g_param_spec_enum (
			"lock-type",
			"Lock Type",
			"Access type of the lock",
			GDAV_TYPE_LOCK_TYPE,
			GDAV_LOCK_TYPE_WRITE,
			G_PARAM_READWRITE |
			G_PARAM_CONSTRUCT_ONLY |
			G_PARAM_STATIC_STRINGS));
}

static void
gdav_lock_entry_init (GDavLockEntry *lock_entry)
{
	lock_entry->priv = GDAV_LOCK_ENTRY_GET_PRIVATE (lock_entry);
}

GDavLockEntry *
gdav_lock_entry_new (GDavLockScope lock_scope,
                     GDavLockType lock_type)
{
	return g_object_new (
		GDAV_TYPE_LOCK_ENTRY,
		"lock-scope", lock_scope,
		"lock-type", lock_type,
		NULL);
}

GDavLockScope
gdav_lock_entry_get_lock_scope (GDavLockEntry *lock_entry)
{
	g_return_val_if_fail (GDAV_IS_LOCK_ENTRY (lock_entry), 0);

	return lock_entry->priv->lock_scope;
}

GDavLockType
gdav_lock_entry_get_lock_type (GDavLockEntry *lock_entry)
{
	g_return_val_if_fail (GDAV_IS_LOCK_ENTRY (lock_entry), 0);

	return lock_entry->priv->lock_type;
}


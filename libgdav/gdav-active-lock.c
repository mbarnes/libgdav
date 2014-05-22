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

#include "gdav-active-lock.h"

#include "gdav-enumtypes.h"

#define GDAV_ACTIVE_LOCK_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE \
	((obj), GDAV_TYPE_ACTIVE_LOCK, GDavActiveLockPrivate))

struct _GDavActiveLockPrivate {
	GDavLockScope lock_scope;
	GDavLockType lock_type;
	GDavDepth depth;
	gchar *owner;
	gint timeout;  /* negative means infinite */
	gchar *lock_token;
	gchar *lock_root;
};

enum {
	PROP_0,
	PROP_DEPTH,
	PROP_LOCK_ROOT,
	PROP_LOCK_SCOPE,
	PROP_LOCK_TOKEN,
	PROP_LOCK_TYPE,
	PROP_OWNER,
	PROP_TIMEOUT
};

G_DEFINE_TYPE (GDavActiveLock, gdav_active_lock, GDAV_TYPE_PARSABLE)

static void
gdav_active_lock_set_property (GObject *object,
                               guint property_id,
                               const GValue *value,
                               GParamSpec *pspec)
{
	switch (property_id) {
		case PROP_DEPTH:
			gdav_active_lock_set_depth (
				GDAV_ACTIVE_LOCK (object),
				g_value_get_enum (value));
			return;

		case PROP_LOCK_ROOT:
			gdav_active_lock_set_lock_root (
				GDAV_ACTIVE_LOCK (object),
				g_value_get_string (value));
			return;

		case PROP_LOCK_SCOPE:
			gdav_active_lock_set_lock_scope (
				GDAV_ACTIVE_LOCK (object),
				g_value_get_enum (value));
			return;

		case PROP_LOCK_TOKEN:
			gdav_active_lock_set_lock_token (
				GDAV_ACTIVE_LOCK (object),
				g_value_get_string (value));
			return;

		case PROP_LOCK_TYPE:
			gdav_active_lock_set_lock_type (
				GDAV_ACTIVE_LOCK (object),
				g_value_get_enum (value));
			return;

		case PROP_OWNER:
			gdav_active_lock_set_owner (
				GDAV_ACTIVE_LOCK (object),
				g_value_get_string (value));
			return;

		case PROP_TIMEOUT:
			gdav_active_lock_set_timeout (
				GDAV_ACTIVE_LOCK (object),
				g_value_get_int (value));
			return;
	}

	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
gdav_active_lock_get_property (GObject *object,
                               guint property_id,
                               GValue *value,
                               GParamSpec *pspec)
{
	switch (property_id) {
		case PROP_DEPTH:
			g_value_set_enum (
				value,
				gdav_active_lock_get_depth (
				GDAV_ACTIVE_LOCK (object)));
			return;

		case PROP_LOCK_ROOT:
			g_value_set_string (
				value,
				gdav_active_lock_get_lock_root (
				GDAV_ACTIVE_LOCK (object)));
			return;

		case PROP_LOCK_SCOPE:
			g_value_set_enum (
				value,
				gdav_active_lock_get_lock_scope (
				GDAV_ACTIVE_LOCK (object)));
			return;

		case PROP_LOCK_TOKEN:
			g_value_set_string (
				value,
				gdav_active_lock_get_lock_token (
				GDAV_ACTIVE_LOCK (object)));
			return;

		case PROP_LOCK_TYPE:
			g_value_set_enum (
				value,
				gdav_active_lock_get_lock_type (
				GDAV_ACTIVE_LOCK (object)));
			return;

		case PROP_OWNER:
			g_value_set_string (
				value,
				gdav_active_lock_get_owner (
				GDAV_ACTIVE_LOCK (object)));
			return;

		case PROP_TIMEOUT:
			g_value_set_int (
				value,
				gdav_active_lock_get_timeout (
				GDAV_ACTIVE_LOCK (object)));
			return;
	}

	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
gdav_active_lock_finalize (GObject *object)
{
	GDavActiveLockPrivate *priv;

	priv = GDAV_ACTIVE_LOCK_GET_PRIVATE (object);

	g_free (priv->owner);
	g_free (priv->lock_token);
	g_free (priv->lock_root);

	/* Chain up to parent's finalize() method. */
	G_OBJECT_CLASS (gdav_active_lock_parent_class)->finalize (object);
}

static void
gdav_active_lock_class_init (GDavActiveLockClass *class)
{
	GObjectClass *object_class;
	GDavParsableClass *parsable_class;

	g_type_class_add_private (class, sizeof (GDavActiveLockPrivate));

	object_class = G_OBJECT_CLASS (class);
	object_class->set_property = gdav_active_lock_set_property;
	object_class->get_property = gdav_active_lock_get_property;
	object_class->finalize = gdav_active_lock_finalize;

	parsable_class = GDAV_PARSABLE_CLASS (class);
	parsable_class->element_name = "activelock";
	parsable_class->element_namespace = GDAV_XMLNS_DAV;

	g_object_class_install_property (
		object_class,
		PROP_DEPTH,
		g_param_spec_enum (
			"depth",
			"Depth",
			"Depth of the lock",
			GDAV_TYPE_DEPTH,
			GDAV_DEPTH_0,
			G_PARAM_READWRITE |
			G_PARAM_CONSTRUCT |
			G_PARAM_STATIC_STRINGS));

	/* Don't use G_PARAM_CONSTRUCT here;
	 * the default value is unacceptable. */
	g_object_class_install_property (
		object_class,
		PROP_LOCK_ROOT,
		g_param_spec_string (
			"lock-root",
			"Lock Root",
			"Root URL of the lock",
			NULL,
			G_PARAM_READWRITE |
			G_PARAM_STATIC_STRINGS));

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
			G_PARAM_CONSTRUCT |
			G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (
		object_class,
		PROP_LOCK_TOKEN,
		g_param_spec_string (
			"lock-token",
			"Lock Token",
			"The token associated with the lock",
			NULL,
			G_PARAM_READWRITE |
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
			G_PARAM_CONSTRUCT |
			G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (
		object_class,
		PROP_OWNER,
		g_param_spec_string (
			"owner",
			"Owner",
			"Owner of the lock",
			NULL,
			G_PARAM_READWRITE |
			G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (
		object_class,
		PROP_TIMEOUT,
		g_param_spec_int (
			"timeout",
			"Timeout",
			"Lock timeout in seconds (-1 means infinite)",
			-1,
			G_MAXINT32,
			-1,
			G_PARAM_READWRITE |
			G_PARAM_CONSTRUCT |
			G_PARAM_STATIC_STRINGS));
}

static void
gdav_active_lock_init (GDavActiveLock *active_lock)
{
	active_lock->priv = GDAV_ACTIVE_LOCK_GET_PRIVATE (active_lock);
}

GDavActiveLock *
gdav_active_lock_new (GDavLockScope lock_scope,
                      GDavLockType lock_type,
                      GDavDepth depth,
                      const gchar *lock_root)
{
	g_return_val_if_fail (lock_root != NULL, NULL);

	return g_object_new (
		GDAV_TYPE_ACTIVE_LOCK,
		"lock-scope", lock_scope,
		"lock-type", lock_type,
		"depth", depth,
		"lock-root", lock_root,
		NULL);
}

GDavLockScope
gdav_active_lock_get_lock_scope (GDavActiveLock *active_lock)
{
	g_return_val_if_fail (GDAV_IS_ACTIVE_LOCK (active_lock), 0);

	return active_lock->priv->lock_scope;
}

void
gdav_active_lock_set_lock_scope (GDavActiveLock *active_lock,
                                 GDavLockScope lock_scope)
{
	g_return_if_fail (GDAV_IS_ACTIVE_LOCK (active_lock));

	if (lock_scope != active_lock->priv->lock_scope) {
		active_lock->priv->lock_scope = lock_scope;
		g_object_notify (G_OBJECT (active_lock), "lock-scope");
	}
}

GDavLockType
gdav_active_lock_get_lock_type (GDavActiveLock *active_lock)
{
	g_return_val_if_fail (GDAV_IS_ACTIVE_LOCK (active_lock), 0);

	return active_lock->priv->lock_type;
}

void
gdav_active_lock_set_lock_type (GDavActiveLock *active_lock,
                                GDavLockType lock_type)
{
	g_return_if_fail (GDAV_IS_ACTIVE_LOCK (active_lock));

	if (lock_type != active_lock->priv->lock_type) {
		active_lock->priv->lock_type = lock_type;
		g_object_notify (G_OBJECT (active_lock), "lock-type");
	}
}

GDavDepth
gdav_active_lock_get_depth (GDavActiveLock *active_lock)
{
	g_return_val_if_fail (GDAV_IS_ACTIVE_LOCK (active_lock), 0);

	return active_lock->priv->depth;
}

void
gdav_active_lock_set_depth (GDavActiveLock *active_lock,
                            GDavDepth depth)
{
	g_return_if_fail (GDAV_IS_ACTIVE_LOCK (active_lock));

	if (depth != active_lock->priv->depth) {
		active_lock->priv->depth = depth;
		g_object_notify (G_OBJECT (active_lock), "depth");
	}
}

const gchar *
gdav_active_lock_get_owner (GDavActiveLock *active_lock)
{
	g_return_val_if_fail (GDAV_IS_ACTIVE_LOCK (active_lock), NULL);

	return active_lock->priv->owner;
}

void
gdav_active_lock_set_owner (GDavActiveLock *active_lock,
                            const gchar *owner)
{
	g_return_if_fail (GDAV_IS_ACTIVE_LOCK (active_lock));
	/* owner is optional - argument may be NULL */

	if (g_strcmp0 (owner, active_lock->priv->owner) != 0) {
		g_free (active_lock->priv->owner);
		active_lock->priv->owner = g_strdup (owner);
		g_object_notify (G_OBJECT (active_lock), "owner");
	}
}

gint
gdav_active_lock_get_timeout (GDavActiveLock *active_lock)
{
	g_return_val_if_fail (GDAV_IS_ACTIVE_LOCK (active_lock), -1);

	return active_lock->priv->timeout;
}

void
gdav_active_lock_set_timeout (GDavActiveLock *active_lock,
                              gint timeout)
{
	g_return_if_fail (GDAV_IS_ACTIVE_LOCK (active_lock));

	/* Any negative timeout means infinite.  Clamp it to -1.
	 * A non-negative timeout must NOT be greater than 2^32-1. */
	timeout = CLAMP (timeout, -1, G_MAXINT32);

	if (timeout != active_lock->priv->timeout) {
		active_lock->priv->timeout = timeout;
		g_object_notify (G_OBJECT (active_lock), "timeout");
	}
}

const gchar *
gdav_active_lock_get_lock_token (GDavActiveLock *active_lock)
{
	g_return_val_if_fail (GDAV_IS_ACTIVE_LOCK (active_lock), NULL);

	return active_lock->priv->lock_token;
}

void
gdav_active_lock_set_lock_token (GDavActiveLock *active_lock,
                                 const gchar *lock_token)
{
	g_return_if_fail (GDAV_IS_ACTIVE_LOCK (active_lock));
	/* locktoken is optional - argument may be NULL */

	if (g_strcmp0 (lock_token, active_lock->priv->lock_token) != 0) {
		g_free (active_lock->priv->lock_token);
		active_lock->priv->lock_token = g_strdup (lock_token);
		g_object_notify (G_OBJECT (active_lock), "lock-token");
	}
}

const gchar *
gdav_active_lock_get_lock_root (GDavActiveLock *active_lock)
{
	g_return_val_if_fail (GDAV_IS_ACTIVE_LOCK (active_lock), NULL);

	return active_lock->priv->lock_root;
}

void
gdav_active_lock_set_lock_root (GDavActiveLock *active_lock,
                                const gchar *lock_root)
{
	g_return_if_fail (GDAV_IS_ACTIVE_LOCK (active_lock));
	/* lockroot is mandatory - argument may NOT be NULL */
	g_return_if_fail (lock_root != NULL);

	if (g_strcmp0 (lock_root, active_lock->priv->lock_root) != 0) {
		g_free (active_lock->priv->lock_root);
		active_lock->priv->lock_root = g_strdup (lock_root);
		g_object_notify (G_OBJECT (active_lock), "lock-root");
	}
}


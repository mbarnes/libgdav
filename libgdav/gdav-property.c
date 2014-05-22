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

#include "gdav-property.h"

#define GDAV_PROPERTY_GET_PRIVATE(obj) \
	(G_TYPE_INSTANCE_GET_PRIVATE \
	((obj), GDAV_TYPE_PROPERTY, GDavPropertyPrivate))

struct _GDavPropertyPrivate {
	GValue value;
};

enum {
	PROP_0,
	PROP_VALUE
};

G_DEFINE_ABSTRACT_TYPE (GDavProperty, gdav_property, GDAV_TYPE_PARSABLE)

static void
gdav_property_meta_set_value (GDavProperty *property,
                              const GValue *value)
{
	/* Copying a GValue containing the real property value,
	 * hence the "meta". */

	if (value != NULL)
		g_value_copy (value, &property->priv->value);
}

static GValue *
gdav_property_meta_get_value (GDavProperty *property)
{
	/* Copying a GValue containing the real property value,
	 * hence the "meta". */

	return g_boxed_copy (G_TYPE_VALUE, &property->priv->value);
}

static void
gdav_property_set_property (GObject *object,
                            guint property_id,
                            const GValue *value,
                            GParamSpec *pspec)
{
	switch (property_id) {
		case PROP_VALUE:
			gdav_property_meta_set_value (
				GDAV_PROPERTY (object),
				g_value_get_boxed (value));
			return;
	}

	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
gdav_property_get_property (GObject *object,
                            guint property_id,
                            GValue *value,
                            GParamSpec *pspec)
{
	switch (property_id) {
		case PROP_VALUE:
			g_value_set_boxed (
				value,
				gdav_property_meta_get_value (
				GDAV_PROPERTY (object)));
			return;
	}

	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
}

static void
gdav_property_dispose (GObject *object)
{
	GDavPropertyPrivate *priv;

	priv = GDAV_PROPERTY_GET_PRIVATE (object);

	g_value_reset (&priv->value);

	/* Chain up to parent's dispose() method. */
	G_OBJECT_CLASS (gdav_property_parent_class)->dispose (object);
}

static void
gdav_property_finalize (GObject *object)
{
	GDavPropertyPrivate *priv;

	priv = GDAV_PROPERTY_GET_PRIVATE (object);

	g_value_unset (&priv->value);

	/* Chain up to parent's finalize() method. */
	G_OBJECT_CLASS (gdav_property_parent_class)->finalize (object);
}

static void
gdav_property_constructed (GObject *object)
{
	GDavPropertyClass *class;
	GDavPropertyPrivate *priv;

	class = GDAV_PROPERTY_GET_CLASS (object);
	priv = GDAV_PROPERTY_GET_PRIVATE (object);

	g_value_init (&priv->value, class->value_type);

	/* Chain up to parent's constructed() method. */
	G_OBJECT_CLASS (gdav_property_parent_class)->constructed (object);
}

static void
gdav_property_class_init (GDavPropertyClass *class)
{
	GObjectClass *object_class;

	g_type_class_add_private (class, sizeof (GDavPropertyPrivate));

	object_class = G_OBJECT_CLASS (class);
	object_class->set_property = gdav_property_set_property;
	object_class->get_property = gdav_property_get_property;
	object_class->dispose = gdav_property_dispose;
	object_class->finalize = gdav_property_finalize;
	object_class->constructed = gdav_property_constructed;

	g_object_class_install_property (
		object_class,
		PROP_VALUE,
		g_param_spec_boxed (
			"value",
			"Value",
			"A GValue containing the property value",
			G_TYPE_VALUE,
			G_PARAM_READWRITE |
			G_PARAM_CONSTRUCT_ONLY |
			G_PARAM_STATIC_STRINGS));
}

static void
gdav_property_init (GDavProperty *property)
{
	property->priv = GDAV_PROPERTY_GET_PRIVATE (property);
}

void
gdav_property_get_value (GDavProperty *property,
                         GValue *value)
{
	g_return_if_fail (GDAV_IS_PROPERTY (property));
	g_return_if_fail (value != NULL);

	g_value_init (value, G_VALUE_TYPE (&property->priv->value));
	g_value_copy (&property->priv->value, value);
}

gboolean
gdav_property_set_value (GDavProperty *property,
                         const GValue *value)
{
	g_return_val_if_fail (GDAV_IS_PROPERTY (property), FALSE);
	g_return_val_if_fail (value != NULL, FALSE);

	return g_value_transform (value, &property->priv->value);
}


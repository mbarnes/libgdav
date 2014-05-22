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

#include "gdav-lockdiscovery-property.h"

G_DEFINE_TYPE (
	GDavLockDiscoveryProperty,
	gdav_lockdiscovery_property,
	GDAV_TYPE_PROPERTY)

static void
gdav_lockdiscovery_property_class_init (GDavLockDiscoveryPropertyClass *class)
{
	GDavParsableClass *parsable_class;
	GDavPropertyClass *property_class;

	parsable_class = GDAV_PARSABLE_CLASS (class);
	parsable_class->element_name = "lockdiscovery";
	parsable_class->element_namespace = GDAV_XMLNS_DAV;

	property_class = GDAV_PROPERTY_CLASS (class);
	property_class->value_type = G_TYPE_PTR_ARRAY;
}

static void
gdav_lockdiscovery_property_init (GDavLockDiscoveryProperty *property)
{
}


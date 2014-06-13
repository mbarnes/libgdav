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

#ifndef __GDAV_SUPPORTEDLOCK_PROPERTY_H__
#define __GDAV_SUPPORTEDLOCK_PROPERTY_H__

#include <libgdav/gdav-property.h>

/* Standard GObject macros */
#define GDAV_TYPE_SUPPORTEDLOCK_PROPERTY \
	(gdav_supportedlock_property_get_type ())
#define GDAV_IS_SUPPORTEDLOCK_PROPERTY(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), GDAV_TYPE_SUPPORTEDLOCK_PROPERTY))

G_BEGIN_DECLS

typedef GDavProperty GDavSupportedLockProperty;
typedef GDavPropertyClass GDavSupportedLockPropertyClass;

GType		gdav_supportedlock_property_get_type
						(void) G_GNUC_CONST;

G_END_DECLS

#endif /* __GDAV_SUPPORTEDLOCK_PROPERTY_H__ */

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

#ifndef __GDAV_ENUMS_H__
#define __GDAV_ENUMS_H__

typedef enum { /*< flags >*/
	GDAV_ALLOW_ACL = 1 << 0,
	GDAV_ALLOW_COPY = 1 << 1,
	GDAV_ALLOW_DELETE = 1 << 2,
	GDAV_ALLOW_GET = 1 << 3,
	GDAV_ALLOW_HEAD = 1 << 4,
	GDAV_ALLOW_LOCK = 1 << 5,
	GDAV_ALLOW_MKCALENDAR = 1 << 6,
	GDAV_ALLOW_MKCOL = 1 << 7,
	GDAV_ALLOW_MOVE = 1 << 8,
	GDAV_ALLOW_OPTIONS = 1 << 9,
	GDAV_ALLOW_POST = 1 << 10,
	GDAV_ALLOW_PROPFIND = 1 << 11,
	GDAV_ALLOW_PROPPATCH = 1 << 12,
	GDAV_ALLOW_PUT = 1 << 13,
	GDAV_ALLOW_REPORT = 1 << 14,
	GDAV_ALLOW_UNLOCK = 1 << 15
} GDavAllow;

typedef enum { /*< flags >*/
	GDAV_COPY_FLAGS_NONE = 0,
	GDAV_COPY_FLAGS_NO_OVERWRITE = 1 << 0,
	GDAV_COPY_FLAGS_COLLECTION_ONLY = 1 << 1
} GDavCopyFlags;

typedef enum {
	GDAV_DEPTH_0,
	GDAV_DEPTH_1,
	GDAV_DEPTH_INFINITY
} GDavDepth;

typedef enum { /*< flags >*/
	GDAV_ERROR_FLAGS_LOCK_TOKEN_MATCHES_REQUEST_URI = 1 << 0,
	GDAV_ERROR_FLAGS_LOCK_TOKEN_SUBMITTED = 1 << 1,
	GDAV_ERROR_FLAGS_NO_CONFLICTING_LOCK = 1 << 2,
	GDAV_ERROR_FLAGS_NO_EXTERNAL_ENTITIES = 1 << 3,
	GDAV_ERROR_FLAGS_PRESERVED_LIVE_PROPERTIES = 1 << 4,
	GDAV_ERROR_FLAGS_PROPFIND_FINITE_DEPTH = 1 << 5,
	GDAV_ERROR_FLAGS_CANNOT_MODIFY_PROTECTED_PROPERTY = 1 << 6,
} GDavErrorFlags;

typedef enum { /*< flags >*/
	GDAV_LOCK_FLAGS_NONE = 0,
	GDAV_LOCK_FLAGS_NON_RECURSIVE = 1 << 0,
	GDAV_LOCK_FLAGS_OWNER_IS_URI = 1 << 1,
} GDavLockFlags;

typedef enum {
	GDAV_LOCK_SCOPE_EXCLUSIVE,
	GDAV_LOCK_SCOPE_SHARED
} GDavLockScope;

typedef enum {
	GDAV_LOCK_TYPE_UNKNOWN,
	GDAV_LOCK_TYPE_WRITE
} GDavLockType;

typedef enum { /*< flags >*/
	GDAV_MOVE_FLAGS_NONE = 0,
	GDAV_MOVE_FLAGS_NO_OVERWRITE = 1 << 0
} GDavMoveFlags;

typedef enum { /*< flags >*/
	GDAV_OPTIONS_COMPLIANCE_CLASS_1 = 1 << 0,
	GDAV_OPTIONS_COMPLIANCE_CLASS_2 = 1 << 1,
	GDAV_OPTIONS_COMPLIANCE_CLASS_3 = 1 << 2,
	GDAV_OPTIONS_ACCESS_CONTROL = 1 << 3,
	GDAV_OPTIONS_VERSION_CONTROL = 1 << 4,
	GDAV_OPTIONS_CALENDAR_ACCESS = 1 << 5,
	GDAV_OPTIONS_CALENDAR_SCHEDULE = 1 << 6,
	GDAV_OPTIONS_CALENDAR_AUTO_SCHEDULE = 1 << 7,
	GDAV_OPTIONS_CALENDAR_PROXY = 1 << 8,
	GDAV_OPTIONS_ADDRESSBOOK = 1 << 9
} GDavOptions;

typedef enum {
	GDAV_PROPFIND_PROP,
	GDAV_PROPFIND_ALLPROP,
	GDAV_PROPFIND_PROPNAME
} GDavPropFindType;

/* Remember to update the xml_map in gdav-resourcetype-property.c */
typedef enum { /*< flags >*/
	GDAV_RESOURCE_TYPE_COLLECTION = 1 << 0
} GDavResourceType;

#endif /* __GDAV_ENUMS_H__ */


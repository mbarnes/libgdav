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

#ifndef __GDAV_H__
#define __GDAV_H__

#include <libgdav/gdav-enums.h>
#include <libgdav/gdav-enumtypes.h>
#include <libgdav/gdav-xml-namespaces.h>

#include <libgdav/gdav-active-lock.h>
#include <libgdav/gdav-date-property.h>
#include <libgdav/gdav-error.h>
#include <libgdav/gdav-href-property.h>
#include <libgdav/gdav-lock-entry.h>
#include <libgdav/gdav-methods.h>
#include <libgdav/gdav-multi-status.h>
#include <libgdav/gdav-parsable.h>
#include <libgdav/gdav-pcdata-property.h>
#include <libgdav/gdav-prop-stat.h>
#include <libgdav/gdav-property.h>
#include <libgdav/gdav-property-set.h>
#include <libgdav/gdav-property-update.h>
#include <libgdav/gdav-requests.h>
#include <libgdav/gdav-response.h>
#include <libgdav/gdav-uint-property.h>
#include <libgdav/gdav-utils.h>

/* DAV Properties */
#include <libgdav/gdav-creationdate-property.h>
#include <libgdav/gdav-displayname-property.h>
#include <libgdav/gdav-getcontentlanguage-property.h>
#include <libgdav/gdav-getcontentlength-property.h>
#include <libgdav/gdav-getcontenttype-property.h>
#include <libgdav/gdav-getetag-property.h>
#include <libgdav/gdav-getlastmodified-property.h>
#include <libgdav/gdav-lockdiscovery-property.h>
#include <libgdav/gdav-resourcetype-property.h>
#include <libgdav/gdav-supportedlock-property.h>

/* CalDAV Properties */
#include <libgdav/gdav-calendar-description-property.h>
#include <libgdav/gdav-calendar-timezone-property.h>
#include <libgdav/gdav-max-resource-size-property.h>
#include <libgdav/gdav-schedule-outbox-url-property.h>
#include <libgdav/gdav-supported-calendar-component-set-property.h>
#include <libgdav/gdav-supported-calendar-data-property.h>

#endif /* __GDAV_H__ */


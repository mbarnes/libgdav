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

#ifndef __GDAV_XML_NAMESPACES_H__
#define __GDAV_XML_NAMESPACES_H__

#include <glib.h>
#include <libxml/tree.h>

#define GDAV_XMLNS_DAV			"DAV:"
#define GDAV_XMLNS_CALDAV		"urn:ietf:params:xml:ns:caldav"
#define GDAV_XMLNS_CARDDAV		"urn:ietf:params:xml:ns:carddav"
#define GDAV_XMLNS_YAHOO		"http://yahoo.com/ns/"

G_BEGIN_DECLS

const gchar *	gdav_get_xmlns_prefix		(const gchar *xmlns_href);
void		gdav_set_xmlns_prefix		(const gchar *xmlns_href,
						 const gchar *xmlns_prefix);
gboolean	gdav_is_xmlns			(xmlNode *node,
						 const gchar *xmlns_href);

G_END_DECLS

#endif /* __GDAV_XML_NAMESPACES_H__ */


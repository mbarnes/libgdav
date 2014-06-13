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

#ifndef __GDAV_PARSABLE_H__
#define __GDAV_PARSABLE_H__

#include <gio/gio.h>
#include <libsoup/soup.h>

/* For convenience to subclasses. */
#include <libgdav/gdav-xml-namespaces.h>

/* Standard GObject macros */
#define GDAV_TYPE_PARSABLE \
	(gdav_parsable_get_type ())
#define GDAV_PARSABLE(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST \
	((obj), GDAV_TYPE_PARSABLE, GDavParsable))
#define GDAV_PARSABLE_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_CAST \
	((cls), GDAV_TYPE_PARSABLE, GDavParsableClass))
#define GDAV_IS_PARSABLE(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE \
	((obj), GDAV_TYPE_PARSABLE))
#define GDAV_IS_PARSABLE_CLASS(cls) \
	(G_TYPE_CHECK_CLASS_TYPE \
	((cls), GDAV_TYPE_PARSABLE))
#define GDAV_PARSABLE_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS \
	((obj), GDAV_TYPE_PARSABLE, GDavParsableClass))

#define GDAV_PARSABLE_ERROR \
	(gdav_parsable_error_quark ())

G_BEGIN_DECLS

typedef struct _GDavParsable GDavParsable;
typedef struct _GDavParsableClass GDavParsableClass;
typedef struct _GDavParsablePrivate GDavParsablePrivate;

struct _GDavParsable {
	GObject parent;
	GDavParsablePrivate *priv;
};

struct _GDavParsableClass {
	GObjectClass parent_class;

	const gchar *element_name;
	const gchar *element_namespace;

	gboolean	(*serialize)		(GDavParsable *parsable,
						 GHashTable *namespaces,
						 xmlDoc *doc,
						 xmlNode *parent,
						 GError **error);
	gboolean	(*deserialize)		(GDavParsable *parsable,
						 SoupURI *base_uri,
						 xmlDoc *doc,
						 xmlNode *node,
						 GError **error);
	void		(*collect_types)	(GDavParsable *parsable,
						 GHashTable *parsable_types);
};

/**
 * GDAV_PARSABLE_ERROR:
 * @GDAV_PARSABLE_ERROR_PARSER_FAILED:
 *   Failed to parse the input data.
 * @GDAV_PARSABLE_ERROR_EMPTY_DOCUMENT:
 *   Empty document.
 * @GDAV_PARSABLE_ERROR_UNKNOWN_ELEMENT:
 *   No #GDavParsable class for XML element.
 * @GDAV_PARSABLE_ERROR_CONTENT_VIOLATION:
 *   Response data violates a Document Type Definition (DTD).
 * @GDAV_PARSABLE_ERROR_INTERNAL:
 *   Internal error while serializing or deserializing.
 *
 * Error codes for manipulating XML data.
 **/
typedef enum {
	GDAV_PARSABLE_ERROR_PARSER_FAILED,
	GDAV_PARSABLE_ERROR_EMPTY_DOCUMENT,
	GDAV_PARSABLE_ERROR_UNKNOWN_ELEMENT,
	GDAV_PARSABLE_ERROR_CONTENT_VIOLATION,
	GDAV_PARSABLE_ERROR_INTERNAL
} GDavParsableError;

GQuark		gdav_parsable_error_quark	(void) G_GNUC_CONST;
GType		gdav_parsable_get_type		(void) G_GNUC_CONST;
gboolean	gdav_parsable_is_a		(xmlNode *node,
						 GType parsable_type);
GType		gdav_parsable_lookup_type	(xmlNode *node,
						 GError **error);
gpointer	gdav_parsable_new_from_data	(GType parsable_type,
						 SoupURI *base_uri,
						 gconstpointer data,
						 gsize data_size,
						 GError **error);
gpointer	gdav_parsable_new_from_xml_node	(GType parsable_type,
						 SoupURI *base_uri,
						 xmlDoc *doc,
						 xmlNode *node,
						 GError **error);
gboolean	gdav_parsable_serialize		(GDavParsable *parsable,
						 GHashTable *namespaces,
						 xmlDoc *doc,
						 xmlNode *parent,
						 GError **error);
gboolean	gdav_parsable_deserialize	(GDavParsable *parsable,
						 SoupURI *base_uri,
						 xmlDoc *doc,
						 xmlNode *node,
						 GError **error);
void		gdav_parsable_collect_types	(GDavParsable *parsable,
						 GHashTable *parsable_types);

/* Helpful utilities for subclasses. */
xmlNode *	gdav_parsable_new_text_child	(GType parsable_type,
						 GHashTable *namespaces,
						 xmlNode *parent,
						 const xmlChar *content);

G_END_DECLS

#endif /* __GDAV_PARSABLE_H__ */


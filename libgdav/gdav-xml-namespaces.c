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

#include "gdav-xml-namespaces.h"

G_LOCK_DEFINE_STATIC (gdav_xmlns_prefixes);
static GOnce gdav_xmlns_prefixes = G_ONCE_INIT;

#define GDAV_ADD_XMLNS_PREFIX(ht, ns, pre) \
	(g_hash_table_replace ( \
		(ht), \
		(gpointer) g_intern_static_string (ns), \
		(gpointer) g_intern_static_string (pre)))

static gpointer
gdav_xmlns_prefixes_init (gpointer data)
{
	GHashTable *hash_table;

	hash_table = g_hash_table_new (g_str_hash, g_str_equal);

	/* xmlns:D='DAV:' */
	GDAV_ADD_XMLNS_PREFIX (hash_table, GDAV_XMLNS_DAV, "D");

	/* xmlns:C='urn:ietf:params:xml:ns:caldav' */
	GDAV_ADD_XMLNS_PREFIX (hash_table, GDAV_XMLNS_CALDAV, "C");

	return hash_table;
}

const gchar *
gdav_get_xmlns_prefix (const gchar *xmlns_href)
{
	GHashTable *hash_table;
	const gchar *xmlns_prefix;

	g_return_val_if_fail (xmlns_href != NULL, NULL);

	g_once (&gdav_xmlns_prefixes, gdav_xmlns_prefixes_init, NULL);
	hash_table = (GHashTable *) gdav_xmlns_prefixes.retval;

	G_LOCK (gdav_xmlns_prefixes);

	xmlns_prefix = g_hash_table_lookup (hash_table, xmlns_href);

	G_UNLOCK (gdav_xmlns_prefixes);

	return xmlns_prefix;
}

void
gdav_set_xmlns_prefix (const gchar *xmlns_href,
                       const gchar *xmlns_prefix)
{
	GHashTable *hash_table;

	g_return_if_fail (xmlns_href != NULL);
	g_return_if_fail (xmlns_prefix != NULL);

	g_once (&gdav_xmlns_prefixes, gdav_xmlns_prefixes_init, NULL);
	hash_table = (GHashTable *) gdav_xmlns_prefixes.retval;

	G_LOCK (gdav_xmlns_prefixes);

	g_hash_table_replace (
		hash_table,
		(gpointer) g_intern_string (xmlns_href),
		(gpointer) g_intern_string (xmlns_prefix));

	G_UNLOCK (gdav_xmlns_prefixes);
}

gboolean
gdav_is_xmlns (xmlNode *node,
               const gchar *xmlns_href)
{
	/* xmlStrcmp() handles NULL arguments gracefully. */
	return (node != NULL && node->ns != NULL &&
		xmlStrcmp (node->ns->href, BAD_CAST xmlns_href) == 0);
}

const gchar *
gdav_xmlns_from_prefix (const gchar *xmlns_prefix)
{
	GHashTableIter iter;
	const gchar *xmlns = NULL;
	gpointer key, value;

	g_return_val_if_fail (xmlns_prefix != NULL, NULL);

	xmlns_prefix = g_intern_string (xmlns_prefix);

	G_LOCK (gdav_xmlns_prefixes);

	g_hash_table_iter_init (&iter, gdav_xmlns_prefixes.retval);

	while (g_hash_table_iter_next (&iter, &key, &value)) {
		if (xmlns_prefix == (gchar *) value) {
			xmlns = key;
			break;
		}
	}

	G_UNLOCK (gdav_xmlns_prefixes);

	return xmlns;
}

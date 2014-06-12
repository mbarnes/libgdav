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

#include "commands.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include "enumtypes.h"
#include "utils.h"

/* for readability */
#define ONLINE_CMD  TRUE
#define OFFLINE_CMD FALSE

#define VARARGS G_MAXINT

const static struct {
	GDavCommand id;
	const gchar *alias;
} command_aliases[] = {
	{ GDAV_COMMAND_LESS, "more" },
	{ GDAV_COMMAND_MKCOL, "mkdir" },
	{ GDAV_COMMAND_DELETE, "rm" },
	{ GDAV_COMMAND_COPY, "cp" },
	{ GDAV_COMMAND_MOVE, "mv" },
	{ GDAV_COMMAND_HELP, "h" },
	{ GDAV_COMMAND_HELP, "?" },
	{ GDAV_COMMAND_QUIT, "exit" },
	{ GDAV_COMMAND_QUIT, "bye" }
};

static void
output_start (const gchar *action,
              const gchar *target)
{
	g_print ("%s '%s':", action, target);
}

static void
output_success (void)
{
	g_print ("%s\n", _("succeeded"));
}

static void
output_result (const GError *error)
{
	if (error == NULL) {
		output_success ();
	} else {
		g_print ("%s:\n%s\n", _("failed"), error->message);
	}
}

static void
handle_ls (GlobalState *state,
           gint argc,
           const gchar **argv)
{
	SoupURI *uri;
	GQueue queue = G_QUEUE_INIT;
	GError *local_error = NULL;

	g_return_if_fail (state->base_uri != NULL);

	if (argc == 0) {
		uri = soup_uri_copy (state->base_uri);
	} else {
		uri = soup_uri_new_with_base (state->base_uri, argv[0]);
	}

	g_return_if_fail (uri != NULL);

	output_start (_("Listing collection"), uri->path);

	get_resource_list (
		state->session, uri, GDAV_DEPTH_1, &queue, &local_error);

	if (local_error != NULL) {
		output_result (local_error);
		g_error_free (local_error);

	} else if (g_queue_is_empty (&queue)) {
		g_print ("%s\n", _("collection is empty"));

	} else {
		Resource *resource;

		output_success ();

		while ((resource = g_queue_pop_head (&queue)) != NULL) {
			if (!soup_uri_equal (uri, resource->href))
				print_resource (resource);
			free_resource (resource);
		}
	}

	soup_uri_free (uri);
}

static void
handle_cd (GlobalState *state,
           gint argc,
           const gchar **argv)
{
	SoupURI *uri;

	if (g_str_equal (argv[0], "-")) {
		if (state->last_uri == NULL) {
			g_print ("%s\n", _("No previous collection"));
			return;
		}
		uri = soup_uri_copy (state->last_uri);
	} else {
		uri = soup_uri_new_with_base (state->base_uri, argv[0]);
	}

	g_return_if_fail (uri != NULL);

	if (set_path (state, uri)) {
		if (state->last_uri != NULL)
			soup_uri_free (state->last_uri);
		state->last_uri = state->base_uri;
		state->base_uri = uri;
	}
}

static void
handle_pwd (GlobalState *state,
            gint argc,
            const gchar **argv)
{
	gchar *uri_string;

	uri_string = soup_uri_to_string (state->base_uri, FALSE);
	g_print (_("Current collection is '%s'"), uri_string);
	putchar ('\n');
	g_free (uri_string);
}

static void
handle_put (GlobalState *state,
            gint argc,
            const gchar **argv)
{
}

static void
handle_get (GlobalState *state,
            gint argc,
            const gchar **argv)
{
}

static void
handle_mget (GlobalState *state,
             gint argc,
             const gchar **argv)
{
}

static void
handle_mput (GlobalState *state,
             gint argc,
             const gchar **argv)
{
}

static void
handle_edit (GlobalState *state,
             gint argc,
             const gchar **argv)
{
}

static void
handle_less (GlobalState *state,
             gint argc,
             const gchar **argv)
{
}

static void
handle_mkcol (GlobalState *state,
              gint argc,
              const gchar **argv)
{
}

static void
handle_cat (GlobalState *state,
            gint argc,
            const gchar **argv)
{
}

static void
handle_delete (GlobalState *state,
               gint argc,
               const gchar **argv)
{
}

static void
handle_rmcol (GlobalState *state,
              gint argc,
              const gchar **argv)
{
}

static void
handle_copy (GlobalState *state,
             gint argc,
             const gchar **argv)
{
}

static void
handle_move (GlobalState *state,
             gint argc,
             const gchar **argv)
{
}

static void
handle_lock (GlobalState *state,
             gint argc,
             const gchar **argv)
{
}

static void
handle_unlock (GlobalState *state,
               gint argc,
               const gchar **argv)
{
}

static void
handle_discover (GlobalState *state,
                 gint argc,
                 const gchar **argv)
{
}

static void
handle_steal (GlobalState *state,
              gint argc,
              const gchar **argv)
{
}

static void
handle_showlocks (GlobalState *state,
                  gint argc,
                  const gchar **argv)
{
}

static void
handle_version (GlobalState *state,
                gint argc,
                const gchar **argv)
{
}

static void
handle_checkin (GlobalState *state,
                gint argc,
                const gchar **argv)
{
}

static void
handle_checkout (GlobalState *state,
                 gint argc,
                 const gchar **argv)
{
}

static void
handle_uncheckout (GlobalState *state,
                   gint argc,
                   const gchar **argv)
{
}

static void
handle_history (GlobalState *state,
                gint argc,
                const gchar **argv)
{
}

static void
handle_label (GlobalState *state,
              gint argc,
              const gchar **argv)
{
}

static void
handle_propnames (GlobalState *state,
                  gint argc,
                  const gchar **argv)
{
}

static void
handle_chexec (GlobalState *state,
               gint argc,
               const gchar **argv)
{
}

static void
handle_propget (GlobalState *state,
                gint argc,
                const gchar **argv)
{
}

static void
handle_propdel (GlobalState *state,
                gint argc,
                const gchar **argv)
{
}

static void
handle_propset (GlobalState *state,
                gint argc,
                const gchar **argv)
{
}

static void
handle_set (GlobalState *state,
            gint argc,
            const gchar **argv)
{
}

static void
handle_open (GlobalState *state,
             gint argc,
             const gchar **argv)
{
	open_connection (state, argv[0]);
}

static void
handle_close (GlobalState *state,
              gint argc,
              const gchar **argv)
{
	close_connection (state);
}

static void
handle_echo (GlobalState *state,
             gint argc,
             const gchar **argv)
{
	gint ii = 0;

	for (ii = 0; ii < argc; ii++) {
		if (ii > 0)
			putchar (' ');
		g_print ("%s", argv[ii]);
	}
	putchar ('\n');
}

static void
handle_unset (GlobalState *state,
              gint argc,
              const gchar **argv)
{
}

static void
handle_lcd (GlobalState *state,
            gint argc,
            const gchar **argv)
{
	/* FIXME Do word expansions with wordexp() - if available. */
	if (g_chdir (argc > 0 ? argv[0] : g_get_home_dir ()) < 0)
		perror (_("Could not change local directory:\nchdir"));
}

static void
handle_lls (GlobalState *state,
            gint argc,
            const gchar **argv)
{
	gchar **child_argv;
	gint ii;
	GError *local_error = NULL;

	/* Reserve extra slots for the program name and NULL terminator. */
	child_argv = g_new0 (gchar *, argc + 2);

	child_argv[0] = g_strdup ("ls");
	for (ii = 0; ii < argc; ii++)
		child_argv[ii + 1] = g_strdup (argv[ii]);

	g_spawn_async_with_pipes (
		NULL, child_argv, NULL,
		G_SPAWN_SEARCH_PATH |
		G_SPAWN_DO_NOT_REAP_CHILD,
		NULL, NULL, NULL,
		NULL, NULL, NULL,
		&local_error);

	if (local_error == NULL) {
		wait (NULL);
	} else {
		g_printerr ("%s\n", local_error->message);
		g_error_free (local_error);
	}

	g_strfreev (child_argv);
}

static void
handle_lpwd (GlobalState *state,
             gint argc,
             const gchar **argv)
{
	g_print (_("Local directory: %s\n"), g_get_current_dir ());
}

static void
handle_logout (GlobalState *state,
               gint argc,
               const gchar **argv)
{
}

static void
handle_help (GlobalState *state,
             gint argc,
             const gchar **argv)
{
	const Command *command;

	if (argc == 0) {
		print_commands ();
		return;
	}

	if ((command = get_command (argv[0])) == NULL) {
		g_print (_("Command name not known: %s\n"), argv[0]);
		return;
	}

	g_print (" '%s'   %s\n", command->usage, gettext (command->blurb));
	if (command->needs_connection)
		g_print (_("This command can only be used when connected to a server.\n"));
}

static void
handle_about (GlobalState *state,
              gint argc,
              const gchar **argv)
{
	print_version ();
}

const static Command commands[] = {
	{ GDAV_COMMAND_LS, handle_ls,
	  ONLINE_CMD, 0, 1, "ls [PATH]",
	  N_("List contents of current [or other] collection") },

	{ GDAV_COMMAND_CD, handle_cd,
	  ONLINE_CMD, 1, 1, "cd PATH",
	  N_("Change to specified collection") },

	{ GDAV_COMMAND_PWD, handle_pwd,
	  ONLINE_CMD, 0, 0, "pwd",
	  N_("Display name of current collection") },

	{ GDAV_COMMAND_PUT, handle_put,
	  ONLINE_CMD, 1, 2, "put LOCAL [REMOTE]",
	  N_("Upload local file") },

	{ GDAV_COMMAND_GET, handle_get,
	  ONLINE_CMD, 1, 2, "get REMOTE [LOCAL]",
	  N_("Download remote resource") },

	{ GDAV_COMMAND_MGET, handle_mget,
	  ONLINE_CMD, 1, VARARGS, "mget REMOTE...",
	  N_("Download many remote resources") },

	{ GDAV_COMMAND_MPUT, handle_mput,
	  ONLINE_CMD, 1, VARARGS, "mput LOCAL...",
	  N_("Upload many local files") },

	{ GDAV_COMMAND_EDIT, handle_edit,
	  ONLINE_CMD, 1, 1, "edit REMOTE",
	  N_("Edit remote resource") },

	{ GDAV_COMMAND_LESS, handle_less,
	  ONLINE_CMD, 1, VARARGS, "less REMOTE...",
	  N_("Display remote resource(s) through pager") },

	{ GDAV_COMMAND_MKCOL, handle_mkcol,
	  ONLINE_CMD, 1, VARARGS, "mkcol REMOTE...",
	  N_("Create remote collection(s)") },

	{ GDAV_COMMAND_CAT, handle_cat,
	  ONLINE_CMD, 1, VARARGS, "cat REMOTE...",
	  N_("Display remote resource(s)") },

	{ GDAV_COMMAND_DELETE, handle_delete,
	  ONLINE_CMD, 1, VARARGS, "delete REMOTE...",
	  N_("Delete non-collection resource(s)") },

	{ GDAV_COMMAND_RMCOL, handle_rmcol,
	  ONLINE_CMD, 1, VARARGS, "rmcol REMOTE...",
	  N_("Delete remote collection(s) and ALL contents") },

	{ GDAV_COMMAND_COPY, handle_copy,
	  ONLINE_CMD, 2, VARARGS, "copy SOURCE... DEST",
	  N_("Copy remote resource(s)") },

	{ GDAV_COMMAND_MOVE, handle_move,
	  ONLINE_CMD, 2, VARARGS, "move SOURCE... DEST",
	  N_("Move remote resource(s)") },

	/* Locking Comands */

	{ GDAV_COMMAND_LOCK, handle_lock,
	  ONLINE_CMD, 1, 1, "lock REMOTE",
	  N_("Lock remote resource") },

	{ GDAV_COMMAND_UNLOCK, handle_unlock,
	  ONLINE_CMD, 1, 1, "unlock REMOTE",
	  N_("Unlock remote resource") },

	{ GDAV_COMMAND_DISCOVER, handle_discover,
	  ONLINE_CMD, 1, 1, "discover REMOTE",
	  N_("Display lock information for remote resource") },

	{ GDAV_COMMAND_STEAL, handle_steal,
	  ONLINE_CMD, 1, 1, "steal REMOTE",
	  N_("Steal lock token for remote resource") },

	{ GDAV_COMMAND_SHOWLOCKS, handle_showlocks,
	  ONLINE_CMD, 0, 0, "showlocks",
	  N_("Display list of owned locks") },

	/* Versioning Commands */

	{ GDAV_COMMAND_VERSION, handle_version,
	  ONLINE_CMD, 1, 1, "version REMOTE",
	  N_("Place remote resource under version control") },

	{ GDAV_COMMAND_CHECKIN, handle_checkin,
	  ONLINE_CMD, 1, 1, "checkin REMOTE",
	  N_("Check in remote resource") },

	{ GDAV_COMMAND_CHECKOUT, handle_checkout,
	  ONLINE_CMD, 1, 1, "checkout REMOTE",
	  N_("Check out remote resource") },

	{ GDAV_COMMAND_UNCHECKOUT, handle_uncheckout,
	  ONLINE_CMD, 1, 1, "uncheckout REMOTE",
	  N_("Cancel remote resource check out") },

	{ GDAV_COMMAND_HISTORY, handle_history,
	  ONLINE_CMD, 1, 1, "history REMOTE",
	  N_("Show version history of remote resource") },

	{ GDAV_COMMAND_LABEL, handle_label,
	  ONLINE_CMD, 3, 3, "label REMOTE add|set|remove LABELNAME",
	  N_("Add/change/remove label on remote resource") },

	/* Property Handling */

	{ GDAV_COMMAND_PROPNAMES, handle_propnames,
	  ONLINE_CMD, 1, 1, "propnames REMOTE",
	  N_("List property names defined on remote resource") },

	{ GDAV_COMMAND_CHEXEC, handle_chexec,
	  ONLINE_CMD, 2, 2, "chexec +|- REMOTE",
	  N_("Change 'isexecutable' property on remote resource") },

	{ GDAV_COMMAND_PROPGET, handle_propget,
	  ONLINE_CMD, 1, 2, "propget REMOTE [PROPNAME]",
	  N_("Retrieve properties of remote resource") },

	{ GDAV_COMMAND_PROPDEL, handle_propdel,
	  ONLINE_CMD, 2, 2, "propdel REMOTE PROPNAME",
	  N_("Delete property from remote resource") },

	{ GDAV_COMMAND_PROPSET, handle_propset,
	  ONLINE_CMD, 3, 3, "propset REMOTE PROPNAME VALUE",
	  N_("Set property on remote resource") },

#if 0  /* maybe skip this? */
	{ GDAV_COMMAND_SEARCH },
#endif

	{ GDAV_COMMAND_SET, handle_set,
	  OFFLINE_CMD, 0, 2, "set [OPTION] [VALUE]",
	  N_("Set an option, or display options") },

	{ GDAV_COMMAND_OPEN, handle_open,
	  OFFLINE_CMD, 1, 1, "open URL",
	  N_("Open a connection to a WebDAV server") },

	{ GDAV_COMMAND_CLOSE, handle_close,
	  ONLINE_CMD, 0, 0, "close",
	  N_("Close current connection") },

	{ GDAV_COMMAND_ECHO, handle_echo,
	  OFFLINE_CMD, 1, VARARGS, "echo WORDS...",
	  N_("Echo a message to the console") },

	/* Handled directly in main.c */
	{ GDAV_COMMAND_QUIT, NULL,
	  OFFLINE_CMD, 0, 0, "quit",
	  N_("Exit program") },

	{ GDAV_COMMAND_UNSET, handle_unset,
	  OFFLINE_CMD, 1, 2, "unset OPTION [VALUE]",
	  N_("Unsets or clears value from option") },

	/* Local Commands */

	{ GDAV_COMMAND_LCD, handle_lcd,
	  OFFLINE_CMD, 0, 1, "lcd [DIRECTORY]",
	  N_("Change local working directory") },

	{ GDAV_COMMAND_LLS, handle_lls,
	  OFFLINE_CMD, 0, VARARGS, "lls [OPTIONS]",
	  N_("Print local directory listing") },

	{ GDAV_COMMAND_LPWD, handle_lpwd,
	  OFFLINE_CMD, 0, 0, "lpwd",
	  N_("Print local working directory") },

	{ GDAV_COMMAND_LOGOUT, handle_logout,
	  ONLINE_CMD, 0, 0, "logout",
	  N_("Log out of authenticated session") },

	/* "help" alone displays command list */
	{ GDAV_COMMAND_HELP, handle_help,
	  OFFLINE_CMD, 0, 1, "help [COMMAND]",
	  N_("Display help message") },

	{ GDAV_COMMAND_ABOUT, handle_about,
	  OFFLINE_CMD, 0, 0, "about",
	  N_("Display program information") }
};

const Command *
get_command (const gchar *name)
{
	GDavCommand id = GDAV_COMMAND_UNKNOWN;
	GEnumClass *class;
	guint ii;

	class = g_type_class_ref (GDAV_TYPE_COMMAND);

	/* Check proper command names. */
	for (ii = 0; ii < class->n_values; ii++) {
		const gchar *cmdname;

		/* UNKNOWN is not a real command - skip it. */
		if (class->values[ii].value == GDAV_COMMAND_UNKNOWN)
			continue;

		cmdname = class->values[ii].value_nick;
		if (g_ascii_strcasecmp (name, cmdname) == 0) {
			id = class->values[ii].value;
			break;
		}
	}

	g_type_class_unref (class);

	/* Check command aliases. */
	if (id == GDAV_COMMAND_UNKNOWN) {
		for (ii = 0; ii < G_N_ELEMENTS (command_aliases); ii++) {
			const gchar *cmdname;

			cmdname = command_aliases[ii].alias;
			if (g_ascii_strcasecmp (name, cmdname) == 0) {
				id = command_aliases[ii].id;
				break;
			}
		}
	}

	/* Find an return a matching Command struct. */
	if (id != GDAV_COMMAND_UNKNOWN) {
		for (ii = 0; ii < G_N_ELEMENTS (commands); ii++) {
			if (id == commands[ii].id)
				return &commands[ii];
		}
	}

	return NULL;
}

void
print_commands (void)
{
	GEnumClass *class;
	guint column = 0;
	guint ii;

	class = g_type_class_ref (GDAV_TYPE_COMMAND);

	g_print ("Available commands: \n ");

	for (ii = 0; ii < G_N_ELEMENTS (commands); ii++) {
		GEnumValue *value;

		value = g_enum_get_value (class, commands[ii].id);
		g_return_if_fail (value != NULL);

		g_print ("%-11s", value->value_nick);
		if (column == 6)
			g_print ("\n ");
		column = (column + 1) % 7;
	}

	putchar ((column == 6) ? '\r' : '\n');

	g_print (
		"%s: rm=delete, mkdir=mkcol, mv=move, cp=copy, "
		"more=less, quit=exit=bye\n", _("Aliases"));

	g_type_class_unref (class);
}

void
print_version (void)
{
	g_print ("%s %s\n",
		g_get_prgname (),
		PACKAGE_VERSION);
	g_print ("libsoup %u.%u.%u\n",
		soup_get_major_version (),
		soup_get_minor_version (),
		soup_get_micro_version ());
	g_print ("libxml2 %s\n",
		LIBXML_DOTTED_VERSION);
	g_print ("glib %u.%u.%u\n",
		glib_major_version,
		glib_minor_version,
		glib_micro_version);
}

void
print_resource (Resource *resource)
{
	const gchar *type;
	gchar *name;
	gsize len;

	g_return_if_fail (resource != NULL);

	if (resource->type & GDAV_RESOURCE_TYPE_COLLECTION)
		type = "Coll:";
	else if (resource->type & GDAV_RESOURCE_TYPE_REDIRECTREF)
		type = "Ref:";
	else
		type = "";

	len = strlen (resource->href->path);
	if (resource->href->path[len - 1] == '/')
		resource->href->path[len - 1] = '\0';

	name = strrchr (resource->href->path, '/');
	if (name != NULL && strlen (name + 1) > 0)
		name++;
	else
		name = resource->href->path;

	/* Allocates a new string. */
	name = soup_uri_decode (name);

	if (SOUP_STATUS_IS_SUCCESSFUL (resource->status)) {
		gchar *date;

		date = soup_date_to_string (
			resource->last_modified, SOUP_DATE_RFC2822);
		g_print (
			"%5s %-29s %10" G_GUINT64_FORMAT "  %s\n",
			type, name, resource->content_length, date);
		g_free (date);
	} else {
		g_print (
			"%s: %-30s %u %s\n",
			_("Error"), name,
			resource->status,
			(resource->reason_phrase != NULL) ?
			resource->reason_phrase : _("unspecified"));
	}
}


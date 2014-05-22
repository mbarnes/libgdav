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

#include <locale.h>
#include <stdlib.h>
#include <stdio.h>

#include <histedit.h>

#include <glib/gi18n.h>

#include "commands.h"
#include "utils.h"

#define PARAMETER_STRING	"http://hostname[:port]/path"

static GlobalState state;
static gchar *opt_log;
static gchar *opt_proxy;
static gboolean opt_version;
static gchar **opt_remaining;

static GOptionEntry options[] = {
	{ "log", 'l', 0,
	  G_OPTION_ARG_STRING, &opt_log,
	  "Log HTTP traffic for debugging",
	  "minimal|headers|body" },
	{ "proxy", 'p', 0,
	  G_OPTION_ARG_STRING, &opt_proxy,
	  "Use HTTP proxy host PROXY and optional PORT",
	  "PROXY[:PORT]" },
	{ "version", 'V', 0,
	  G_OPTION_ARG_NONE, &opt_version,
	  "Show version information", NULL },
	{ G_OPTION_REMAINING, 0, 0,
	  G_OPTION_ARG_STRING_ARRAY, &opt_remaining, NULL, NULL },

	{ NULL }
};

static void
parse_args (gint argc,
            gchar **argv)
{
	GOptionContext *context;
	GError *local_error = NULL;

	context = g_option_context_new (PARAMETER_STRING);
	g_option_context_add_main_entries (context, options, GETTEXT_PACKAGE);

	if (!g_option_context_parse (context, &argc, &argv, &local_error)) {
		const gchar *prgname = g_get_prgname ();
		g_print ("%s: %s\n", prgname, local_error->message);
		g_print (_("Try '%s --help' for more information.\n"), prgname);
		exit (-1);
	}

	if (opt_version) {
		print_version ();
		exit (0);
	}
}

static void
authenticate (SoupSession *session,
              SoupMessage *message,
              SoupAuth *auth,
              gboolean retrying,
              EditLine *el)
{
	const gchar *prompt;
	gchar *username = NULL;
	gchar *password = NULL;
	gint length;

	if (soup_auth_is_for_proxy (auth))
		prompt = _("Authentication required for %s on proxy server '%s'");
	else
		prompt = _("Authentication required for %s on server '%s'");

	g_print (
		prompt,
		soup_auth_get_realm (auth),
		soup_auth_get_host (auth));
	g_print (":\n");

	state.prompt_username = TRUE;
	username = g_strstrip (g_strdup (el_gets (el, &length)));
	state.prompt_username = FALSE;

	if (username != NULL)
		password = g_strdup (getpass (_("Password: ")));

	if (username != NULL && password != NULL) {
		soup_auth_authenticate (auth, username, password);
	} else {
		g_print (_("\rAuthentication aborted!\n"));
	}

	g_free (username);
	g_free (password);
}

static void
config_session (SoupSession *session,
                EditLine *el)
{
	g_signal_connect (
		session, "authenticate",
		G_CALLBACK (authenticate), el);

	/* Trailing space tells libsoup to append its name/version. */
	g_object_set (
		session,
		SOUP_SESSION_USER_AGENT,
		"gdav/" VERSION " ", NULL);

	if (opt_log != NULL) {
		SoupLogger *logger;
		SoupLoggerLogLevel level;

		/* Be forgiving and default to "headers". */
		if (g_ascii_strcasecmp (opt_log, "minimal") == 0)
			level = SOUP_LOGGER_LOG_MINIMAL;
		else if (g_ascii_strcasecmp (opt_log, "body") == 0)
			level = SOUP_LOGGER_LOG_BODY;
		else
			level = SOUP_LOGGER_LOG_HEADERS;

		logger = soup_logger_new (level, -1);
		soup_session_add_feature (
			session, SOUP_SESSION_FEATURE (logger));
		g_object_unref (logger);
	}

	if (opt_proxy != NULL) {
		GProxyResolver *proxy_resolver;
		gchar *proxy_uri;

		proxy_uri = g_strdup_printf ("http://%s", opt_proxy);
		proxy_resolver = g_simple_proxy_resolver_new (proxy_uri, NULL);
		g_object_set (
			session,
			SOUP_SESSION_PROXY_RESOLVER,
			proxy_resolver, NULL);
		g_object_unref (proxy_resolver);
		g_free (proxy_uri);
	}
}

static gchar *
get_prompt (EditLine *el)
{
	static gchar prompt[BUFSIZ];

	if (state.prompt_username) {
		g_snprintf (prompt, BUFSIZ, _("Username: "));
	} else if (state.base_uri != NULL && state.base_uri->path != NULL) {
		gchar *path;
		gchar ch = state.isdav ? '>' : '?';

		path = soup_uri_decode (state.base_uri->path);
		g_snprintf (prompt, BUFSIZ, "dav:%s%c ", path, ch);
		g_free (path);
	} else {
		g_snprintf (prompt, BUFSIZ, "dav:! ");
	}

	return prompt;
}

static gboolean
execute_command (gint argc,
                 const gchar **argv)
{
	const Command *command;

	if (argc == 0)
		return TRUE;

	command = get_command (argv[0]);
	argc--;

	if (command == NULL) {
		g_print (
			"%s %s\n",
			_("Unrecognized command."),
			_("Type 'help' for a list of commands."));
	} else if (argc < command->min_args) {
		g_print (ngettext (
			"The '%s' command requires %d argument",
			"The '%s' command requires %d arguments",
			command->min_args),
			argv[0], command->min_args);
		if (command->blurb != NULL) {
			g_print (
				":\n  %s : %s\n",
				command->usage,
				command->blurb);
		} else {
			g_print (".\n");
		}
	} else if (argc > command->max_args) {
		if (command->max_args > 0) {
			g_print (ngettext (
				"The '%s' command takes at most %d argument",
				"The '%s' command takes at most %d arguments",
				command->max_args),
				argv[0], command->max_args);
		} else {
			g_print (
				_("The '%s' command takes no arguments"),
				argv[0]);
		}
		if (command->blurb != NULL) {
			g_print (
				":\n  %s : %s\n",
				command->usage,
				command->blurb);
		} else {
			g_print (".\n");
		}
	} else if (!state.connected && command->needs_connection) {
		g_print (
			_("The '%s' command can only be used when connected to the server.\n"
			  "Try running 'open' first (see 'help open' for more details).\n"),
			argv[0]);
	} else if (command->id == GDAV_COMMAND_QUIT) {
		return FALSE;
	} else {
		g_return_val_if_fail (command->handler != NULL, FALSE);

		/* Already decremented argc. */
		command->handler (&state, argc, argv + 1);
	}

	return TRUE;
}

gint
main (gint argc,
      gchar **argv)
{
	EditLine *el;
	History *hist;
	HistEvent ev;
	Tokenizer *tok;
	gboolean stop = FALSE;

	setlocale (LC_ALL, "");
	bindtextdomain (PACKAGE_NAME, LOCALEDIR);
	textdomain (PACKAGE_NAME);

	state.session = soup_session_new ();

	parse_args (argc, argv);

	hist = history_init ();
	history (hist, &ev, H_SETSIZE, 100);

	tok = tok_init (NULL);

	el = el_init (g_get_prgname (), stdin, stdout, stderr);

	el_set (el, EL_PROMPT, get_prompt);
	el_set (el, EL_HIST, history, hist);

	/* Process .editrc */
	el_source (el, NULL);

	config_session (state.session, el);

	/* If a URI was given on command-line, open it. */
	if (opt_remaining != NULL) {
		gchar *line;

		open_connection (&state, opt_remaining[0]);

		line = g_strdup_printf ("open %s", opt_remaining[0]);
		history (hist, &ev, H_ENTER, line);
		g_free (line);
	}

	while (!stop) {
		const gchar *line;
		const gchar **tokens;
		gint line_len = 0;
		gint n_tokens = 0;

		line = el_gets (el, &line_len);

		if (line == NULL)
			break;

		/* Tokenizer retains ownership of the token array. */
		switch (tok_str (tok, line, &n_tokens, &tokens)) {
			case 0:
				history (hist, &ev, H_ENTER, line);
				if (!execute_command (n_tokens, tokens))
					stop = TRUE;
				break;

			case -1:
				/* internal error */
				stop = TRUE;
				break;

			default:
				/* read another line */
				break;
		}

		tok_reset (tok);
	}

	/* Free global state. */
	close_connection (&state);
	g_clear_object (&state.session);

	/* Free editline structs. */
	el_end (el);
	tok_end (tok);
	history_end (hist);

	/* Free options. */
	g_free (opt_log);
	g_free (opt_proxy);
	g_strfreev (opt_remaining);

	return 0;
}

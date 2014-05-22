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

#ifndef __ENUMS_H__
#define __ENUMS_H__

typedef enum {
	GDAV_COMMAND_LS,
	GDAV_COMMAND_CD,
	GDAV_COMMAND_QUIT,
	GDAV_COMMAND_OPEN,
	GDAV_COMMAND_CLOSE,
	GDAV_COMMAND_ABOUT,
	GDAV_COMMAND_PWD,
	GDAV_COMMAND_HELP,
	GDAV_COMMAND_PUT,
	GDAV_COMMAND_GET,
	GDAV_COMMAND_MKCOL,
	GDAV_COMMAND_DELETE,
	GDAV_COMMAND_MOVE,
	GDAV_COMMAND_COPY,
	GDAV_COMMAND_LESS,
	GDAV_COMMAND_CAT,
	GDAV_COMMAND_LPWD,
	GDAV_COMMAND_LCD,
	GDAV_COMMAND_LLS,
	GDAV_COMMAND_MPUT,
	GDAV_COMMAND_MGET,
	GDAV_COMMAND_ECHO,
	GDAV_COMMAND_SET,
	GDAV_COMMAND_UNSET,
	GDAV_COMMAND_RMCOL,
	GDAV_COMMAND_LOCK,
	GDAV_COMMAND_UNLOCK,
	GDAV_COMMAND_STEAL,
	GDAV_COMMAND_DISCOVER,
	GDAV_COMMAND_SHOWLOCKS,
	GDAV_COMMAND_PROPEDIT,
	GDAV_COMMAND_PROPNAMES,
	GDAV_COMMAND_PROPGET,
	GDAV_COMMAND_PROPSET,
	GDAV_COMMAND_PROPDEL,
	GDAV_COMMAND_CHEXEC,
	GDAV_COMMAND_EDIT,
	GDAV_COMMAND_LOGOUT,
	GDAV_COMMAND_DESCRIBE,
	GDAV_COMMAND_SEARCH,
	GDAV_COMMAND_VERSION,
	GDAV_COMMAND_CHECKIN,
	GDAV_COMMAND_CHECKOUT,
	GDAV_COMMAND_UNCHECKOUT,
	GDAV_COMMAND_HISTORY,
	GDAV_COMMAND_LABEL,
	GDAV_COMMAND_UNKNOWN
} GDavCommand;

#endif /* __ENUMS_H__ */


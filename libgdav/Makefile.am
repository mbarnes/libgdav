NULL =

ENUM_TYPES = gdav-enums.h
ENUM_GENERATED = gdav-enumtypes.h gdav-enumtypes.c

gdav-enumtypes.h: $(top_srcdir)/enumtypes.h.template $(ENUM_TYPES)
	$(AM_V_GEN) $(GLIB_MKENUMS) \
	--identifier-prefix "GDav" \
	--template $(top_srcdir)/enumtypes.h.template \
	--fhead "#ifndef __GDAV_ENUMTYPES_H__\n#define __GDAV_ENUMTYPES_H__\n" \
	--ftail "#endif /* __GDAV_ENUMTYPES_H__ */\n" \
	$(addprefix $(srcdir)/,$(ENUM_TYPES)) > $@

gdav-enumtypes.c: $(top_srcdir)/enumtypes.c.template $(ENUM_TYPES)
	$(AM_V_GEN) $(GLIB_MKENUMS) \
	--identifier-prefix "GDav" \
	--template $(top_srcdir)/enumtypes.c.template \
	--fhead "#include \"gdav-enumtypes.h\"" \
	$(addprefix $(srcdir)/,$(ENUM_TYPES)) > $@

BUILT_SOURCES = $(ENUM_GENERATED)

lib_LTLIBRARIES = libgdav.la

libgdav_la_CPPFLAGS = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"libgdav\" \
	$(NULL)

libgdav_la_CFLAGS = \
	$(LIBSOUP_CFLAGS) \
	$(LIBXML2_CFLAGS) \
	$(GIO_CFLAGS) \
	$(NULL)

libgdavincludedir = $(pkgincludedir)/libgdav

libgdav_headers = \
	gdav.h \
	gdav-active-lock.h \
	gdav-calendar-description-property.h \
	gdav-calendar-timezone-property.h \
	gdav-creationdate-property.h \
	gdav-displayname-property.h \
	gdav-getcontentlanguage-property.h \
	gdav-getcontentlength-property.h \
	gdav-getcontenttype-property.h \
	gdav-getetag-property.h \
	gdav-getlastmodified-property.h \
	gdav-enums.h \
	gdav-enumtypes.h \
	gdav-error.h \
	gdav-lock-entry.h \
	gdav-lockdiscovery-property.h \
	gdav-max-resource-size-property.h \
	gdav-methods.h \
	gdav-multi-status.h \
	gdav-parsable.h \
	gdav-prop-stat.h \
	gdav-property.h \
	gdav-property-set.h \
	gdav-property-update.h \
	gdav-requests.h \
	gdav-resourcetype-property.h \
	gdav-response.h \
	gdav-supported-calendar-component-set-property.h \
	gdav-supported-calendar-data-property.h \
	gdav-supportedlock-property.h \
	gdav-utils.h \
	gdav-xml-namespaces.h \
	$(NULL)

libgdav_la_SOURCES = \
	$(BUILT_SOURCES) \
	$(libgdav_headers) \
	gdav-active-lock.c \
	gdav-calendar-description-property.c \
	gdav-calendar-timezone-property.c \
	gdav-creationdate-property.c \
	gdav-displayname-property.c \
	gdav-getcontentlanguage-property.c \
	gdav-getcontentlength-property.c \
	gdav-getcontenttype-property.c \
	gdav-getetag-property.c \
	gdav-getlastmodified-property.c \
	gdav-error.c \
	gdav-lock-entry.c \
	gdav-lockdiscovery-property.c \
	gdav-max-resource-size-property.c \
	gdav-methods.c \
	gdav-multi-status.c \
	gdav-parsable.c \
	gdav-prop-stat.c \
	gdav-property.c \
	gdav-property-set.c \
	gdav-property-update.c \
	gdav-requests.c \
	gdav-resourcetype-property.c \
	gdav-response.c \
	gdav-supported-calendar-component-set-property.c \
	gdav-supported-calendar-data-property.c \
	gdav-supportedlock-property.c \
	gdav-utils.c \
	gdav-xml-namespaces.c \
	$(NULL)

libgdav_la_LIBADD = \
	$(LIBSOUP_LIBS) \
	$(LIBXML2_LIBS) \
	$(GIO_LIBS) \
	$(NULL)

CLEANFILES = $(BUILT_SOURCES)

-include $(top_srcdir)/git.mk

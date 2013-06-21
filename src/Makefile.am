### Makefile.am
include $(top_builddir)/version.mk

AM_CFLAGS = $(EXTRA_CFLAGS)
AM_CPPFLAGS = -D_GNU_SOURCE -D_POSIX_C_SOURCE=201001L -D_XOPEN_SOURCE=800 -D_BSD_SOURCE
AM_LDFLAGS = $(XCCLDFLAGS)

bin_PROGRAMS =
noinst_PROGRAMS =
lib_LTLIBRARIES =
noinst_LTLIBRARIES =
pkglib_LTLIBRARIES =
noinst_HEADERS =
BUILT_SOURCES =
EXTRA_DIST = $(BUILT_SOURCES)
CLEANFILES = 

CLEANFILES += version.c
EXTRA_DIST += version.c.in

lib_LTLIBRARIES += librotz.la
librotz_la_SOURCES = rotz.c rotz.h
librotz_la_SOURCES += nifty.h
librotz_la_CPPFLAGS = $(AM_CPPFLAGS)
librotz_la_CPPFLAGS += $(tokyocabinet_CFLAGS)
librotz_la_CPPFLAGS += $(lmdb_CFLAGS)
librotz_la_LDFLAGS = $(AM_LDFLAGS)
librotz_la_LDFLAGS += $(tokyocabinet_LIBS)
librotz_la_LDFLAGS += $(lmdb_LIBS)
EXTRA_librotz_la_SOURCES =
EXTRA_librotz_la_SOURCES += rotz-tcbdb.c
EXTRA_librotz_la_SOURCES += rotz-lmdb.c

noinst_LTLIBRARIES += libraux.la
libraux_la_SOURCES = raux.c raux.h
libraux_la_SOURCES += version.c version.h
libraux_la_SOURCES += nifty.h

bin_PROGRAMS += rotz-add
rotz_add_SOURCES = rotz-add.c
rotz_add_SOURCES += rotz-cmd-api.h
rotz_add_SOURCES += rotz-add-clo.h rotz-add-clo.ggo
rotz_add_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_add_LDFLAGS = $(AM_LDFLAGS)
rotz_add_LDADD = librotz.la
rotz_add_LDADD += libraux.la
BUILT_SOURCES += rotz-add-clo.c rotz-add-clo.h

bin_PROGRAMS += rotz-del
rotz_del_SOURCES = rotz-del.c
rotz_del_SOURCES += rotz-cmd-api.h
rotz_del_SOURCES += rotz-del-clo.h rotz-del-clo.ggo
rotz_del_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_del_LDFLAGS = $(AM_LDFLAGS)
rotz_del_LDADD = librotz.la
rotz_del_LDADD += libraux.la
BUILT_SOURCES += rotz-del-clo.c rotz-del-clo.h

bin_PROGRAMS += rotz-show
rotz_show_SOURCES = rotz-show.c
rotz_show_SOURCES += rotz-cmd-api.h
rotz_show_SOURCES += rotz-show-clo.h rotz-show-clo.ggo
rotz_show_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_show_LDFLAGS = $(AM_LDFLAGS)
rotz_show_LDADD = librotz.la
rotz_show_LDADD += libraux.la
BUILT_SOURCES += rotz-show-clo.c rotz-show-clo.h

bin_PROGRAMS += rotz-alias
rotz_alias_SOURCES = rotz-alias.c
rotz_alias_SOURCES += rotz-cmd-api.h
rotz_alias_SOURCES += rotz-alias-clo.h rotz-alias-clo.ggo
rotz_alias_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_alias_LDFLAGS = $(AM_LDFLAGS)
rotz_alias_LDADD = librotz.la
rotz_alias_LDADD += libraux.la
BUILT_SOURCES += rotz-alias-clo.c rotz-alias-clo.h

bin_PROGRAMS += rotz-combine
rotz_combine_SOURCES = rotz-combine.c
rotz_combine_SOURCES += rotz-cmd-api.h
rotz_combine_SOURCES += rotz-combine-clo.h rotz-combine-clo.ggo
rotz_combine_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_combine_LDFLAGS = $(AM_LDFLAGS)
rotz_combine_LDADD = librotz.la
rotz_combine_LDADD += libraux.la
BUILT_SOURCES += rotz-combine-clo.c rotz-combine-clo.h

bin_PROGRAMS += rotz-rename
rotz_rename_SOURCES = rotz-rename.c
rotz_rename_SOURCES += rotz-cmd-api.h
rotz_rename_SOURCES += rotz-rename-clo.h rotz-rename-clo.ggo
rotz_rename_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_rename_LDFLAGS = $(AM_LDFLAGS)
rotz_rename_LDADD = librotz.la
rotz_rename_LDADD += libraux.la
BUILT_SOURCES += rotz-rename-clo.c rotz-rename-clo.h

bin_PROGRAMS += rotz-cloud
rotz_cloud_SOURCES = rotz-cloud.c
rotz_cloud_SOURCES += rotz-cmd-api.h
rotz_cloud_SOURCES += rotz-cloud-clo.h rotz-cloud-clo.ggo
rotz_cloud_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_cloud_LDFLAGS = $(AM_LDFLAGS)
rotz_cloud_LDADD = librotz.la
rotz_cloud_LDADD += libraux.la
BUILT_SOURCES += rotz-cloud-clo.c rotz-cloud-clo.h

bin_PROGRAMS += rotz-search
rotz_search_SOURCES = rotz-search.c
rotz_search_SOURCES += rotz-cmd-api.h
rotz_search_SOURCES += rotz-search-clo.h rotz-search-clo.ggo
rotz_search_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_search_LDFLAGS = $(AM_LDFLAGS)
rotz_search_LDADD = librotz.la
rotz_search_LDADD += libraux.la
BUILT_SOURCES += rotz-search-clo.c rotz-search-clo.h

bin_PROGRAMS += rotz-export
rotz_export_SOURCES = rotz-export.c
rotz_export_SOURCES += rotz-cmd-api.h
rotz_export_SOURCES += rotz-export-clo.h rotz-export-clo.ggo
rotz_export_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_export_LDFLAGS = $(AM_LDFLAGS)
rotz_export_LDADD = librotz.la
rotz_export_LDADD += libraux.la
BUILT_SOURCES += rotz-export-clo.c rotz-export-clo.h

bin_PROGRAMS += rotz-fsck
rotz_fsck_SOURCES = rotz-fsck.c
rotz_fsck_SOURCES += rotz-cmd-api.h
rotz_fsck_SOURCES += rotz-fsck-clo.h rotz-fsck-clo.ggo
rotz_fsck_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_fsck_CPPFLAGS += $(tokyocabinet_CFLAGS)
rotz_fsck_LDFLAGS = $(AM_LDFLAGS)
rotz_fsck_LDFLAGS += $(tokyocabinet_LIBS)
rotz_fsck_LDADD = librotz.la
rotz_fsck_LDADD += libraux.la
BUILT_SOURCES += rotz-fsck-clo.c rotz-fsck-clo.h

if USE_LMDB
noinst_PROGRAMS += rotz-dump
rotz_dump_SOURCES = rotz-dump.c
rotz_dump_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_dump_CPPFLAGS += $(lmdb_CFLAGS)
rotz_dump_LDFLAGS = $(AM_LDFLAGS)
rotz_dump_LDFLAGS += $(lmdb_LIBS)
endif  USE_LMDB


version.c: version.c.in $(top_builddir)/version.mk
	sed -e 's,[@]VERSION[@],$(VERSION),g' <$(srcdir)/$@.in >$@

## gperf rule
%.c: %.gperf
	$(AM_V_GEN) $(GPERF) -L ANSI-C $< --output-file $@

## gengetopt rule with sed hack to avoid using VERSION and config.h
## depend on Makefile.am to survive a git pull
%.c %.h: %.ggo Makefile.am
	$(AM_V_GEN) gengetopt -l -i $< -F $* \
	&& sed \
	  -e 's,config.h,version.h,g' \
	  -e 's,\(.*\)_PARSER_VERSION VERSION,\1_PARSER_VERSION $(PACKAGE)_version_string,g' \
	  $*.h >$*.h-t \
	&& mv $*.h-t $*.h

## for dist or dist check we need binaries, so help2man can work
dist-hook: $(bin_PROGRAMS)


## Help the developers get nice post-processed source files

## Create preprocessor output (debugging purposes only)
.c.i:
	$(COMPILE) -E -o $@ $<

## Create assembler output (debugging purposes only)
.c.s:
	$(COMPILE) -S -c $(AM_CFLAGS) $<

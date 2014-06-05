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
SUFFIXES =

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
rotz_add_SOURCES += rotz-add.yuck
rotz_add_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_add_LDFLAGS = $(AM_LDFLAGS)
rotz_add_LDADD = librotz.la
rotz_add_LDADD += libraux.la
BUILT_SOURCES += rotz-add.yucc

bin_PROGRAMS += rotz-del
rotz_del_SOURCES = rotz-del.c
rotz_del_SOURCES += rotz-cmd-api.h
rotz_del_SOURCES += rotz-del.yuck
rotz_del_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_del_LDFLAGS = $(AM_LDFLAGS)
rotz_del_LDADD = librotz.la
rotz_del_LDADD += libraux.la
BUILT_SOURCES += rotz-del.yucc

bin_PROGRAMS += rotz-show
rotz_show_SOURCES = rotz-show.c
rotz_show_SOURCES += rotz-cmd-api.h
rotz_show_SOURCES += rotz-show.yuck
rotz_show_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_show_LDFLAGS = $(AM_LDFLAGS)
rotz_show_LDADD = librotz.la
rotz_show_LDADD += libraux.la
BUILT_SOURCES += rotz-show.yucc

bin_PROGRAMS += rotz-alias
rotz_alias_SOURCES = rotz-alias.c
rotz_alias_SOURCES += rotz-cmd-api.h
rotz_alias_SOURCES += rotz-alias.yuck
rotz_alias_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_alias_LDFLAGS = $(AM_LDFLAGS)
rotz_alias_LDADD = librotz.la
rotz_alias_LDADD += libraux.la
BUILT_SOURCES += rotz-alias.yucc

bin_PROGRAMS += rotz-combine
rotz_combine_SOURCES = rotz-combine.c
rotz_combine_SOURCES += rotz-cmd-api.h
rotz_combine_SOURCES += rotz-combine.yuck
rotz_combine_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_combine_LDFLAGS = $(AM_LDFLAGS)
rotz_combine_LDADD = librotz.la
rotz_combine_LDADD += libraux.la
BUILT_SOURCES += rotz-combine.yucc

bin_PROGRAMS += rotz-rename
rotz_rename_SOURCES = rotz-rename.c
rotz_rename_SOURCES += rotz-cmd-api.h
rotz_rename_SOURCES += rotz-rename.yuck
rotz_rename_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_rename_LDFLAGS = $(AM_LDFLAGS)
rotz_rename_LDADD = librotz.la
rotz_rename_LDADD += libraux.la
BUILT_SOURCES += rotz-rename.yucc

bin_PROGRAMS += rotz-cloud
rotz_cloud_SOURCES = rotz-cloud.c
rotz_cloud_SOURCES += rotz-cmd-api.h
rotz_cloud_SOURCES += rotz-cloud.yuck
rotz_cloud_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_cloud_LDFLAGS = $(AM_LDFLAGS)
rotz_cloud_LDADD = librotz.la
rotz_cloud_LDADD += libraux.la
BUILT_SOURCES += rotz-cloud.yucc

bin_PROGRAMS += rotz-search
rotz_search_SOURCES = rotz-search.c
rotz_search_SOURCES += rotz-cmd-api.h
rotz_search_SOURCES += rotz-search.yuck
rotz_search_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_search_LDFLAGS = $(AM_LDFLAGS)
rotz_search_LDADD = librotz.la
rotz_search_LDADD += libraux.la
BUILT_SOURCES += rotz-search.yucc

bin_PROGRAMS += rotz-export
rotz_export_SOURCES = rotz-export.c
rotz_export_SOURCES += rotz-cmd-api.h
rotz_export_SOURCES += rotz-export.yuck
rotz_export_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_export_LDFLAGS = $(AM_LDFLAGS)
rotz_export_LDADD = librotz.la
rotz_export_LDADD += libraux.la
BUILT_SOURCES += rotz-export.yucc

bin_PROGRAMS += rotz-fsck
rotz_fsck_SOURCES = rotz-fsck.c
rotz_fsck_SOURCES += rotz-cmd-api.h
rotz_fsck_SOURCES += rotz-fsck.yuck
rotz_fsck_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_fsck_CPPFLAGS += $(tokyocabinet_CFLAGS)
rotz_fsck_LDFLAGS = $(AM_LDFLAGS)
rotz_fsck_LDFLAGS += $(tokyocabinet_LIBS)
rotz_fsck_LDADD = librotz.la
rotz_fsck_LDADD += libraux.la
BUILT_SOURCES += rotz-fsck.yucc

bin_PROGRAMS += rotz-grep
rotz_grep_SOURCES = rotz-grep.c
rotz_grep_SOURCES += rotz-cmd-api.h
rotz_grep_SOURCES += rotz-grep.yuck
rotz_grep_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_grep_LDFLAGS = $(AM_LDFLAGS)
rotz_grep_LDADD = librotz.la
rotz_grep_LDADD += libraux.la
BUILT_SOURCES += rotz-grep.yucc

if USE_LMDB
noinst_PROGRAMS += rotz-dump
rotz_dump_SOURCES = rotz-dump.c
rotz_dump_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_dump_CPPFLAGS += $(lmdb_CFLAGS)
rotz_dump_LDFLAGS = $(AM_LDFLAGS)
rotz_dump_LDFLAGS += $(lmdb_LIBS)
endif  USE_LMDB


## version rules
version.c: version.c.in $(top_builddir)/.version
	$(AM_V_GEN) PATH="$(top_builddir)/build-aux:$${PATH}" \
		yuck$(EXEEXT) scmver --ignore-noscm --force -o $@ \
			--use-reference --reference $(top_builddir)/.version $<

## gperf rule
SUFFIXES += .gperf
.gperf.c:
	$(AM_V_GEN) $(GPERF) -L ANSI-C $< --output-file $@

## yuck rule
SUFFIXES += .yuck
SUFFIXES += .yucc
.yuck.yucc:
	$(AM_V_GEN) PATH=$(top_builddir)/build-aux:"$${PATH}" \
		yuck$(EXEEXT) gen -o $@ $<

## for dist or dist check we need binaries, so help2man can work
dist-hook: $(bin_PROGRAMS)


## Help the developers get nice post-processed source files

## Create preprocessor output (debugging purposes only)
.c.i:
	$(COMPILE) -E -o $@ $<

## Create assembler output (debugging purposes only)
.c.s:
	$(COMPILE) -S -c $(AM_CFLAGS) $<

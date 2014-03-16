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

if USE_LMDB
noinst_PROGRAMS += rotz-dump
rotz_dump_SOURCES = rotz-dump.c
rotz_dump_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_dump_CPPFLAGS += $(lmdb_CFLAGS)
rotz_dump_LDFLAGS = $(AM_LDFLAGS)
rotz_dump_LDFLAGS += $(lmdb_LIBS)
endif  USE_LMDB

## the big rotz umbrella
bin_PROGRAMS += rotz
rotz_SOURCES = rotz-umb.c rotz-umb.h rotz-umb.yuck
rotz_SOURCES += rotz-cmd-api.h
rotz_SOURCES += rotz-add.c
rotz_SOURCES += rotz-alias.c
rotz_SOURCES += rotz-cloud.c
rotz_SOURCES += rotz-combine.c
rotz_SOURCES += rotz-del.c
rotz_SOURCES += rotz-export.c
rotz_SOURCES += rotz-fsck.c
rotz_SOURCES += rotz-grep.c
rotz_SOURCES += rotz-rename.c
rotz_SOURCES += rotz-search.c
rotz_SOURCES += rotz-show.c
rotz_CPPFLAGS = $(AM_CPPFLAGS) -DSTANDALONE
rotz_CPPFLAGS += $(tokyocabinet_CFLAGS)
rotz_LDFLAGS = $(AM_LDFLAGS)
rotz_LDFLAGS += -static $(tokyocabinet_LIBS)
rotz_LDADD = librotz.la libraux.la
BUILT_SOURCES += rotz-umb.yucc
BUILT_SOURCES += rotz-umb.yuch

version.c: version.c.in $(top_builddir)/version.mk
	sed -e 's,[@]VERSION[@],$(VERSION),g' <$(srcdir)/$@.in >$@

## gperf rule
SUFFIXES += .gperf
.gperf.c:
	$(AM_V_GEN) $(GPERF) -L ANSI-C "$<" --output-file "$@"

## yuck rule
SUFFIXES += .yuck
SUFFIXES += .yucc
.yuck.yucc:
	$(AM_V_GEN) PATH=$(top_builddir)/build-aux:"$${PATH}" \
		yuck$(EXEEXT) gen -H "$*.yuch" -o "$@" "$<"

SUFFIXES += .yuch
.yuck.yuch:
	$(AM_V_GEN) PATH=$(top_builddir)/build-aux:"$${PATH}" \
		yuck$(EXEEXT) gen -H "$@" -o /dev/null "$<"

## for dist or dist check we need binaries, so help2man can work
dist-hook: $(bin_PROGRAMS)


## Help the developers get nice post-processed source files

## Create preprocessor output (debugging purposes only)
.c.i:
	$(COMPILE) -E -o $@ $<

## Create assembler output (debugging purposes only)
.c.s:
	$(COMPILE) -S -c $(AM_CFLAGS) $<

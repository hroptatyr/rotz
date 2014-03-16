# Help the Developers and yourself. Just use the C locale and settings
# for the compilation. They can still be overriden by make LANG=<whatever>
# but that is general a not very good idea
include $(top_builddir)/version.mk

LANG = C
LC_ALL = C

AM_CFLAGS = $(EXTRA_CFLAGS)
AM_CPPFLAGS = -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -D_BSD_SOURCE
AM_CPPFLAGS += -DTEST

EXTRA_DIST = $(BUILT_SOURCES) $(TESTS)
TESTS =
TEST_EXTENSIONS =
BUILT_SOURCES =
dt_tests =
bin_tests =
batch_tests =
check_PROGRAMS =
CLEANFILES = $(check_PROGRAMS)
SUFFIXES =

TEST_EXTENSIONS += .tst
TST_LOG_COMPILER = $(builddir)/clitoris
AM_TST_LOG_FLAGS = --builddir $(top_builddir)/src

## the actual tests here
TESTS += add_01.tst
TESTS += add_02.tst
TESTS += add_03.tst
TESTS += del_01.tst
TESTS += del_02.tst
TESTS += del_03.tst
TESTS += del_04.tst

TESTS += show_01.tst
TESTS += show_02.tst
TESTS += show_03.tst
TESTS += show_04.tst
TESTS += show_05.tst
TESTS += show_06.tst
TESTS += show_07.tst
TESTS += show_08.tst

## tests will generate a rotz.tcb file
CLEANFILES += rotz.tcb

## our friendly helpers
check_PROGRAMS += clitoris
clitoris_SOURCES = clitoris.c clitoris.yuck
clitoris_CPPFLAGS = $(AM_CPPFLAGS)
clitoris_CPPFLAGS += -D_GNU_SOURCE
clitoris_CPPFLAGS += -DHAVE_PTY_H
clitoris_LDFLAGS = $(AM_LDFLAGS) -static
clitoris_LDFLAGS += -lutil
BUILT_SOURCES += clitoris.yucc


## yuck rule
SUFFIXES += .yuck
SUFFIXES += .yucc
.yuck.yucc:
	$(AM_V_GEN) PATH=$(top_builddir)/build-aux:"$${PATH}" \
		yuck$(EXEEXT) gen -o $@ $<

clean-local:
	-rm -rf *.tmpd

## Makefile.am ends here

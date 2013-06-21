# Help the Developers and yourself. Just use the C locale and settings
# for the compilation. They can still be overriden by make LANG=<whatever>
# but that is general a not very good idea
include $(top_builddir)/version.mk

LANG = C
LC_ALL = C

AM_CFLAGS = $(EXTRA_CFLAGS)
AM_CPPFLAGS = -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=600 -D_BSD_SOURCE
AM_CPPFLAGS += -DTEST

EXTRA_DIST =
EXTRA_DIST += $(BUILT_SOURCES)
TESTS =
TESTS_ENVIRONMENT =
TEST_EXTENSIONS =
BUILT_SOURCES =
dt_tests =
bin_tests =
batch_tests =
check_PROGRAMS =
CLEANFILES = $(check_PROGRAMS)

TEST_EXTENSIONS += .tst
check_PROGRAMS += rtz-test
TST_LOG_COMPILER = $(builddir)/rtz-test
AM_TST_LOG_FLAGS = --builddir $(top_builddir)/src
EXTRA_DIST += $(srcdir)/rtz-test.sh

## the actual tests here


## our friendly helpers
check_PROGRAMS += rtz-test
BUILT_SOURCES += rtz-test-clo.c rtz-test-clo.h
EXTRA_DIST += rtz-test.sh

## ggo rule
%.c %.h: %.ggo
	$(AM_V_GEN) gengetopt -F $* -i $<
	@sed -e 's,config.h,version.h,g' \
	     -e 's,_PARSER_VERSION VERSION,_PARSER_VERSION "$(VERSION)",g' \
	     $*.h > $*.h-t \
	&& mv $*.h-t $*.h

clean-local:
	-rm -rf *.tmpd

## Makefile.am ends here

/*** rtz-test.c -- just to work around getopt issues in rtz-test.sh */

#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>


static void
__attribute__((format(printf, 2, 3)))
error(int eno, const char *fmt, ...)
{
	va_list vap;
	va_start(vap, fmt);
	vfprintf(stderr, fmt, vap);
	va_end(vap);
	if (eno || errno) {
		fputc(':', stderr);
		fputc(' ', stderr);
		fputs(strerror(eno ?: errno), stderr);
	}
	fputc('\n', stderr);
	return;
}

static void*
mmap_fd(int fd, size_t fz)
{
	void *p;

	if ((p = mmap(NULL, fz, PROT_READ, MAP_PRIVATE, 0, fd)) == MAP_FAILED) {
		return NULL;
	}
	return p;
}

static int
munmap_fd(void *fp, size_t fz)
{
	return munmap(fp, fz);
}


static int
test(const char *testfile)
{
	int fd;
	int rc = 99;
	struct stat st;
	void *tfmp;

	if ((fd = open(testfile, O_RDONLY)) < 0) {
		error(0, "Error: cannot open file `%s'", testfile);
		goto out;
	} else if (fstat(fd, &st) < 0) {
		error(0, "Error: cannot stat file `%s'", testfile);
		goto out;
	} else if ((tfmp = mmap_fd(fd, st.st_size)) == NULL) {
		error(0, "Error: cannot map file `%s'", testfile);
		goto out;
	}


	munmap_fd(tfmp, st.st_size);
out:
	return rc;
}


#if defined __INTEL_COMPILER
# pragma warning (disable:593)
# pragma warning (disable:181)
#endif	/* __INTEL_COMPILER */
#include "rtz-test-clo.h"
#include "rtz-test-clo.c"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
# pragma warning (default:181)
#endif	/* __INTEL_COMPILER */

int
main(int argc, char *argv[])
{
	struct gengetopt_args_info argi[1];
	int rc = 99;

	if (cmdline_parser(argc, argv, argi)) {
		goto out;
	} else if (argi->inputs_num != 1) {
		print_help_common();
		goto out;
	}

	if (argi->builddir_given) {
		setenv("builddir", argi->builddir_arg, 1);
	}
	if (argi->srcdir_given) {
		setenv("srcdir", argi->srcdir_arg, 1);
	}
	if (argi->hash_given) {
		setenv("hash", argi->hash_arg, 1);
	}
	if (argi->husk_given) {
		setenv("husk", argi->husk_arg, 1);
	}

	/* just to be clear about this */
#if defined WORDS_BIGENDIAN
	setenv("endian", "big", 1);
#else  /* !WORDS_BIGENDIAN */
	setenv("endian", "little", 1);
#endif	/* WORDS_BIGENDIAN */

	rc = test(argi->inputs[0]);

out:
	cmdline_parser_free(argi);
	/* never succeed */
	return rc;
}

/* rtz-test.c ends here */

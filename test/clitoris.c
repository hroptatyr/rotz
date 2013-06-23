/*** clitoris.c -- command line testing oris
 *
 * Copyright (C) 2013 Sebastian Freundt
 *
 * Author:  Sebastian Freundt <freundt@ga-group.nl>
 *
 * This file is part of clitoris.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the author nor the names of any contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ***/
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

typedef struct clitf_s clitf_t;

struct clitf_s {
	size_t z;
	void *d;
};


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


static clitf_t
mmap_fd(int fd, size_t fz)
{
	void *p;

	if ((p = mmap(NULL, fz, PROT_READ, MAP_PRIVATE, 0, fd)) == MAP_FAILED) {
		return (clitf_t){.z = 0U, .d = NULL};
	}
	return (clitf_t){.z = fz, .d = p};
}

static int
munmap_fd(clitf_t map)
{
	return munmap(map.d, map.z);
}


static int
test(const char *testfile)
{
	int fd;
	int rc = 99;
	struct stat st;
	clitf_t tf;

	if ((fd = open(testfile, O_RDONLY)) < 0) {
		error(0, "Error: cannot open file `%s'", testfile);
		goto out;
	} else if (fstat(fd, &st) < 0) {
		error(0, "Error: cannot stat file `%s'", testfile);
		goto out;
	} else if ((tf = mmap_fd(fd, st.st_size)).d == NULL) {
		error(0, "Error: cannot map file `%s'", testfile);
		goto out;
	}

	const char *beef = tf.d;
	if (*beef == '$' || beef = memmem(beef, "\n$", 2)) {
		puts("beef is there:");
		puts(beef);
	}

	munmap_fd(tfmp, st.st_size);
out:
	return rc;
}


#if defined __INTEL_COMPILER
# pragma warning (disable:593)
# pragma warning (disable:181)
#endif	/* __INTEL_COMPILER */
#include "clitoris.h"
#include "clitoris.x"
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

/* clitoris.c ends here */

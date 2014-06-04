/*** rotz-add.c -- rotz tag adder
 *
 * Copyright (C) 2013 Sebastian Freundt
 *
 * Author:  Sebastian Freundt <freundt@ga-group.nl>
 *
 * This file is part of rotz.
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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#include "rotz.h"
#include "rotz-cmd-api.h"
#include "nifty.h"


static int verbosep;
#define _		rotz_massage_name

static void
add_tag(rotz_t ctx, rtz_vtx_t tid, const char *sym)
{
	const char *symspc_sym;
	rtz_vtx_t sid;

	if (UNLIKELY((symspc_sym = rotz_sym(sym)) == NULL)) {
		return;
	} else if (UNLIKELY((sid = rotz_add_vertex(ctx, symspc_sym)) == 0U)) {
		return;
	}
	rotz_add_edge(ctx, tid, sid);
	rotz_add_edge(ctx, sid, tid);

	if (UNLIKELY(verbosep)) {
		fputc('+', stdout);
		fputs(_(rotz_get_name(ctx, tid)), stdout);
		fputc('\t', stdout);
		fputs(_(rotz_get_name(ctx, sid)), stdout);
		fputc('\n', stdout);
	}
	return;
}

static void
add_tagsym(rotz_t ctx, const char *tag, const char *sym)
{
	rtz_vtx_t tid;

	tag = rotz_tag(tag);
	if (UNLIKELY((tid = rotz_add_vertex(ctx, tag)) == 0U)) {
		return;
	}
	add_tag(ctx, tid, sym);
	return;
}


#if defined STANDALONE
#include "rotz-add.yucc"

int
main(int argc, char *argv[])
{
	yuck_t argi[1];
	rotz_t ctx;
	const char *db = RTZ_DFLT_DB;
	const char *tag;
	rtz_vtx_t tid;
	int res = 0;

	if (yuck_parse(argi, argc, argv)) {
		res = 1;
		goto out;
	}

	if (argi->database_arg) {
		db = argi->database_arg;
	}
	if (argi->verbose_flag) {
		verbosep = 1;
	}

	if (UNLIKELY((ctx = make_rotz(db, O_CREAT | O_RDWR)) == NULL)) {
		fputs("Error opening rotz datastore\n", stderr);
		res = 1;
		goto out;
	}
	if (argi->nargs == 0U && !isatty(STDIN_FILENO)) {
		/* tag \t sym mode, both from stdin */
		char *line = NULL;
		size_t llen = 0U;
		ssize_t nrd;

		while ((nrd = getline(&line, &llen, stdin)) > 0) {
			const char *sym;

			line[nrd - 1] = '\0';
			tag = line;
			if (UNLIKELY((sym = strchr(line, '\t')) == NULL)) {
				continue;
			}
			/* \t -> \0 */
			line[sym++ - line] = '\0';
			add_tagsym(ctx, tag, sym);
		}
		free(line);
		goto fini;
	}
	/* ... otherwise associate with TAG somehow */
	tag = rotz_tag(argi->args[0U]);
	if (UNLIKELY((tid = rotz_add_vertex(ctx, tag)) == 0U)) {
		goto fini;
	}
	for (size_t i = 1U; i < argi->nargs; i++) {
		add_tag(ctx, tid, argi->args[i]);
	}
	if (argi->nargs == 1U && !isatty(STDIN_FILENO)) {
		/* add tags from stdin */
		char *line = NULL;
		size_t llen = 0U;
		ssize_t nrd;

		while ((nrd = getline(&line, &llen, stdin)) > 0) {
			line[nrd - 1] = '\0';
			add_tag(ctx, tid, line);
		}
		free(line);
	}

fini:
	/* big resource freeing */
	free_rotz(ctx);
out:
	yuck_free(argi);
	return res;
}
#endif	/* STANDALONE */

/* rotz-add.c ends here */

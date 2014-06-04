/*** rotz-alias.c -- rotz tag aliaser
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


static void
alias_tag(rotz_t ctx, rtz_vtx_t tid, const char *alias)
{
	const char *rtag;

	if (UNLIKELY((rtag = rotz_tag(alias)) == NULL)) {
		return;
	}
	(void)rotz_add_alias(ctx, tid, rtag);
	return;
}

static void
aliases(rotz_t ctx, rtz_vtx_t tid, char sep)
{
	rtz_buf_t al;

	al = rotz_get_aliases(ctx, tid);
	for (const char *p = al.d, *const ep = al.d + al.z; p < ep;
	     p += strlen(p) + 1/*for \nul*/) {
		if (LIKELY(p > al.d)) {
			fputc(sep, stdout);
		}
		fputs(rotz_massage_name(p), stdout);
	}
	rotz_free_r(al);
	fputc('\n', stdout);
	return;
}

static int
iter_cb(rtz_vtx_t vid, const char *vtx, void *clo)
{
	if (memcmp(vtx, RTZ_SYMSPC, sizeof(RTZ_SYMSPC) - 1) == 0) {
		/* that's a symbol, symbol can't have aliases, bugger off */
		return 0;
	}
	aliases(clo, vid, '\t');
	return 0;
}


#if defined STANDALONE
#include "rotz-alias.yucc"

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
	if (UNLIKELY((ctx = make_rotz(db, O_CREAT | O_RDWR)) == NULL)) {
		fputs("Error opening rotz datastore\n", stderr);
		res = 1;
		goto out;
	}

	if (argi->nargs > 0U) {
		tag = rotz_tag(argi->args[0]);
		if (UNLIKELY((tid = rotz_get_vertex(ctx, tag)) == 0U)) {
			goto fini;
		}
		if (argi->nargs > 1U && !argi->delete_flag) {
			for (unsigned int i = 1U; i < argi->nargs; i++) {
				alias_tag(ctx, tid, argi->args[i]);
			}
		} else if (argi->delete_flag) {
			/* del tag */
			rotz_rem_alias(ctx, tag);
			for (unsigned int i = 1U; i < argi->nargs; i++) {
				rotz_rem_alias(ctx, rotz_tag(argi->args[i]));
			}
		} else if (isatty(STDIN_FILENO)) {
			/* report aliases for TAG */
			aliases(ctx, tid, '\n');
		} else {
			/* alias tags from stdin */
			char *line = NULL;
			size_t llen = 0U;
			ssize_t nrd;

			while ((nrd = getline(&line, &llen, stdin)) > 0) {
				line[nrd - 1] = '\0';
				alias_tag(ctx, tid, line);
			}
			free(line);
		}
	} else {
		/* show all aliases mode */
		rotz_vtx_iter(ctx, iter_cb, ctx);
	}

fini:
	/* big resource freeing */
	free_rotz(ctx);
out:
	yuck_free(argi);
	return res;
}
#endif	/* STANDALONE */

/* rotz-alias.c ends here */

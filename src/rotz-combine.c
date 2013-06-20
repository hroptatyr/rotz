/*** rotz-combine.c -- rotz tag combineer
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
combine_into(rotz_t ctx, rtz_vtx_t into_tid, const char *tag)
{
	rtz_vtx_t tid;
	rtz_vtxlst_t el;

	if (UNLIKELY((tag = rotz_tag(tag)) == NULL)) {
		return;
	} else if (UNLIKELY(!(tid = rotz_get_vertex(ctx, tag)))) {
		return;
	} else if (UNLIKELY((el = rotz_get_edges(ctx, tid)).d == NULL)) {
		return;
	}
	/* otherwise combine TID into MASTER_TID by
	 * traversing the edges of TID and adding them to MASTER_TID's */
	for (rtz_vtx_t *p = el.d, *const ep = el.d + el.z; p < ep; p++) {
		rotz_add_edge(ctx, into_tid, *p);
		rotz_rem_edge(ctx, *p, tid);
		rotz_add_edge(ctx, *p, into_tid);
	}
	rotz_free_vtxlst(el);
	/* delete that vertex' edges */
	rotz_rem_edges(ctx, tid);
	/* now delete the whole vertex */
	rotz_rem_vertex(ctx, tag);
	return;
}

static void
combine_tag(rotz_t ctx, const char *tag)
{
	static rtz_vtx_t master_tid;

	if (UNLIKELY(!master_tid)) {
		/* singleton */
		master_tid = rotz_get_vertex(ctx, rotz_tag(tag));
		return;
	}
	combine_into(ctx, master_tid, tag);
	/* and add it as an alias for master_tid */
	rotz_add_alias(ctx, master_tid, rotz_tag(tag));
	return;
}


#if defined STANDALONE
#if defined __INTEL_COMPILER
# pragma warning (disable:593)
#endif	/* __INTEL_COMPILER */
#include "rotz-combine-clo.h"
#include "rotz-combine-clo.c"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
#endif	/* __INTEL_COMPILER */

int
main(int argc, char *argv[])
{
	struct rotz_args_info argi[1];
	rotz_t ctx;
	const char *db = RTZ_DFLT_DB;
	int res = 0;

	if (rotz_parser(argc, argv, argi)) {
		res = 1;
		goto out;
	}

	if (argi->database_given) {
		db = argi->database_arg;
	}
	if (UNLIKELY((ctx = make_rotz(db, O_CREAT | O_RDWR)) == NULL)) {
		fputs("Error opening rotz datastore\n", stderr);
		res = 1;
		goto out;
	}

	if (argi->into_given) {
		const char *tag = rotz_tag(argi->into_arg);
		rtz_vtx_t into_tid;

		if (UNLIKELY((into_tid = rotz_get_vertex(ctx, tag)) == 0U)) {
			fprintf(stderr, "\
Error moving into tag %s, no such tag\n", tag);
			goto fina;
		}
		for (unsigned int i = 0; i < argi->inputs_num; i++) {
			combine_into(ctx, into_tid, argi->inputs[i]);
		}
	} else if (argi->inputs_num) {
		/* combine everything into the first argument */
		for (unsigned int i = 0; i < argi->inputs_num; i++) {
			combine_tag(ctx, argi->inputs[i]);
		}
	} else if (!isatty(STDIN_FILENO)) {
		/* combine tags from stdin */
		char *line = NULL;
		size_t llen = 0U;
		ssize_t nrd;

		while ((nrd = getline(&line, &llen, stdin)) > 0) {
			line[nrd - 1] = '\0';
			combine_tag(ctx, line);
		}
		free(line);
	}

fina:
	/* big resource freeing */
	free_rotz(ctx);
out:
	rotz_parser_free(argi);
	return res;
}
#endif	/* STANDALONE */

/* rotz-combine.c ends here */

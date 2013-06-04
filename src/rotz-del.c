/*** rotz-del.c -- rotz tag del'er
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
del_tag(rotz_t ctx, rtz_vtx_t tid, const char *sym)
{
	const char *symspc_sym;
	rtz_vtx_t sid;

	if (UNLIKELY((symspc_sym = rotz_sym(sym)) == NULL)) {
		return;
	} else if (LIKELY((sid = rotz_get_vertex(ctx, symspc_sym)) == 0U)) {
		return;
	}
	rotz_rem_edge(ctx, tid, sid);
	rotz_rem_edge(ctx, sid, tid);
	return;
}

static void
del_syms(rotz_t ctx, const char *tag)
{
	rtz_vtxlst_t el;
	rtz_vtx_t tid;

	/* massage tag */
	tag = rotz_tag(tag);
	if (UNLIKELY((tid = rotz_get_vertex(ctx, tag)) == 0U)) {
		/* not sure what to delete */
		return;
	} else if (UNLIKELY((el = rotz_get_edges(ctx, tid)).d == NULL)) {
		/* nothing to delete */
		return;
	}
	/* get rid of all the edges */
	rotz_rem_edges(ctx, tid);
	/* now go through the list EL and delete TID */
	for (size_t i = 0; i < el.z; i++) {
		rotz_rem_edge(ctx, el.d[i], tid);
	}
	/* finalise the list */
	rotz_free_vtxlst(el);
	/* finally delete the vertex */
	rotz_rem_vertex(ctx, tag);
	return;
}


#if defined STANDALONE
#if defined __INTEL_COMPILER
# pragma warning (disable:593)
#endif	/* __INTEL_COMPILER */
#include "rotz-del-clo.h"
#include "rotz-del-clo.c"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
#endif	/* __INTEL_COMPILER */

int
main(int argc, char *argv[])
{
	struct rotz_args_info argi[1];
	rotz_t ctx;
	const char *db = "rotz.tcb";
	const char *tag;
	rtz_vtx_t tid;
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
	if (argi->inputs_num > 1) {
		tag = rotz_tag(argi->inputs[0]);
		if (UNLIKELY((tid = rotz_get_vertex(ctx, tag)) == 0U)) {
			goto fini;
		}
		for (unsigned int i = 1; i < argi->inputs_num; i++) {
			del_tag(ctx, tid, argi->inputs[i]);
		}
	} else if (argi->inputs_num == 1 && isatty(STDIN_FILENO)) {
		/* del all syms assoc'd with TAG */
		del_syms(ctx, argi->inputs[0]);
	} else if (argi->inputs_num == 1) {
		/* del tag/sym pairs from stdin */
		char *line = NULL;
		size_t llen = 0U;
		ssize_t nrd;

		while ((nrd = getline(&line, &llen, stdin)) > 0) {
			line[nrd - 1] = '\0';
			del_tag(ctx, tid, line);
		}
		free(line);
	} else if (!isatty(STDIN_FILENO)) {
		/* del tags from stdin */
		char *line = NULL;
		size_t llen = 0U;
		ssize_t nrd;

		while ((nrd = getline(&line, &llen, stdin)) > 0) {
			line[nrd - 1] = '\0';
			del_syms(ctx, line);
		}
		free(line);
	}

fini:
	/* big resource freeing */
	free_rotz(ctx);
out:
	rotz_parser_free(argi);
	return res;
}
#endif	/* STANDALONE */

/* rotz-del.c ends here */

/*** rotz-cloud.c -- rotz tag cloud'er
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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "rotz.h"
#include "rotz-cmd-api.h"
#include "nifty.h"

static rotz_t ctx;

struct iter_clo_s {
	struct {
		size_t z;
		const char *d;
	} pre;

	size_t ntop;
	struct {
		char *sym;
		size_t cnt;
	} *top;
};


static void
iter_cb(rtz_vtx_t vid, const char *vtx, void *clo)
{
	const struct iter_clo_s *cp = clo;
	rtz_vtxlst_t el;

	if (memcmp(vtx, RTZ_SYMSPC, sizeof(RTZ_SYMSPC) - 1) == 0) {
		/* that's a symbol, vtx would be a tag then */
		return;
	} else if (memcmp(vtx, RTZ_TAGSPC, sizeof(RTZ_TAGSPC) - 1) == 0) {
		vtx += RTZ_PRE_Z;
	}

	if (cp->pre.d && memcmp(vtx, cp->pre.d, cp->pre.z)) {
		/* not matching prefix */
		return;
	}

	el = rotz_get_edges(ctx, vid);
	if (!cp->ntop) {
		fputs(vtx, stdout);
		fputc('\t', stdout);
		fprintf(stdout, "%zu\n", el.z);
	} else if (el.z >= cp->top[0].cnt) {
		size_t pos;

		for (pos = 1; pos < cp->ntop && el.z >= cp->top[pos].cnt; pos++);

		/* pos - 1 is the position to insert to */
		if (cp->top[0].sym != NULL) {
			free(cp->top[0].sym);
		}
		memmove(cp->top, cp->top + 1, pos * sizeof(*cp->top));
		cp->top[pos].sym = strdup(vtx);
		cp->top[pos].cnt = el.z;
	}
	rotz_free_vtxlst(el);
	return;
}

static void
prnt_top(const struct iter_clo_s *cp)
{
	for (size_t i = cp->ntop; i-- > 0 && cp->top[i].sym != NULL;) {
		fputs(cp->top[i].sym, stdout);
		fputc('\t', stdout);
		fprintf(stdout, "%zu\n", cp->top[i].cnt);
		/* also free the whole shebang */
		free(cp->top[i].sym);
	}
	return;
}


#if defined STANDALONE
#if defined __INTEL_COMPILER
# pragma warning (disable:593)
#endif	/* __INTEL_COMPILER */
#include "rotz-cloud-clo.h"
#include "rotz-cloud-clo.c"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
#endif	/* __INTEL_COMPILER */

int
main(int argc, char *argv[])
{
	static struct iter_clo_s clo[1];
	struct rotz_args_info argi[1];
	const char *db = "rotz.tcb";
	int res = 0;

	if (rotz_parser(argc, argv, argi)) {
		res = 1;
		goto out;
	}

	if (argi->database_given) {
		db = argi->database_arg;
	}
	if (UNLIKELY((ctx = make_rotz(db)) == NULL)) {
		fputs("Error opening rotz datastore\n", stderr);
		res = 1;
		goto out;
	}

	/* cloud all tags mode, undocumented prefix feature */
	if (argi->inputs_num) {
		clo->pre.z = strlen(argi->inputs[0]);
		clo->pre.d = argi->inputs[0];
	}
	if (argi->top_given) {
		clo->ntop = argi->top_arg;
		clo->top = calloc(clo->ntop, sizeof(*clo->top));
	}
	rotz_vtx_iter(ctx, iter_cb, clo);
	if (argi->top_given) {
		prnt_top(clo);
	}

	/* big resource freeing */
	free_rotz(ctx);
out:
	rotz_parser_free(argi);
	return res;
}
#endif	/* STANDALONE */

/* rotz-cloud.c ends here */

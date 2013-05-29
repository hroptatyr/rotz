/*** rotz-show.c -- rotz tag show'er
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


static void
iter_cb(rtz_vtx_t UNUSED(vid), const char *vtx, void *UNUSED(clo))
{
	if (memcmp(vtx, RTZ_SYMSPC, sizeof(RTZ_SYMSPC) - 1) == 0) {
		/* that's a symbol, vtx would be a tag then */
		return;
	} else if (memcmp(vtx, RTZ_TAGSPC, sizeof(RTZ_TAGSPC) - 1) == 0) {
		vtx += RTZ_PRE_Z;
	}
	puts(vtx);
	return;
}

static void
prnt_vtxlst(rotz_t ctx, rtz_vtxlst_t el)
{
	for (size_t j = 0; j < el.z; j++) {
		const char *s = rotz_get_name(ctx, el.d[j]);

		if (UNLIKELY((s = rotz_massage_name(s)) == NULL)) {
			/* uh oh */
			continue;
		}
		puts(s);
	}
	return;
}

static void
show_tagsym(rotz_t ctx, rtz_vtx_t tsid)
{
/* show all syms associated with tag vertex TSID, or
 * all tags assoc'd with sym vertex TSID. */
	rtz_vtxlst_t vl;

	/* get all them edges and iterate */
	vl = rotz_get_edges(ctx, tsid);

	/* print it */
	prnt_vtxlst(ctx, vl);

	/* get ready for the next round */
	rotz_free_vtxlst(vl);
	return;
}


#if defined STANDALONE
#if defined __INTEL_COMPILER
# pragma warning (disable:593)
#endif	/* __INTEL_COMPILER */
#include "rotz-show-clo.h"
#include "rotz-show-clo.c"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
#endif	/* __INTEL_COMPILER */

int
main(int argc, char *argv[])
{
	struct rotz_args_info argi[1];
	rotz_t ctx;
	const char *db = "rotz.tcb";
	rtz_vtxlst_t vl = {0U};
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

	for (unsigned int i = 0; i < argi->inputs_num; i++) {
		const char *const input = argi->inputs[i];
		const char *tagsym;
		rtz_vtx_t tsid;

		if ((tagsym = rotz_tag(input),
		     tsid = rotz_get_vertex(ctx, tagsym))) {
			;
		} else if ((tagsym = rotz_sym(input),
			    tsid = rotz_get_vertex(ctx, tagsym))) {
			;
		} else {
			/* nothing to worry about */
			continue;
		}

		if (argi->union_given) {
			vl = rotz_union(ctx, vl, tsid);
		} else if (argi->intersection_given) {
			if (i > 0) {
				vl = rotz_intersection(ctx, vl, tsid);
			} else {
				vl = rotz_get_edges(ctx, tsid);
			}
		} else {
			show_tagsym(ctx, tsid);
		}
	}
	if (argi->union_given || argi->intersection_given) {
		prnt_vtxlst(ctx, vl);
	}
	if (argi->inputs_num == 0) {
		/* show all tags mode */
		rotz_vtx_iter(ctx, iter_cb, NULL);
	}

	/* big resource freeing */
	free_rotz(ctx);
out:
	rotz_parser_free(argi);
	return res;
}
#endif	/* STANDALONE */

/* rotz-show.c ends here */

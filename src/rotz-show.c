/*** rotz-show.c -- rotz tag show'er
 *
 * Copyright (C) 2013-2014 Sebastian Freundt
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

#include "rotz.h"
#include "rotz-cmd-api.h"
#include "rotz-umb.h"
#include "raux.h"
#include "nifty.h"


static int
iter_cb(rtz_vtx_t UNUSED(vid), const char *vtx, void *UNUSED(clo))
{
	if (memcmp(vtx, RTZ_SYMSPC, sizeof(RTZ_SYMSPC) - 1) == 0) {
		/* that's a symbol, vtx would be a tag then */
		return 0;
	} else if (memcmp(vtx, RTZ_TAGSPC, sizeof(RTZ_TAGSPC) - 1) == 0) {
		vtx += RTZ_PRE_Z;
	}
	puts(vtx);
	return 0;
}

static int
iter_syms_cb(rtz_vtx_t UNUSED(vid), const char *vtx, void *UNUSED(clo))
{
	if (memcmp(vtx, RTZ_SYMSPC, sizeof(RTZ_SYMSPC) - 1) != 0) {
		/* it's not a symbol, bugger off */
		return 0;
	}
	vtx += RTZ_PRE_Z;
	puts(vtx);
	return 0;
}

static void
prnt_vtxlst(rotz_t ctx, rtz_vtxlst_t el)
{
	for (size_t j = 0; j < el.z; j++) {
		const char *s = rotz_get_name(ctx, el.d[j]);
		puts(rotz_massage_name(s));
	}
	return;
}

static void
prnt_vtxlst_pair(rotz_t ctx, rtz_vtxlst_t el, const char *pair)
{
	for (size_t j = 0; j < el.z; j++) {
		const char *s = rotz_get_name(ctx, el.d[j]);

		fputs(pair, stdout);
		fputc('\t', stdout);
		fputs(rotz_massage_name(s), stdout);
		fputc('\n', stdout);
	}
	return;
}

static void
prnt_wtxlst(rotz_t ctx, rtz_wtxlst_t wl)
{
	/* quick service */

	for (size_t j = 0; j < wl.z; j++) {
		const char *s = rotz_get_name(ctx, wl.d[j]);
		fputs(rotz_massage_name(s), stdout);
		fputc('\t', stdout);
		fprintf(stdout, "%u\n", wl.w[j] + 1U);
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

static void
show_tagsym_pair(rotz_t ctx, rtz_vtx_t tsid, const char *pair)
{
/* show all syms associated with tag vertex TSID, or
 * all tags assoc'd with sym vertex TSID. */
	rtz_vtxlst_t vl;

	/* get all them edges and iterate */
	vl = rotz_get_edges(ctx, tsid);

	/* print it */
	prnt_vtxlst_pair(ctx, vl, pair);

	/* get ready for the next round */
	rotz_free_vtxlst(vl);
	return;
}


#if defined STANDALONE
static union {
	rtz_vtxlst_t vl;
	rtz_wtxlst_t wl;
} r = {0U};

static void
handle_one(rotz_t ctx, const struct yuck_cmd_show_s *argi, const char *input)
{
	static size_t i;
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
		return;
	}

	if (argi->union_flag) {
		r.vl = rotz_union(ctx, r.vl, tsid);
	} else if (argi->munion_flag) {
		r.wl = rotz_munion(ctx, r.wl, tsid);
	} else if (argi->intersection_flag) {
		if (i++ > 0) {
			r.vl = rotz_intersection(ctx, r.vl, tsid);
		} else {
			r.vl = rotz_get_edges(ctx, tsid);
		}
	} else if (argi->pairs_flag) {
		show_tagsym_pair(ctx, tsid, input);
	} else {
		show_tagsym(ctx, tsid);
	}
	return;
}

int
rotz_cmd_show(const struct yuck_cmd_show_s argi[static 1U])
{
	rotz_t ctx;

	if (UNLIKELY((ctx = make_rotz(db)) == NULL)) {
		fputs("Error opening rotz datastore\n", stderr);
		return 1;
	}

	for (size_t i = 0U; i < argi->nargs; i++) {
		const char *const input = argi->args[i];

		handle_one(ctx, argi, input);
	}
	if (argi->nargs == 0U && !isatty(STDIN_FILENO)) {
		/* read the guys from STDIN */
		char *line = NULL;
		size_t llen = 0U;
		ssize_t nrd;

		while ((nrd = getline(&line, &llen, stdin)) > 0) {
			line[nrd - 1] = '\0';
			handle_one(ctx, argi, line);
		}
		free(line);
	} else if (argi->nargs == 0U && argi->syms_flag) {
		/* show all syms mode */
		rotz_vtx_iter(ctx, iter_syms_cb, NULL);
		goto fina;
	} else if (argi->nargs == 0U) {
		/* show all tags mode */
		rotz_vtx_iter(ctx, iter_cb, NULL);
		goto fina;
	}
	if (argi->union_flag || argi->intersection_flag) {
		prnt_vtxlst(ctx, r.vl);
	} else if (argi->munion_flag) {
		/* quick service, sort r.wl, could be an option */
		sort_wtxlst(r.wl);
		prnt_wtxlst(ctx, r.wl);
	}

fina:
	/* big rcource freeing */
	free_rotz(ctx);
	return 0;
}
#endif	/* STANDALONE */

/* rotz-show.c ends here */

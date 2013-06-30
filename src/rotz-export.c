/*** rotz-export.c -- rotz graph exporter
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

static int clusterp;


static int
iter_csv_cb(rtz_vtx_t vid, rtz_const_vtxlst_t vl, void *clo)
{
	rtz_buf_t n = rotz_get_name_r(clo, vid);
	const char *vtx = n.d;

	if (memcmp(vtx, RTZ_SYMSPC, sizeof(RTZ_SYMSPC) - 1) == 0) {
		/* that's a symbol, vtx would be a tag then */
		return 0;
	} else if (memcmp(vtx, RTZ_TAGSPC, sizeof(RTZ_TAGSPC) - 1) == 0) {
		vtx += RTZ_PRE_Z;
	} else if (clusterp) {
		char *p = strchr(vtx, ':');
		*p = '\0';
	}

	for (size_t i = 0; i < vl.z; i++) {
		const char *vld = rotz_get_name(clo, vl.d[i]);
		const char *tgt = rotz_massage_name(vld);

		fputs(vtx, stdout);
		fputc('\t', stdout);
		fputs(tgt, stdout);
		fputc('\n', stdout);
	}
	rotz_free_r(n);
	return 0;
}

static int
iter_dot_cb(rtz_vtx_t vid, rtz_const_vtxlst_t vl, void *clo)
{
	rtz_buf_t n = rotz_get_name_r(clo, vid);
	const char *vtx = n.d;

	if (memcmp(vtx, RTZ_SYMSPC, sizeof(RTZ_SYMSPC) - 1) == 0) {
		/* that's a symbol, vtx would be a tag then */
		return 0;
	} else if (memcmp(vtx, RTZ_TAGSPC, sizeof(RTZ_TAGSPC) - 1) == 0) {
		vtx += RTZ_PRE_Z;
	} else if (clusterp) {
		char *p = strchr(vtx, ':');
		*p = '\0';
	}

	for (size_t i = 0; i < vl.z; i++) {
		const char *vld = rotz_get_name(clo, vl.d[i]);
		const char *tgt = rotz_massage_name(vld);

		printf("  \"%s\" -- \"%s\";\n", vtx, tgt);
	}
	rotz_free_r(n);
	return 0;
}

static int
iter_gmlv_cb(rtz_vtx_t vid, const char *vtx, void *UNUSED(clo))
{
	printf("\
  node [\n\
    id %u\n\
    label \"%s\"\n\
  ]\n", vid, rotz_massage_name(vtx));
	return 0;
}

static int
iter_gmle_cb(rtz_vtx_t sid, rtz_const_vtxlst_t vl, void *clo)
{
	const char *stx = rotz_get_name(clo, sid);

	if (memcmp(stx, RTZ_SYMSPC, sizeof(RTZ_SYMSPC) - 1) == 0) {
		return 0;
	}

	for (size_t i = 0; i < vl.z; i++) {
		rtz_vtx_t tid = vl.d[i];

		printf("\
  edge [\n\
    source %u\n\
    target %u\n\
  ]\n", sid, tid);
	}
	return 0;
}

static void
xprt_csv(rotz_t ctx)
{
	/* go through all edges */
	rotz_edg_iter(ctx, iter_csv_cb, ctx);
	return;
}

static void
xprt_dot(rotz_t ctx)
{
	puts("graph rotz {");

	/* go through all edges */
	rotz_edg_iter(ctx, iter_dot_cb, ctx);

	puts("}");
	return;
}

static void
xprt_gml(rotz_t ctx)
{
	puts("\
graph [\n\
  directed 0\n\
  id 0\n");

	/* go through all vertices */
	rotz_vtx_iter(ctx, iter_gmlv_cb, ctx);

	/* go through all edges */
	rotz_edg_iter(ctx, iter_gmle_cb, ctx);

	puts("]");
	return;
}


#if defined STANDALONE
#if defined __INTEL_COMPILER
# pragma warning (disable:593)
#endif	/* __INTEL_COMPILER */
#include "rotz-export.h"
#include "rotz-export.x"
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
	if (UNLIKELY((ctx = make_rotz(db)) == NULL)) {
		fputs("Error opening rotz datastore\n", stderr);
		res = 1;
		goto out;
	}

	/* setting global opts */
	clusterp = argi->cluster_given;

	if (argi->gml_given) {
		xprt_gml(ctx);
	} else if (argi->dot_given) {
		xprt_dot(ctx);
	} else {
		/* default file format is csv */
		xprt_csv(ctx);
	}

	/* big resource freeing */
	free_rotz(ctx);
out:
	rotz_parser_free(argi);
	return res;
}
#endif	/* STANDALONE */

/* rotz-export.c ends here */

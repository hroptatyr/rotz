/*** rotz-search.c -- rotz tag search
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

struct iter_clo_s {
	rotz_t ctx;
	rtz_const_buf_t prfx;
	size_t ntop;
	unsigned int show_numbers;
};


static int
iter_cb(rtz_const_buf_t k, rtz_const_buf_t v, void *clo)
{
	static size_t iter;
	const struct iter_clo_s *cp = clo;

	/* just print the name and stats */
	fputs(rotz_massage_name(k.d), stdout);
	if (cp->show_numbers) {
		/* get the number also */
		const rtz_vtx_t *vp = (const void*)v.d;

		fputc('\t', stdout);
		fprintf(stdout, "%zu", rotz_get_nedges(cp->ctx, *vp));
	}
	fputc('\n', stdout);
	iter++;
	return 0;
}

static void
iter(rotz_t ctx, const struct iter_clo_s *clo)
{
	rotz_iter(ctx, clo->prfx, iter_cb, clo);
	return;
}


#if defined STANDALONE
#if defined __INTEL_COMPILER
# pragma warning (disable:593)
#endif	/* __INTEL_COMPILER */
#include "rotz-search.h"
#include "rotz-search.x"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
#endif	/* __INTEL_COMPILER */

int
main(int argc, char *argv[])
{
	static struct iter_clo_s clo[1];
	struct rotz_args_info argi[1];
	const char *db = RTZ_DFLT_DB;
	rotz_t ctx;
	int res = 0;

	if (rotz_parser(argc, argv, argi)) {
		res = 1;
		goto out;
	} else if (argi->inputs_num < 1) {
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
	clo->ctx = ctx;
	clo->prfx.d = rotz_tag(argi->inputs[0]);
	clo->prfx.z = strlen(clo->prfx.d);
	clo->show_numbers = 1;

	if (argi->top_given) {
		clo->ntop = argi->top_arg;
	} else {
		clo->ntop = -1UL;
	}
	iter(ctx, clo);

	/* big resource freeing */
	free_rotz(ctx);
out:
	rotz_parser_free(argi);
	return res;
}
#endif	/* STANDALONE */

/* rotz-search.c ends here */

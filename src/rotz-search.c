/*** rotz-search.c -- rotz tag search
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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "rotz.h"
#include "rotz-cmd-api.h"
#include "rotz-umb.h"
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
int
rotz_cmd_search(const struct yuck_cmd_search_s argi[static 1U])
{
	static struct iter_clo_s clo[1];
	rotz_t ctx;

	if (argi->nargs < 1) {
		fputs("Error: need a search string\n", stderr);
		return 1;
	}

	if (UNLIKELY((ctx = make_rotz(db)) == NULL)) {
		fputs("Error opening rotz datastore\n", stderr);
		return 1;
	}

	/* cloud all tags mode, undocumented prefix feature */
	clo->ctx = ctx;
	clo->prfx.d = rotz_tag(argi->args[0U]);
	clo->prfx.z = strlen(clo->prfx.d);
	clo->show_numbers = 1;

	if (argi->top_arg) {
		clo->ntop = strtoul(argi->top_arg, NULL, 0);
	} else {
		clo->ntop = -1UL;
	}
	iter(ctx, clo);

	/* big rcource freeing */
	free_rotz(ctx);
	return 0;
}
#endif	/* STANDALONE */

/* rotz-search.c ends here */

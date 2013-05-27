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
#include <tcbdb.h>

#include "rotz.h"
#include "nifty.h"


/* namespacify our objects */
/* lib stuff? */
static const char*
rotz_glue(const char *pre, const char *str, size_t ssz)
{
/* produces PRE:STR, all *our* prefixes are 3 chars long */
	static struct {
		size_t z;
		char *d;
	} builder;

	if (UNLIKELY(4U/*pre*/ + ssz + 1U/*\nul*/ > builder.z)) {
		builder.z = ((4U + ssz) / 64U + 1U) * 64U;
		builder.d = realloc(builder.d, builder.z);
	}
	memcpy(builder.d, pre, 3U);
	builder.d[3] = ':';
	memcpy(builder.d + 4U, str, ssz + 1U/*\nul*/);
	return builder.d;
}

static const char*
rotz_maybe_glue(const char *pre, const char *str)
{
	const char *p;

	if (UNLIKELY(*(p = strchrnul(str, ':')))) {
		return str;
	}
	/* otherwise glue */
	return rotz_glue(pre, str, p - str);
}

static const char*
rotz_tag(const char *tag)
{
	return rotz_maybe_glue("tag", tag);
}

static const char*
rotz_sym(const char *sym)
{
	return rotz_glue("sym", sym, strlen(sym));
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
	const char *tagsym;
	rtz_vtx_t tsid;
	int res = 0;

	if (rotz_parser(argc, argv, argi)) {
		res = 1;
		goto out;
	} else if (argi->inputs_num < 1) {
		fputs("Error: no TAG argument specified\n\n", stderr);
		rotz_parser_print_help();
		res = 1;
		goto out;
	}

	if (UNLIKELY((ctx = make_rotz("rotz.tcb")) == NULL)) {
		fputs("Error opening rotz datastore\n", stderr);
		res = 1;
		goto out;
	}
	tagsym = rotz_tag(argi->inputs[0]);
	if (UNLIKELY((tsid = rotz_get_vertex(ctx, tagsym)) == 0U)) {
		goto fini;
	}
	{
		rtz_vtxlst_t vl = rotz_get_edges(ctx, tsid);

		for (size_t i = 0; i < vl.z; i++) {
			const char *const s = rotz_get_name(ctx, vl.d[i]);

			if (UNLIKELY(s == NULL)) {
				/* uh oh */
				continue;
			}
			puts(s);
		}

		rotz_free_vtxlst(vl);
	}

fini:
	/* big resource freeing */
	free_rotz(ctx);
out:
	rotz_parser_free(argi);
	return res;
}
#endif	/* STANDALONE */

/* rotz-show.c ends here */

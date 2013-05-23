/*** rotz-add.c -- rotz tag adder
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

#if !defined LIKELY
# define LIKELY(_x)	__builtin_expect((_x), 1)
#endif	/* !LIKELY */
#if !defined UNLIKELY
# define UNLIKELY(_x)	__builtin_expect((_x), 0)
#endif	/* UNLIKELY */
#if !defined UNUSED
# define UNUSED(x)	__attribute__((unused)) x
#endif	/* UNUSED */

typedef enum {
	RTZCTX_INIT,
	RTZCTX_GET,
	RTZCTX_FINI,
} rtzctx_action_t;


static TCBDB*
rotz_ctx(rtzctx_action_t what)
{
	static int omode = BDBOREADER | BDBOWRITER | BDBOCREAT;
	static TCBDB *tc;

	switch (what) {
	case RTZCTX_INIT:
		if (LIKELY(tc == NULL)) {
			tc = tcbdbnew();
			tcbdbopen(tc, "rotz.hence", omode);
		}
		break;

	case RTZCTX_FINI:
		if (LIKELY(tc != NULL)) {
			tcbdbclose(tc);
			tcbdbdel(tc);
			tc = NULL;
		}
		break;

	case RTZCTX_GET:
	default:
		break;
	}
	return tc;
}

static const char*
find_in_strlst(rtz_strlst_t lst, const char *str, size_t stz)
{
	const char *rest = lst.d;
	size_t rezt = lst.n;

	/* include the final \nul in the needle search */
	stz++;

	for (const char *sp;
	     (sp = memmem(rest, rezt, str, stz)) != NULL;
	     rest += stz, rezt -= stz) {
		/* make sure we don't find a suffix */
		if (sp == rest || sp[-1] == '\0') {
			return sp;
		}
	}
	return NULL;
}


rtz_strlst_t
rotz_get_syms(rtzctx_t UNUSED(ctx), const char *tag)
{
	TCBDB *tc = rotz_ctx(RTZCTX_GET);
	const char *sp;
	int z[1];

	if (UNLIKELY((sp = tcbdbget3(tc, tag, strlen(tag), z)) == NULL)) {
		return (rtz_strlst_t){0U};
	}
	return (rtz_strlst_t){.n = (size_t)*z, .d = sp};
}

void
rotz_add(rtzctx_t ctx, const char *tag, const char *sym)
{
	TCBDB *tc = rotz_ctx(RTZCTX_GET);
	rtz_strlst_t so_far;
	size_t taz = strlen(tag);
	size_t syz = strlen(sym);

	if ((so_far = rotz_get_syms(ctx, tag)).d != NULL) {
		if (UNLIKELY(find_in_strlst(so_far, sym, syz) != NULL)) {
			/* sym is already in there, so fuck off */
			return;
		}
	}
	tcbdbputcat(tc, tag, taz, sym, syz + 1/*for \nul*/);
	return;
}


#if defined STANDALONE
#if defined __INTEL_COMPILER
# pragma warning (disable:593)
#endif	/* __INTEL_COMPILER */
#include "rotz-add-clo.h"
#include "rotz-add-clo.c"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
#endif	/* __INTEL_COMPILER */

int
main(int argc, char *argv[])
{
	struct rotz_args_info argi[1];
	rtzctx_t ctx;
	const char *tag;
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

	ctx = rotz_ctx(RTZCTX_INIT);
	tag = argi->inputs[0];
	for (unsigned int i = 1; i < argi->inputs_num; i++) {
		const char *sym = argi->inputs[i];

		rotz_add(ctx, tag, sym);
	}
	rotz_ctx(RTZCTX_FINI);

out:
	rotz_parser_free(argi);
	return res;
}
#endif	/* STANDALONE */

/* rotz-add.c ends here */

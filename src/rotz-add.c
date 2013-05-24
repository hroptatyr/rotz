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

struct rtzctx_s {
	TCBDB *t2s;
	TCBDB *s2t;
};


static rtzctx_t
rotz_init(void)
{
	static const int omode = BDBOREADER | BDBOWRITER | BDBOCREAT;
	struct rtzctx_s res;

	if (UNLIKELY((res.t2s = tcbdbnew()) == NULL)) {
		goto out;
	} else if (UNLIKELY((res.s2t = tcbdbnew()) == NULL)) {
		goto out_free_t2s;
	} else if (!tcbdbopen(res.t2s, "rotz.hence", omode)) {
		goto out_free_s2t;
	} else if (!tcbdbopen(res.s2t, "rotz.forth", omode)) {
		goto out_clo_hence;
	}

	/* clone the result */
	{
		struct rtzctx_s *resp = malloc(sizeof(*resp));
		*resp = res;
		return resp;
	}

out_clo_hence:
	tcbdbclose(res.t2s);
out_free_s2t:
	tcbdbdel(res.s2t);
out_free_t2s:
	tcbdbdel(res.t2s);
out:
	return NULL;
}

static void
rotz_fini(rtzctx_t ctx)
{
	struct rtzctx_s *restrict cp = ctx;

	tcbdbclose(cp->s2t);
	tcbdbclose(cp->t2s);
	tcbdbdel(cp->s2t);
	tcbdbdel(cp->t2s);
	free(ctx);
	return;
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
rotz_get_syms(rtzctx_t ctx, const char *tag)
{
	struct rtzctx_s *restrict cp = ctx;
	const char *sp;
	int z[1];

	if (UNLIKELY((sp = tcbdbget3(cp->t2s, tag, strlen(tag), z)) == NULL)) {
		return (rtz_strlst_t){0U};
	}
	return (rtz_strlst_t){.n = (size_t)*z, .d = sp};
}

rtz_strlst_t
rotz_get_tags(rtzctx_t ctx, const char *sym)
{
	struct rtzctx_s *restrict cp = ctx;
	const char *sp;
	int z[1];

	if (UNLIKELY((sp = tcbdbget3(cp->s2t, sym, strlen(sym), z)) == NULL)) {
		return (rtz_strlst_t){0U};
	}
	return (rtz_strlst_t){.n = (size_t)*z, .d = sp};
}

void
rotz_add(rtzctx_t ctx, const char *tag, const char *sym)
{
	struct rtzctx_s *restrict cp = ctx;
	rtz_strlst_t so_far;
	size_t taz = strlen(tag);
	size_t syz = strlen(sym);

	if ((so_far = rotz_get_syms(ctx, tag)).d != NULL) {
		if (UNLIKELY(find_in_strlst(so_far, sym, syz) != NULL)) {
			/* sym is already in there, so fuck off */
			goto step2;
		}
	}
	tcbdbputcat(cp->t2s, tag, taz, sym, syz + 1/*for \nul*/);

step2:
	if ((so_far = rotz_get_tags(ctx, sym)).d != NULL) {
		if (UNLIKELY(find_in_strlst(so_far, tag, taz) != NULL)) {
			/* tag is already in there, so fuck off */
			goto final;
		}
	}
	tcbdbputcat(cp->s2t, sym, syz, tag, taz + 1/*for \nul*/);

final:
	return;
}


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

	if (UNLIKELY((ctx = make_rotz("rotz.tcb")) == NULL)) {
		fputs("Error opening rotz datastore\n", stderr);
		res = 1;
		goto out;
	}
	tag = rotz_tag(argi->inputs[0]);
	rotz_add_vertex(ctx, tag);
	for (unsigned int i = 1; i < argi->inputs_num; i++) {
		const char *sym = rotz_sym(argi->inputs[i]);

		rotz_add_vertex(ctx, sym);
	}

	/* big resource freeing */
	free_rotz(ctx);
out:
	rotz_parser_free(argi);
	return res;
}
#endif	/* STANDALONE */

/* rotz-add.c ends here */

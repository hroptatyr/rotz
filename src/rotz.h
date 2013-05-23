/*** rotz.h -- master rotz api
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
#if !defined INCLUDED_rotz_h_
#define INCLUDED_rotz_h_

#include <stdlib.h>
#include <stdint.h>

typedef void *rtzctx_t;

/* for multiple \nul terminated strings, N is the total length in bytes. */
typedef struct {
	size_t n;
	const char *d;
} rtz_strlst_t;

/* our tags are 1-adic maps:
 * PRE+KEY -> {SYM, ...}  and
 * SYM -> {PRE+KEY, ...}  where
 * PRE for the 0-adic case is always `tag'. */
extern void
rotz_add(rtzctx_t, const char *tag, const char *sym);

extern void
rotz_del(rtzctx_t, const char *tag, const char *sym);

extern void
rotz_del_tag(rtzctx_t, const char *tag);

extern void
rotz_del_sym(rtzctx_t, const char *sym);

extern rtz_strlst_t rotz_get_tags(rtzctx_t, const char *sym);

extern rtz_strlst_t rotz_get_syms(rtzctx_t, const char *tag);

#endif	/* INCLUDED_rotz_h_ */

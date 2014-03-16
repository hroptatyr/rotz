/*** rotz-cmd-api.h -- some useful glue between rotz commands
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
#if !defined INCLUDED_rotz_cmd_api_h_
#define INCLUDED_rotz_cmd_api_h_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include "nifty.h"

#define RTZ_TAGSPC	"tag"
#define RTZ_SYMSPC	":::"
#define RTZ_PRE_Z	(4U)

#if defined USE_LMDB
# define RTZ_DFLT_DB	"rotz.mdb"
#elif defined USE_TCBDB
# define RTZ_DFLT_DB	"rotz.tcb"
#else
# error cannot define default database file name
#endif	/* USE_*DB */


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

	if (UNLIKELY(ssz == 0U)) {
		return NULL;
	} else if (UNLIKELY(RTZ_PRE_Z + ssz + 1U/*\nul*/ > builder.z)) {
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

static inline const char*
rotz_tag(const char *tag)
{
	return rotz_maybe_glue(RTZ_TAGSPC, tag);
}

static inline const char*
rotz_sym(const char *sym)
{
	return rotz_glue(RTZ_SYMSPC, sym, strlen(sym));
}

static inline __attribute__((pure, const)) const char*
rotz_massage_name(const char *ts)
{
	if (UNLIKELY(ts == NULL)) {
		;
	} else if (memcmp(ts, RTZ_SYMSPC, sizeof(RTZ_SYMSPC) - 1) == 0) {
		ts += RTZ_PRE_Z;
	} else if (memcmp(ts, RTZ_TAGSPC, sizeof(RTZ_TAGSPC) - 1) == 0) {
		ts += RTZ_PRE_Z;
	}
	return ts;
}

#endif	/* INCLUDED_rotz_cmd_api_h_ */

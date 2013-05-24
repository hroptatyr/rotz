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
#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <tcbdb.h>

#include "rotz.h"
#include "nifty.h"

struct rotz_s {
	TCBDB *db;
};


static rtzid_t
next_id(rotz_t cp)
{
	static const char nid[] = "\x1d";
	int res;

	if (UNLIKELY((res = tcbdbaddint(cp->db, nid, sizeof(nid), 1)) <= 0)) {
		return 0U;
	}
	return (rtzid_t)res;
}

static rtzid_t
get_vertex(rotz_t cp, const char *v, size_t z)
{
	int res;

	if (UNLIKELY((res = tcbdbaddint(cp->db, v, z, 0)) <= 0)) {
		return 0U;
	}
	return (rtzid_t)res;
}

static int
put_vertex(rotz_t cp, const char *v, size_t z, rtzid_t i)
{
	if (tcbdbaddint(cp->db, v, z, (int)i) <= 0) {
		return -1;
	}
	return 0;
}

static int
rem_vertex(rotz_t cp, const char *v, size_t z, rtzid_t UNUSED(i))
{
	if (!tcbdbout(cp->db, v, z)) {
		return -1;
	}
	return 0;
}


/* low level graph lib */
rotz_t
make_rotz(const char *db)
{
	static const int omode = BDBOREADER | BDBOWRITER | BDBOCREAT;
	struct rotz_s res;

	if (UNLIKELY((res.db = tcbdbnew()) == NULL)) {
		goto out;
	} else if (UNLIKELY(!tcbdbopen(res.db, db, omode))) {
		goto out_free_db;
	}

	/* clone the result */
	{
		struct rotz_s *resp = malloc(sizeof(*resp));
		*resp = res;
		return resp;
	}

out_free_db:
	tcbdbdel(res.db);
out:
	return NULL;
}

void
free_rotz(rotz_t ctx)
{
	tcbdbclose(ctx->db);
	tcbdbdel(ctx->db);
	free(ctx);
	return;
}


/* vertex accessors */
rtzid_t
rotz_get_vertex(rotz_t ctx, const char *v)
{
	return get_vertex(ctx, v, strlen(v));
}

rtzid_t
rotz_add_vertex(rotz_t ctx, const char *v)
{
	size_t z = strlen(v);
	rtzid_t res;

	/* first check if V is already there, if not get an id and add that */
	if ((res = get_vertex(ctx, v, z))) {
		;
	} else if (UNLIKELY(!(res = next_id(ctx)))) {
		;
	} else if (UNLIKELY(put_vertex(ctx, v, z, res) < 0)) {
		res = 0U;
	}
	return res;
}

rtzid_t
rotz_rem_vertex(rotz_t ctx, const char *v)
{
	size_t z = strlen(v);
	rtzid_t res;

	/* first check if V is really there, if not get an id and add that */
	if (UNLIKELY(!(res = get_vertex(ctx, v, z)))) {
		;
	} else if (UNLIKELY(rem_vertex(ctx, v, z, res) < 0)) {
		res = 0U;
	}
	return res;
}

/* rotz.c ends here */

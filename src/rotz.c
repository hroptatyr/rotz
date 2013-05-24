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
		tcbdbout(cp->db, v, z);
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

/* API */
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


/* edge accessors */
/* for multiple \nul terminated strings, N is the total length in bytes. */
typedef const unsigned char *rtzvtx_t;
#define RTZVTX_Z	(8U)

typedef struct {
	size_t z;
	const rtzid_t *d;
} const_edglst_t;

static rtzvtx_t
rotz_vtx(rtzid_t vid)
{
/* return the key for the incidence list */
	static unsigned char vtx[RTZVTX_Z] = "vtx:";
	unsigned int *vi = (void*)(vtx + 4U);

	*vi = vid;
	return vtx;
}

static const_edglst_t
get_edges(rotz_t ctx, rtzvtx_t sfrom)
{
	const void *sp;
	int z[1];

	if (UNLIKELY((sp = tcbdbget3(ctx->db, sfrom, RTZVTX_Z, z)) == NULL)) {
		return (const_edglst_t){0U};
	}
	return (const_edglst_t){.z = (size_t)*z / sizeof(rtzid_t), .d = sp};
}

static size_t
find_in_edglst(const_edglst_t el, rtzid_t to)
{
	for (size_t i = 0; i < el.z; i++) {
		if (el.d[i] == to) {
			return i + 1U;
		}
	}
	return 0U;
}

static const_edglst_t
rem_from_edglst(const_edglst_t el, size_t idx)
{
	static rtzid_t *edgspc;
	static size_t edgspz;
	rtzid_t *ep;

	if (UNLIKELY(el.z * sizeof(*edgspc) > edgspz)) {
		edgspz = ((el.z * sizeof(*edgspc) - 1) / 64U + 1U) * 64U;
		edgspc = realloc(edgspc, edgspz);
	}
	/* IDX points to the elements after the deletee
	 * so IDX - 1 points to the deletee and
	 * if IDX - 1 > 0 then [0, IDX - 1) are the elements before */
	ep = edgspc;
	if (idx - 1U) {
		memcpy(ep, el.d, (idx - 1U) * sizeof(*el.d));
		ep += idx - 1U;
	}
	if (idx < el.z) {
		el.d += idx;
		el.z -= idx;
		memcpy(ep, el.d, el.z * sizeof(*el.d));
		ep += el.z;
	}
	return (const_edglst_t){.z = ep - edgspc, .d = edgspc};
}

static int
add_edge(rotz_t ctx, rtzvtx_t sfrom, rtzid_t to)
{
	return tcbdbputcat(ctx->db, sfrom, RTZVTX_Z, &to, sizeof(to)) - 1;
}

static int
add_edglst(rotz_t ctx, rtzvtx_t sfrom, const_edglst_t el)
{
	size_t z;

	if (UNLIKELY((z = el.z * sizeof(*el.d)) == 0U)) {
		return tcbdbout(ctx->db, sfrom, RTZVTX_Z) - 1;
	}
	return tcbdbput(ctx->db, sfrom, RTZVTX_Z, el.d, z) - 1;
}

/* API */
int
rotz_get_edge(rotz_t ctx, rtzid_t from, rtzid_t to)
{
	rtzvtx_t sfrom = rotz_vtx(from);
	const_edglst_t el;

	/* get edges under */
	if (LIKELY((el = get_edges(ctx, sfrom)).d != NULL) &&
	    LIKELY(find_in_edglst(el, to) > 0U)) {
		/* to is already there */
		return 1;
	}
	return 0;
}

rtz_edglst_t
rotz_get_edges(rotz_t ctx, rtzid_t from)
{
	rtzvtx_t sfrom = rotz_vtx(from);
	const_edglst_t el;
	rtzid_t *d;

	/* get edges under */
	if (UNLIKELY((el = get_edges(ctx, sfrom)).d == NULL)) {
		return (rtz_edglst_t){0U};
	}
	/* otherwise make a copy */
	d = malloc(el.z * sizeof(*d));
	memcpy(d, el.d, el.z);
	return (rtz_edglst_t){.z = el.z, .d = d};
}

int
rotz_rem_edges(rotz_t ctx, rtzid_t from)
{
	rtzvtx_t sfrom = rotz_vtx(from);

	return add_edglst(ctx, sfrom, (const_edglst_t){0U});
}

void
rotz_free_edglst(rtz_edglst_t el)
{
	if (LIKELY(el.d != NULL)) {
		free(el.d);
	}
	return;
}

int
rotz_add_edge(rotz_t ctx, rtzid_t from, rtzid_t to)
{
	rtzvtx_t sfrom = rotz_vtx(from);
	const_edglst_t el;

	/* get edges under */
	if ((el = get_edges(ctx, sfrom)).d != NULL &&
	    UNLIKELY(find_in_edglst(el, to) > 0U)) {
		/* to is already there */
		return 0;
	}
	if (UNLIKELY(add_edge(ctx, sfrom, to) < 0)) {
		return -1;
	}
	return 1;
}

int
rotz_rem_edge(rotz_t ctx, rtzid_t from, rtzid_t to)
{
	rtzvtx_t sfrom = rotz_vtx(from);
	const_edglst_t el;
	size_t idx;

	/* get edges under */
	if (UNLIKELY((el = get_edges(ctx, sfrom)).d == NULL) ||
	    UNLIKELY((idx = find_in_edglst(el, to)) == 0U)) {
		/* TO isn't in there */
		return 0;
	}
	if (UNLIKELY((el = rem_from_edglst(el, idx)).d == NULL)) {
		/* huh? */
		return -1;
	}
	add_edglst(ctx, sfrom, el);
	return 1;
}

/* rotz.c ends here */

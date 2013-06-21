/*** rotz-tcbdb.c -- tokyocabinet stuff
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
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <tcbdb.h>

#include "rotz.h"
#include "nifty.h"

struct rotz_s {
	TCBDB *db;
};


/* low level graph lib */
rotz_t
make_rotz(const char *db, ...)
{
	va_list ap;
	int omode = BDBOREADER;
	int oparam;
	struct rotz_s res;

	va_start(ap, db);
	oparam = va_arg(ap, int);
	va_end(ap);

	if (oparam & O_RDWR) {
		omode |= BDBOWRITER;
	}
	if (oparam & O_CREAT) {
		omode |= BDBOCREAT;
	}

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


static rtz_vtx_t
next_id(rotz_t cp)
{
	static const char nid[] = "\x1d";

	int res;

	if (UNLIKELY((res = tcbdbaddint(cp->db, nid, sizeof(nid), 1)) <= 0)) {
		return 0U;
	}
	return (rtz_vtx_t)res;
}

static rtz_vtx_t
get_vertex(rotz_t cp, const char *v, size_t z)
{
	rtz_vtx_t res;

	const int *rp;
	int rz[1];

	if (UNLIKELY((rp = tcbdbget3(cp->db, v, z, rz)) == NULL)) {
		return 0U;
	} else if (UNLIKELY(*rz != sizeof(*rp))) {
		return 0U;
	}
	res = (rtz_vtx_t)*rp;
	return res;
}

static int
put_vertex(rotz_t cp, const char *a, size_t az, rtz_vtx_t v)
{
	int res = 0;

	res = tcbdbaddint(cp->db, a, az, (int)v) - 1;
	return res;
}

static int
rnm_vertex(rotz_t cp, rtz_vtxkey_t vkey, const char *v, size_t z)
{
	int res = 0;

	if (UNLIKELY(!tcbdbput(cp->db, vkey, RTZ_VTXKEY_Z, v, z + 1))) {
		tcbdbout(cp->db, v, z);
		res = -1;
	}
	return res;
}

static int
unput_vertex(rotz_t cp, const char *v, size_t z)
{
	int res = 0;

	res = tcbdbout(cp->db, v, z) - 1;
	return res;
}

static int
unrnm_vertex(rotz_t cp, rtz_vtxkey_t vkey)
{
	int res = 0;

	res = tcbdbout(cp->db, vkey, RTZ_VTXKEY_Z) - 1;
	return res;
}

static int
add_alias(rotz_t cp, rtz_vtxkey_t vkey, const char *a, size_t az)
{
	int res = 0;

	res = tcbdbputcat(cp->db, vkey, RTZ_VTXKEY_Z, a, az + 1) - 1;
	return res;
}

static int
add_akalst(rotz_t ctx, rtz_vtxkey_t ak, const_buf_t al)
{
	int res = 0;
	size_t z;

	if (UNLIKELY((z = al.z * sizeof(*al.d)) == 0U)) {
		return tcbdbout(ctx->db, ak, RTZ_VTXKEY_Z) - 1;
	}
	res = tcbdbput(ctx->db, ak, RTZ_VTXKEY_Z, al.d, z) - 1;
	return res;
}

static const_buf_t
get_aliases(rotz_t cp, rtz_vtxkey_t svtx)
{
	const_buf_t res;
	const void *sp;
	int z[1];

	if (UNLIKELY((sp = tcbdbget3(cp->db, svtx, RTZ_VTXKEY_Z, z)) == NULL)) {
		return (const_buf_t){0U};
	}

	res = (const_buf_t){.z = (size_t)*z, .d = sp};
	return res;
}

static rtz_buf_t
get_aliases_r(rotz_t cp, rtz_vtxkey_t svtx)
{
	rtz_buf_t res;
	void *sp;
	int z[1];

	if (UNLIKELY((sp = tcbdbget(cp->db, svtx, RTZ_VTXKEY_Z, z)) == NULL)) {
		return (rtz_buf_t){0U};
	}
	res = (rtz_buf_t){.z = (size_t)*z, .d = sp};
	return res;
}


/* edge accessors */
static const_vtxlst_t
get_edges(rotz_t ctx, rtz_edgkey_t src)
{
	const_vtxlst_t res;
	const void *sp;
	int z[1];

	if (UNLIKELY((sp = tcbdbget3(ctx->db, src, RTZ_EDGKEY_Z, z)) == NULL)) {
		return (const_vtxlst_t){0U};
	}
	res = (const_vtxlst_t){.z = (size_t)*z / sizeof(rtz_vtx_t), .d = sp};
	return res;
}

static int
add_edge(rotz_t ctx, rtz_edgkey_t src, rtz_vtx_t to)
{
	int res = 0;

	res = tcbdbputcat(ctx->db, src, RTZ_EDGKEY_Z, &to, sizeof(to)) - 1;
	return res;
}

static int
add_vtxlst(rotz_t ctx, rtz_edgkey_t src, const_vtxlst_t el)
{
	int res = 0;
	size_t z;

	if (UNLIKELY((z = el.z * sizeof(*el.d)) == 0U)) {
		return tcbdbout(ctx->db, src, RTZ_EDGKEY_Z) - 1;
	}
	res = tcbdbput(ctx->db, src, RTZ_EDGKEY_Z, el.d, z) - 1;
	return res;
}

static int
rem_edges(rotz_t ctx, rtz_edgkey_t src)
{
	int res = 0;

	res = tcbdbout(ctx->db, src, RTZ_EDGKEY_Z) - 1;
	return res;
}


/* iterators
 * we can't keep the promise here to separate keys and tokyocabinet guts */
void
rotz_vtx_iter(rotz_t ctx, int(*cb)(rtz_vtx_t, const char*, void*), void *clo)
{
	BDBCUR *c = tcbdbcurnew(ctx->db);

	tcbdbcurjump(c, RTZ_VTXPRE, sizeof(RTZ_VTXPRE));
	do {
		int z[1];
		const void *kp;
		rtz_vtx_t vid;
		const void *vp;

		if (UNLIKELY((kp = tcbdbcurkey3(c, z)) == NULL) ||
		    UNLIKELY(*z != sizeof(RTZ_VTXPRE) + sizeof(vid)) ||
		    UNLIKELY(!(vid = rtz_vtx(kp)))) {
			break;
		} else if (UNLIKELY((vp = tcbdbcurval3(c, z)) == NULL)) {
			continue;
		}
		/* otherwise just call the callback */
		if (UNLIKELY(cb(vid, vp, clo) < 0)) {
			break;
		}

	} while (tcbdbcurnext(c));

	tcbdbcurdel(c);
	return;
}

void
rotz_edg_iter(rotz_t ctx, int(*cb)(rtz_vtx_t, const_vtxlst_t, void*), void *clo)
{
	rtz_vtxlst_t vl = {.z = 0U};
	BDBCUR *c = tcbdbcurnew(ctx->db);

	tcbdbcurjump(c, RTZ_EDGPRE, sizeof(RTZ_EDGPRE));
	do {
		int z[1];
		const void *kp;
		rtz_vtx_t vid;
		const void *vp;
		const_vtxlst_t cvl;

		if (UNLIKELY((kp = tcbdbcurkey3(c, z)) == NULL) ||
		    UNLIKELY(*z != sizeof(RTZ_EDGPRE) + sizeof(vid)) ||
		    UNLIKELY(!(vid = rtz_edg(kp)))) {
			break;
		} else if (UNLIKELY((vp = tcbdbcurval3(c, z)) == NULL)) {
			continue;
		}
		/* copy the vl */
		cvl.z = *z / sizeof(*vl.d);
		if (UNLIKELY(cvl.z > vl.z)) {
			vl.z = ((cvl.z - 1) / 64U + 1) * 64U;
			vl.d = realloc(vl.d, vl.z * sizeof(*vl.d));
		}
		memcpy(vl.d, vp, cvl.z * sizeof(*cvl.d));
		cvl.d = vl.d;
		/* otherwise just call the callback */
		if (UNLIKELY(cb(vid, cvl, clo) < 0)) {
			break;
		}

	} while (tcbdbcurnext(c));

	tcbdbcurdel(c);
	rotz_free_vtxlst(vl);
	return;
}

void
rotz_iter(
	rotz_t ctx, rtz_const_buf_t prfx_match,
	int(*cb)(rtz_const_buf_t key, rtz_const_buf_t val, void*), void *clo)
{
	BDBCUR *c = tcbdbcurnew(ctx->db);

#define B	(const_buf_t)
	tcbdbcurjump(c, prfx_match.d, prfx_match.z);
	do {
		int z[2];
		const void *kp;
		const void *vp;

		if (UNLIKELY((kp = tcbdbcurkey3(c, z + 0)) == NULL) ||
		    UNLIKELY(memcmp(kp, prfx_match.d, prfx_match.z))) {
			break;
		} else if (UNLIKELY((vp = tcbdbcurval3(c, z + 1)) == NULL)) {
			continue;
		}
		/* otherwise just call the callback */
		if (UNLIKELY(cb(B{z[0], kp}, B{z[1], vp}, clo) < 0)) {
			break;
		}

	} while (tcbdbcurnext(c));
#undef B

	tcbdbcurdel(c);
	return;
}

/* rotz-tcbdb.c ends here */

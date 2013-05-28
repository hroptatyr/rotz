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


/* vertex accessors
 * Our policy is that routines that know about tokyocabinet must not
 * know about vertex keys, aka keys, and so forth.
 * And vice versa. */
typedef const unsigned char *rtz_vtxkey_t;
#define RTZ_VTXPRE	"vtx"
#define RTZ_VTXKEY_Z	(sizeof(RTZ_VTXPRE) + sizeof(rtz_vtx_t))
#define RTZ_AKAPRE	"aka"
#define RTZ_AKAKEY_Z	(sizeof(RTZ_AKAPRE) + sizeof(rtz_vtx_t))

typedef struct {
	size_t z;
	const char *d;
} const_buf_t;

static rtz_vtxkey_t
rtz_vtxkey(rtz_vtx_t vid)
{
/* return the key for the incidence list */
	static unsigned char vtx[RTZ_VTXKEY_Z] = RTZ_VTXPRE;
	unsigned int *vi = (void*)(vtx + sizeof(RTZ_VTXPRE));

	*vi = vid;
	return vtx;
}

static rtz_vtxkey_t
rtz_akakey(rtz_vtx_t v)
{
/* return the key for the incidence list */
	static unsigned char aka[RTZ_AKAKEY_Z] = RTZ_AKAPRE;
	unsigned int *vi = (void*)(aka + sizeof(RTZ_AKAPRE));

	*vi = v;
	return aka;
}

static rtz_vtx_t
rtz_vtx(rtz_vtxkey_t x)
{
	const unsigned int *vi = (const void*)(x + sizeof(RTZ_VTXPRE));
	return *vi;
}

static const char*
find_in_buf(const_buf_t b, const char *s, size_t z)
{
	/* include the final \nul in the needle search */
	z++;

	for (const char *sp;
	     (sp = memmem(b.d, b.z, s, z)) != NULL;
	     b.d += z, b.z -= z) {
		/* make sure we don't find just a suffix */
		if (sp == b.d || sp[-1] == '\0') {
			return sp;
		}
	}
	return NULL;
}

static const_buf_t
rem_from_buf(const_buf_t b, const char *s, size_t z)
{
	static char *akaspc;
	static size_t akaspz;
	char *ap;

	if (UNLIKELY(s < b.d || s + z > b.d + b.z)) {
		/* definitely not in our array */
		return b;
	} else if (UNLIKELY(b.z > akaspz)) {
		akaspz = ((b.z - 1) / 64U + 1U) * 64U;
		akaspc = realloc(akaspc, akaspz);
	}
	/* S points to the element to delete */
	ap = akaspc;
	if (s > b.d) {
		/* cpy the stuff before S */
		memcpy(ap, b.d, (s - b.d));
		ap += s - b.d;
	}
	if (s - b.d + z < b.z) {
		/* cpy the stuff after S */
		b.z -= s - b.d + z;
		memcpy(ap, s + z, b.z);
		ap += b.z;
	}
	return (const_buf_t){.z = ap - akaspc, .d = akaspc};
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
	int res;

	if (UNLIKELY((res = tcbdbaddint(cp->db, v, z, 0)) <= 0)) {
		tcbdbout(cp->db, v, z);
		return 0U;
	}
	return (rtz_vtx_t)res;
}

static int
put_vertex(rotz_t cp, const char *a, size_t az, rtz_vtx_t v)
{
	return tcbdbaddint(cp->db, a, az, (int)v) - 1;
}

static int
rnm_vertex(rotz_t cp, rtz_vtxkey_t vkey, const char *v, size_t z)
{
	if (UNLIKELY(!tcbdbput(cp->db, vkey, RTZ_VTXKEY_Z, v, z))) {
		tcbdbout(cp->db, v, z);
		return -1;
	}
	return 0;
}

static int
unput_vertex(rotz_t cp, const char *v, size_t z)
{
	return tcbdbout(cp->db, v, z) - 1;
}

static int
unrnm_vertex(rotz_t cp, rtz_vtxkey_t vkey)
{
	return tcbdbout(cp->db, vkey, RTZ_VTXKEY_Z) - 1;
}

static int
add_vertex(rotz_t cp, const char *v, size_t z, rtz_vtx_t i)
{
	if (UNLIKELY(put_vertex(cp, v, z, i) < 0)) {
		return -1;
	}
	/* act as though we're renaming the vertex */
	return rnm_vertex(cp, rtz_vtxkey(i), v, z);
}

static int
rem_vertex(rotz_t cp, rtz_vtx_t i, const char *v, size_t z)
{
	int res = 0;

	res += unput_vertex(cp, v, z);
	res += unrnm_vertex(cp, rtz_vtxkey(i));
	return res;
}

static int
add_alias(rotz_t cp, rtz_vtxkey_t vkey, const char *a, size_t az)
{
	return tcbdbputcat(cp->db, vkey, RTZ_VTXKEY_Z, a, az + 1) - 1;
}

static int
add_akalst(rotz_t ctx, rtz_vtxkey_t key, const_buf_t al)
{
	size_t z;

	if (UNLIKELY((z = al.z * sizeof(*al.d)) == 0U)) {
		return tcbdbout(ctx->db, key, RTZ_AKAKEY_Z) - 1;
	}
	return tcbdbput(ctx->db, key, RTZ_AKAKEY_Z, al.d, z) - 1;
}

#define get_aliases	get_name

static const_buf_t
get_name(rotz_t cp, rtz_vtxkey_t svtx)
{
	const void *sp;
	int z[1];

	if (UNLIKELY((sp = tcbdbget3(cp->db, svtx, RTZ_VTXKEY_Z, z)) == NULL)) {
		return (const_buf_t){0U};
	}
	return (const_buf_t){.z = (size_t)*z, .d = sp};
}

static rtz_buf_t
get_name_r(rotz_t cp, rtz_vtxkey_t svtx)
{
	void *sp;
	int z[1];

	if (UNLIKELY((sp = tcbdbget(cp->db, svtx, RTZ_VTXKEY_Z, z)) == NULL)) {
		return (rtz_buf_t){0U};
	}
	return (rtz_buf_t){.z = (size_t)*z, .d = sp};
}

/* API */
rtz_vtx_t
rotz_get_vertex(rotz_t ctx, const char *v)
{
	return get_vertex(ctx, v, strlen(v));
}

rtz_vtx_t
rotz_add_vertex(rotz_t ctx, const char *v)
{
	size_t z = strlen(v);
	rtz_vtx_t res;

	/* first check if V is already there, if not get an id and add that */
	if ((res = get_vertex(ctx, v, z))) {
		;
	} else if (UNLIKELY(!(res = next_id(ctx)))) {
		;
	} else if (UNLIKELY(add_vertex(ctx, v, z, res) < 0)) {
		res = 0U;
	}
	return res;
}

rtz_vtx_t
rotz_rem_vertex(rotz_t ctx, const char *v)
{
	size_t z = strlen(v);
	rtz_vtx_t res;

	/* first check if V is really there, if not get an id and add that */
	if (UNLIKELY(!(res = get_vertex(ctx, v, z)))) {
		;
	} else if (UNLIKELY(rem_vertex(ctx, res, v, z) < 0)) {
		res = 0U;
	}
	return res;
}

const char*
rotz_get_name(rotz_t ctx, rtz_vtx_t v)
{
	static char *nmspc;
	static size_t nmspcz;
	rtz_vtxkey_t vkey = rtz_vtxkey(v);
	const_buf_t buf;

	if (UNLIKELY((buf = get_name(ctx, vkey)).d == NULL)) {
		return 0;
	}
	if (UNLIKELY(buf.z >= nmspcz)) {
		nmspcz = ((buf.z / 64U) + 1U) * 64U;
		nmspc = realloc(nmspc, nmspcz);
	}
	memcpy(nmspc, buf.d, buf.z);
	nmspc[buf.z] = '\0';
	return nmspc;
}

rtz_buf_t
rotz_get_name_r(rotz_t ctx, rtz_vtx_t v)
{
	rtz_vtxkey_t vkey = rtz_vtxkey(v);

	return get_name_r(ctx, vkey);
}

void
rotz_free_r(rtz_buf_t buf)
{
	if (LIKELY(buf.d != NULL)) {
		free(buf.d);
	}
	return;
}

int
rotz_add_alias(rotz_t ctx, rtz_vtx_t v, const char *alias)
{
	size_t aliaz = strlen(alias);
	const_buf_t al;
	rtz_vtxkey_t akey;
	rtz_vtx_t tmp;

	/* first check if V is already there, if not get an id and add that */
	if ((tmp = get_vertex(ctx, alias, aliaz)) && tmp != v) {
		/* alias points to a different vertex already
		 * we return -2 here to indicate this, so that callers that
		 * meant to combine 2 tags can use rotz_combine() */
		return -2;
	} else if (UNLIKELY(tmp)) {
		/* ah, tmp == v, don't bother putting it in again */
		;
	} else if (UNLIKELY(put_vertex(ctx, alias, aliaz, v) < 0)) {
		return -1;
	}
	/* check aliases */
	akey = rtz_akakey(v);
	if ((al = get_aliases(ctx, akey)).d != NULL &&
	    UNLIKELY(find_in_buf(al, alias, aliaz) != NULL)) {
		/* alias is already in the list */
		return 0;
	} else if (UNLIKELY(add_alias(ctx, akey, alias, aliaz) < 0)) {
		return -1;
	}
	return 1;
}

int
rotz_rem_alias(rotz_t ctx, const char *alias)
{
	size_t aliaz = strlen(alias);
	const_buf_t al;
	const char *ap;
	rtz_vtxkey_t akey;
	rtz_vtx_t aid;

	/* first check if V is actually there */
	if (!(aid = get_vertex(ctx, alias, aliaz))) {
		/* nothing to be done */
		return 0;
	}
	/* now remove that alias from the alias list */
	if ((al = get_aliases(ctx, akey = rtz_akakey(aid))).d == NULL ||
	    UNLIKELY((ap = find_in_buf(al, alias, aliaz)) == NULL)) {
		/* alias is already removed innit? */
		;
	} else if (UNLIKELY((al = rem_from_buf(al, ap, aliaz + 1)).d == NULL)) {
		/* huh? */
		return -1;
	} else {
		/* just reassing the list with the tag removed */
		add_akalst(ctx, akey, al);
	}
	unput_vertex(ctx, alias, aliaz);
	return 0;
}

rtz_buf_t
rotz_get_aliases(rotz_t ctx, rtz_vtx_t v)
{
	rtz_vtxkey_t akey = rtz_akakey(v);
	const_buf_t al;
	char *d;

	/* get edges under */
	if (UNLIKELY((al = get_aliases(ctx, akey)).d == NULL)) {
		return (rtz_buf_t){0U};
	}
	/* otherwise make a copy */
	{
		size_t mz = al.z * sizeof(*d);
		d = malloc(mz);
		memcpy(d, al.d, mz);
	}
	return (rtz_buf_t){.z = al.z, .d = d};
}


/* edge accessors */
typedef const unsigned char *rtz_edgkey_t;
#define RTZ_EDGPRE	"edg"
#define RTZ_EDGKEY_Z	(sizeof(RTZ_EDGPRE) + sizeof(rtz_vtx_t))

typedef struct {
	size_t z;
	const rtz_vtx_t *d;
} const_vtxlst_t;

static rtz_edgkey_t
rtz_edgkey(rtz_vtx_t vid)
{
/* return the key for the incidence list */
	static unsigned char edg[RTZ_EDGKEY_Z] = RTZ_EDGPRE;
	unsigned int *vi = (void*)(edg + sizeof(RTZ_EDGPRE));

	*vi = vid;
	return edg;
}

static __attribute__((unused)) rtz_vtx_t
rtz_edg(rtz_edgkey_t edg)
{
/* return the key for the incidence list */
	const unsigned int *vi = (const void*)(edg + sizeof(RTZ_EDGPRE));
	return *vi;
}

static const_vtxlst_t
get_edges(rotz_t ctx, rtz_edgkey_t src)
{
	const void *sp;
	int z[1];

	if (UNLIKELY((sp = tcbdbget3(ctx->db, src, RTZ_EDGKEY_Z, z)) == NULL)) {
		return (const_vtxlst_t){0U};
	}
	return (const_vtxlst_t){.z = (size_t)*z / sizeof(rtz_vtx_t), .d = sp};
}

static size_t
find_in_vtxlst(const_vtxlst_t el, rtz_vtx_t to)
{
	for (size_t i = 0; i < el.z; i++) {
		if (el.d[i] == to) {
			return i + 1U;
		}
	}
	return 0U;
}

static const_vtxlst_t
rem_from_vtxlst(const_vtxlst_t el, size_t idx)
{
	static rtz_vtx_t *edgspc;
	static size_t edgspz;
	rtz_vtx_t *ep;

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
	return (const_vtxlst_t){.z = ep - edgspc, .d = edgspc};
}

static int
add_edge(rotz_t ctx, rtz_edgkey_t src, rtz_vtx_t to)
{
	return tcbdbputcat(ctx->db, src, RTZ_EDGKEY_Z, &to, sizeof(to)) - 1;
}

static int
add_vtxlst(rotz_t ctx, rtz_edgkey_t src, const_vtxlst_t el)
{
	size_t z;

	if (UNLIKELY((z = el.z * sizeof(*el.d)) == 0U)) {
		return tcbdbout(ctx->db, src, RTZ_EDGKEY_Z) - 1;
	}
	return tcbdbput(ctx->db, src, RTZ_EDGKEY_Z, el.d, z) - 1;
}

static int
rem_edges(rotz_t ctx, rtz_edgkey_t src)
{
	return tcbdbout(ctx->db, src, RTZ_EDGKEY_Z) - 1;
}

/* API */
int
rotz_get_edge(rotz_t ctx, rtz_vtx_t from, rtz_vtx_t to)
{
	rtz_edgkey_t sfrom = rtz_edgkey(from);
	const_vtxlst_t el;

	/* get edges under */
	if (LIKELY((el = get_edges(ctx, sfrom)).d != NULL) &&
	    LIKELY(find_in_vtxlst(el, to) > 0U)) {
		/* to is already there */
		return 1;
	}
	return 0;
}

rtz_vtxlst_t
rotz_get_edges(rotz_t ctx, rtz_vtx_t from)
{
	rtz_edgkey_t sfrom = rtz_edgkey(from);
	const_vtxlst_t el;
	rtz_vtx_t *d;

	/* get edges under */
	if (UNLIKELY((el = get_edges(ctx, sfrom)).d == NULL)) {
		return (rtz_vtxlst_t){0U};
	}
	/* otherwise make a copy */
	{
		size_t mz = el.z * sizeof(*d);
		d = malloc(mz);
		memcpy(d, el.d, mz);
	}
	return (rtz_vtxlst_t){.z = el.z, .d = d};
}

int
rotz_rem_edges(rotz_t ctx, rtz_vtx_t from)
{
	rtz_edgkey_t sfrom = rtz_edgkey(from);

	return rem_edges(ctx, sfrom);
}

void
rotz_free_vtxlst(rtz_vtxlst_t el)
{
	if (LIKELY(el.d != NULL)) {
		free(el.d);
	}
	return;
}

int
rotz_add_edge(rotz_t ctx, rtz_vtx_t from, rtz_vtx_t to)
{
	rtz_edgkey_t sfrom = rtz_edgkey(from);
	const_vtxlst_t el;

	/* get edges under */
	if ((el = get_edges(ctx, sfrom)).d != NULL &&
	    UNLIKELY(find_in_vtxlst(el, to) > 0U)) {
		/* to is already there */
		return 0;
	}
	if (UNLIKELY(add_edge(ctx, sfrom, to) < 0)) {
		return -1;
	}
	return 1;
}

int
rotz_rem_edge(rotz_t ctx, rtz_vtx_t from, rtz_vtx_t to)
{
	rtz_edgkey_t sfrom = rtz_edgkey(from);
	const_vtxlst_t el;
	size_t idx;

	/* get edges under */
	if (UNLIKELY((el = get_edges(ctx, sfrom)).d == NULL) ||
	    UNLIKELY((idx = find_in_vtxlst(el, to)) == 0U)) {
		/* TO isn't in there */
		return 0;
	}
	if (UNLIKELY((el = rem_from_vtxlst(el, idx)).d == NULL)) {
		/* huh? */
		return -1;
	}
	add_vtxlst(ctx, sfrom, el);
	return 1;
}


/* iterators
 * we can't keep the promise here to separate keys and tokyocabinet guts */
void
rotz_vtx_iter(rotz_t ctx, void(*cb)(rtz_vtx_t, const char*, void*), void *clo)
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
		cb(vid, vp, clo);

	} while (tcbdbcurnext(c));

	tcbdbcurdel(c);
	return;
}

/* rotz.c ends here */

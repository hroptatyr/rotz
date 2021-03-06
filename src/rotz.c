/*** rotz.c -- master rotz api
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

#include "rotz.h"
#include "nifty.h"

#define const_buf_t	rtz_const_buf_t


/* our internal api */
typedef const unsigned char *rtz_vtxkey_t;
#define RTZ_VTXPRE	"vtx"
#define RTZ_VTXKEY_Z	(sizeof(RTZ_VTXPRE) + sizeof(rtz_vtx_t))

typedef const unsigned char *rtz_edgkey_t;
#define RTZ_EDGPRE	"edg"
#define RTZ_EDGKEY_Z	(sizeof(RTZ_EDGPRE) + sizeof(rtz_vtx_t))

#define const_vtxlst_t	rtz_const_vtxlst_t

static rtz_vtxkey_t rtz_vtxkey(rtz_vtx_t vid);
static rtz_vtx_t rtz_vtx(rtz_vtxkey_t x);

static rtz_vtx_t next_id(rotz_t cp);
static rtz_vtx_t get_vertex(rotz_t cp, const char *v, size_t z);
static int put_vertex(rotz_t cp, const char *a, size_t az, rtz_vtx_t v);
static int rnm_vertex(rotz_t cp, rtz_vtxkey_t vkey, const char *v, size_t z);
static int unput_vertex(rotz_t cp, const char *v, size_t z);
static int unrnm_vertex(rotz_t cp, rtz_vtxkey_t vkey);

static int add_alias(rotz_t cp, rtz_vtxkey_t vkey, const char *a, size_t az);
static int add_akalst(rotz_t ctx, rtz_vtxkey_t ak, const_buf_t al);
static const_buf_t get_aliases(rotz_t cp, rtz_vtxkey_t svtx);
static rtz_buf_t get_aliases_r(rotz_t cp, rtz_vtxkey_t svtx);

static rtz_edgkey_t rtz_edgkey(rtz_vtx_t vid);
static rtz_vtx_t rtz_edg(rtz_edgkey_t x);

static const_vtxlst_t get_edges(rotz_t ctx, rtz_edgkey_t src);
static int add_edge(rotz_t ctx, rtz_edgkey_t src, rtz_vtx_t to);
static int add_vtxlst(rotz_t ctx, rtz_edgkey_t src, const_vtxlst_t el);
static int rem_edges(rotz_t ctx, rtz_edgkey_t src);

#if defined USE_LMDB
# include "rotz-lmdb.c"
#elif defined USE_TCBDB
# include "rotz-tcbdb.c"
#else
# error need a database backend
#endif	/* USE_*DB */


/* vertex accessors
 * Our policy is that routines that know about tokyocabinet must not
 * know about vertex keys, aka keys, and so forth.
 * And vice versa.
 *
 * We maintain 2 lookups, NAME -> ID  and VTXKEY -> NAME(s)
 * where ID is of type rtz_vtx_t and VTXKEY is of type rtz_vtxkey_t
 * and they're converted to one another by rtz_vtxkey() and rtz_vtx()
 * respectively.
 *
 * In the mapping NAME(s) there's usually just one name but aliases
 * of the name will be appended there. */
static rtz_vtxkey_t
rtz_vtxkey(rtz_vtx_t vid)
{
/* return the key for the incidence list */
	static unsigned char vtx[RTZ_VTXKEY_Z] = RTZ_VTXPRE;
	unsigned int *vi = (void*)(vtx + sizeof(RTZ_VTXPRE));

	*vi = vid;
	return vtx;
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

static rtz_buf_t
get_name_r(rotz_t cp, rtz_vtxkey_t svtx)
{
	const_buf_t cb;

	if (UNLIKELY((cb = get_aliases(cp, svtx)).d == NULL)) {
		return (rtz_buf_t){0U};
	}
	/* we're interested in the first name only */
	cb.z = strlen(cb.d);
	return (rtz_buf_t){.z = cb.z, .d = strndup(cb.d, cb.z)};
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
	rtz_vtxkey_t vkey = rtz_vtxkey(i);
	const_buf_t al;
	int res = 0;

	/* get all them aliases */
	if (LIKELY((al = get_aliases(cp, vkey)).d != NULL)) {
		/* go through all names in the alias list */
		for (const char *x = al.d, *const ex = al.d + al.z;
		     x < ex; x += z + 1) {
			z = strlen(x);
			res += unput_vertex(cp, x, z);
		}
	} else {
		/* just to be sure */
		res += unput_vertex(cp, v, z);
	}
	res += unrnm_vertex(cp, rtz_vtxkey(i));
	return res;
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

	if (UNLIKELY((buf = get_aliases(ctx, vkey)).d == NULL)) {
		return 0;
	}
	/* we're interested in the first name only */
	buf.z = strlen(buf.d);
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
	return get_name_r(ctx, rtz_vtxkey(v));
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
	if ((al = get_aliases(ctx, akey = rtz_vtxkey(v))).d != NULL &&
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
	if ((al = get_aliases(ctx, akey = rtz_vtxkey(aid))).d == NULL ||
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
	return get_aliases_r(ctx, rtz_vtxkey(v));
}


/* edge accessors */
static rtz_edgkey_t
rtz_edgkey(rtz_vtx_t vid)
{
/* return the key for the incidence list */
	static unsigned char edg[RTZ_EDGKEY_Z] = RTZ_EDGPRE;
	unsigned int *vi = (void*)(edg + sizeof(RTZ_EDGPRE));

	*vi = vid;
	return edg;
}

static rtz_vtx_t
rtz_edg(rtz_edgkey_t edg)
{
/* return the key for the incidence list */
	const unsigned int *vi = (const void*)(edg + sizeof(RTZ_EDGPRE));
	return *vi;
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

size_t
rotz_get_nedges(rotz_t ctx, rtz_vtx_t from)
{
	rtz_edgkey_t sfrom = rtz_edgkey(from);

	return get_edges(ctx, sfrom).z;
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

void
rotz_free_wtxlst(rtz_wtxlst_t wl)
{
	if (LIKELY(wl.d != NULL)) {
		free(wl.d);
	}
	if (LIKELY(wl.w != NULL)) {
		free(wl.w);
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


/* set opers */
static rtz_vtxlst_t
add_to_vtxlst(rtz_vtxlst_t el, rtz_vtx_t v)
{
	if (UNLIKELY(!(el.z % 64U))) {
		size_t nu = (el.z + 64U) * sizeof(*el.d);

		el.d = realloc(el.d, nu);
		memset(el.d + el.z, 0, 64U * sizeof(*el.d));
	}
	el.d[el.z++] = v;
	return el;
}

static rtz_wtxlst_t
add_to_wtxlst(rtz_wtxlst_t wl, rtz_vtx_t v)
{
	if (UNLIKELY(!(wl.z % 64U))) {
		size_t nu;

		nu = (wl.z + 64U) * sizeof(*wl.d);
		wl.d = realloc(wl.d, nu);
		memset(wl.d + wl.z, 0, 64U * sizeof(*wl.d));

		nu = (wl.z + 64U) * sizeof(*wl.w);
		wl.w = realloc(wl.w, nu);
		memset(wl.w + wl.z, 0, 64U * sizeof(*wl.w));
	}
	wl.d[wl.z++] = v;
	return wl;
}

static rtz_vtxlst_t
vtxlst_union(rtz_vtxlst_t tgt, const_vtxlst_t el)
{
	const size_t tgtz = tgt.z;

	for (size_t i = 0; i < el.z; i++) {
		rtz_vtx_t item = el.d[i];

		/* got this one? */
		if (find_in_vtxlst((const_vtxlst_t){tgtz, tgt.d}, item)) {
			continue;
		}
		tgt = add_to_vtxlst(tgt, item);
	}
	return tgt;
}

static rtz_wtxlst_t
wtxlst_union(rtz_wtxlst_t tgt, const_vtxlst_t el)
{
	const size_t tgtz = tgt.z;

	for (size_t i = 0; i < el.z; i++) {
		rtz_vtx_t item = el.d[i];
		size_t p;

		/* got this one? */
		if ((p = find_in_vtxlst((const_vtxlst_t){tgtz, tgt.d}, item))) {
			/* add 1 in the weight vector then */
			tgt.w[p - 1]++;
			continue;
		}
		tgt = add_to_wtxlst(tgt, item);
	}
	return tgt;
}

static rtz_vtxlst_t
vtxlst_intersection(rtz_vtxlst_t tgt, const_vtxlst_t el)
{
	for (size_t i = 0; i < tgt.z; i++) {
		/* got this one? */
		if (find_in_vtxlst(el, tgt.d[i])) {
			/* item can stay in tgt */
			continue;
		}
		/* item must go, mark it for removal */
		tgt.d[i] = 0U;
	}
	for (rtz_vtx_t *ip = tgt.d, *jp = tgt.d, *const ep = ip + tgt.z;
	     ip < ep; ip++) {
		if (*ip) {
			if (LIKELY(jp < ip)) {
				*jp = *ip;
			}
			jp++;
		} else {
			/* remove the item by decr'ing the tgt item counter */
			tgt.z--;
		}
	}
	return tgt;
}

rtz_vtxlst_t
rotz_union(rotz_t cp, rtz_vtxlst_t x, rtz_vtx_t v)
{
	rtz_edgkey_t vkey = rtz_edgkey(v);
	const_vtxlst_t el;

	if (UNLIKELY((el = get_edges(cp, vkey)).d == NULL)) {
		return x;
	}
	/* just add them one by one */
	return vtxlst_union(x, el);
}

rtz_vtxlst_t
rotz_intersection(rotz_t cp, rtz_vtxlst_t x, rtz_vtx_t v)
{
	rtz_edgkey_t vkey = rtz_edgkey(v);
	const_vtxlst_t el;

	if (UNLIKELY((el = get_edges(cp, vkey)).d == NULL)) {
		return x;
	}
	/* just add them one by one */
	return vtxlst_intersection(x, el);
}

rtz_wtxlst_t
rotz_munion(rotz_t cp, rtz_wtxlst_t x, rtz_vtx_t v)
{
	rtz_edgkey_t vkey = rtz_edgkey(v);
	const_vtxlst_t el;

	if (UNLIKELY((el = get_edges(cp, vkey)).d == NULL)) {
		return x;
	}
	/* just add them one by one */
	return wtxlst_union(x, el);
}

/* rotz.c ends here */

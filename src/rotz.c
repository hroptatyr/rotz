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
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#if defined USE_LMDB
# include <lmdb.h>
#elif defined USE_TCBDB
# include <tcbdb.h>
#else
# error need a database backend
#endif	/* USE_*DB */

#include "rotz.h"
#include "nifty.h"

struct rotz_s {
#if defined USE_LMDB
	MDB_env *db;
	MDB_dbi dbi;
#elif defined USE_TCBDB
	TCBDB *db;
#endif	/* USE_*DB */
};


/* low level graph lib */
rotz_t
make_rotz(const char *db, ...)
{
#if defined USE_LMDB
	va_list ap;
	int omode = MDB_RDONLY | MDB_NOTLS | MDB_NOSUBDIR;
	int dmode = 0;
	int oparam;
	struct rotz_s res;
	MDB_txn *txn;

	va_start(ap, db);
	oparam = va_arg(ap, int);
	va_end(ap);

	if (oparam & O_RDWR) {
		omode &= ~MDB_RDONLY;
		omode |= MDB_WRITEMAP | MDB_MAPASYNC;
	}
	if (oparam & O_CREAT) {
		dmode |= MDB_CREATE;
	}

	if (UNLIKELY(mdb_env_create(&res.db) != 0)) {
		goto out0;
	} else if (UNLIKELY(mdb_env_open(res.db, db, omode, 0644) != 0)) {
		goto out1;
	} else if (UNLIKELY(mdb_txn_begin(res.db, NULL, MDB_RDONLY, &txn) != 0)) {
		goto out2;
	} else if (UNLIKELY(mdb_dbi_open(txn, NULL, dmode, &res.dbi) != 0)) {
		goto out3;
	}
	/* just finalise the transaction now */
	mdb_txn_abort(txn);

	/* clone the result */
	{
		struct rotz_s *resp = malloc(sizeof(*resp));
		*resp = res;
		return resp;
	}

out3:
	mdb_txn_abort(txn);
	mdb_close(res.db, res.dbi);
out2:
out1:
	mdb_env_close(res.db);
out0:
	return NULL;

#elif defined USE_TCBDB
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
#endif	/* USE_*DB */
}

void
free_rotz(rotz_t ctx)
{
#if defined USE_LMDB
	mdb_close(ctx->db, ctx->dbi);
	mdb_env_sync(ctx->db, 1/*force synchronous*/);
	mdb_env_close(ctx->db);
#elif defined USE_TCBDB
	tcbdbclose(ctx->db);
	tcbdbdel(ctx->db);
#endif	/* USE_*DB */
	free(ctx);
	return;
}


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
typedef const unsigned char *rtz_vtxkey_t;
#define RTZ_VTXPRE	"vtx"
#define RTZ_VTXKEY_Z	(sizeof(RTZ_VTXPRE) + sizeof(rtz_vtx_t))

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

#if defined USE_LMDB
	MDB_val key = {
		.mv_size = sizeof(nid),
		.mv_data = nid,
	};
	MDB_txn *txn;
	MDB_val val;
	rtz_vtx_t res = 0U;

	mdb_txn_begin(cp->db, NULL, 0, &txn);
	switch (mdb_get(txn, cp->dbi, &key, &val)) {
	default:
		res = 0U;
		break;
	case 0:
		res = *(const rtz_vtx_t*)val.mv_data;
	case MDB_NOTFOUND:
		res++;
		val.mv_data = &res;
		val.mv_size = sizeof(res);

		/* put back into the db */
		mdb_put(txn, cp->dbi, &key, &val, 0);
		break;
	}
	/* and commit */
	mdb_txn_commit(txn);

#elif defined USE_TCBDB
	int res;

	if (UNLIKELY((res = tcbdbaddint(cp->db, nid, sizeof(nid), 1)) <= 0)) {
		return 0U;
	}
#endif	/* USE_*DB */
	return (rtz_vtx_t)res;
}

static rtz_vtx_t
get_vertex(rotz_t cp, const char *v, size_t z)
{
	rtz_vtx_t res;

#if defined USE_LMDB
	MDB_val key = {
		.mv_size = z,
		.mv_data = v,
	};
	MDB_txn *txn;
	MDB_val val;

	/* get us a transaction */
	mdb_txn_begin(cp->db, NULL, 0, &txn);

	if (UNLIKELY(mdb_get(txn, cp->dbi, &key, &val) != 0)) {
		res = 0U;
	} else if (UNLIKELY(val.mv_size != sizeof(res))) {
		res = 0U;
	} else {
		res = *(const rtz_vtx_t*)val.mv_data;
	}

	/* and commit */
	mdb_txn_commit(txn);

#elif defined USE_TCBDB
	const int *rp;
	int rz[1];

	if (UNLIKELY((rp = tcbdbget3(cp->db, v, z, rz)) == NULL)) {
		return 0U;
	} else if (UNLIKELY(*rz != sizeof(*rp))) {
		return 0U;
	}
	res = (rtz_vtx_t)*rp;
#endif	/* USE_*DB */

	return res;
}

static int
put_vertex(rotz_t cp, const char *a, size_t az, rtz_vtx_t v)
{
	int res = 0;

#if defined USE_LMDB
	MDB_txn *txn;
	MDB_val key = {
		.mv_size = az,
		.mv_data = a,
	};
	MDB_val val = {
		.mv_size = sizeof(v),
		.mv_data = &v,
	};

	/* get us a transaction */
	mdb_txn_begin(cp->db, NULL, 0, &txn);

	if (UNLIKELY(mdb_put(txn, cp->dbi, &key, &val, 0) != 0)) {
		res = -1;
	}

	/* and commit */
	mdb_txn_commit(txn);

#elif defined USE_TCBDB
	res = tcbdbaddint(cp->db, a, az, (int)v) - 1;
#endif	/* USE_*DB */

	return res;
}

static int
rnm_vertex(rotz_t cp, rtz_vtxkey_t vkey, const char *v, size_t z)
{
	int res = 0;

#if defined USE_LMDB
	MDB_val key = {
		.mv_size = RTZ_VTXKEY_Z,
		.mv_data = vkey,
	};
	MDB_val val = {
		.mv_size = z + 1,
		.mv_data= v,
	};
	MDB_txn *txn;

	/* get us a transaction */
	mdb_txn_begin(cp->db, NULL, 0, &txn);

	if (UNLIKELY(mdb_put(txn, cp->dbi, &key, &val, 0) != 0)) {
		key = (MDB_val){z, v};
		mdb_del(txn, cp->dbi, &val, NULL);
		res = -1;
	}

	/* and commit */
	mdb_txn_commit(txn);

#elif defined USE_TCBDB
	if (UNLIKELY(!tcbdbput(cp->db, vkey, RTZ_VTXKEY_Z, v, z + 1))) {
		tcbdbout(cp->db, v, z);
		res = -1;
	}
#endif	/* USE_*DB */

	return res;
}

static int
unput_vertex(rotz_t cp, const char *v, size_t z)
{
	int res = 0;

#if defined USE_LMDB
	MDB_val key = {
		.mv_size = z,
		.mv_data = v,
	};
	MDB_txn *txn;

	/* get us a transaction */
	mdb_txn_begin(cp->db, NULL, 0, &txn);

	if (UNLIKELY(mdb_del(txn, cp->dbi, &key, NULL) != 0)) {
		res = -1;
	}

	/* and commit */
	mdb_txn_commit(txn);

#elif defined USE_TCBDB
	res = tcbdbout(cp->db, v, z) - 1;
#endif	/* USE_*DB */

	return res;
}

static int
unrnm_vertex(rotz_t cp, rtz_vtxkey_t vkey)
{
	int res = 0;

#if defined USE_LMDB
	MDB_val key = {
		.mv_size = RTZ_VTXKEY_Z,
		.mv_data = vkey,
	};
	MDB_txn *txn;

	/* get us a transaction */
	mdb_txn_begin(cp->db, NULL, 0, &txn);

	if (UNLIKELY(mdb_del(txn, cp->dbi, &key, NULL) != 0)) {
		res = -1;
	}

	/* and commit */
	mdb_txn_commit(txn);

#elif defined USE_TCBDB
	res = tcbdbout(cp->db, vkey, RTZ_VTXKEY_Z) - 1;
#endif	/* USE_*DB */

	return res;
}

#if defined USE_LMDB
static int
mdb_putcat(MDB_txn *txn, MDB_dbi dbi, MDB_val *key, MDB_val *data)
{
	MDB_val getval = {0U};
	MDB_val putval;

	switch (mdb_get(txn, dbi, key, &getval)) {
	default:
		return -1;
	case MDB_NOTFOUND:
		/* put mode */
		break;
	case 0:
		/* append mode */
		break;
	}

	putval = (MDB_val){.mv_size = getval.mv_size + data->mv_size, NULL};

	/* now put it back in the pool */
	if (mdb_put(txn, dbi, key, &putval, MDB_RESERVE) != 0) {
		return -1;
	}

	with (char *restrict pp = putval.mv_data) {
		/* copy the original data first */
		memcpy(pp, getval.mv_data, getval.mv_size);
		/* append the new guy */
		memcpy(pp + getval.mv_size, data->mv_data, data->mv_size);
	}
	return 0;
}
#endif	/* USE_LMDB */

static int
add_alias(rotz_t cp, rtz_vtxkey_t vkey, const char *a, size_t az)
{
	int res = 0;

#if defined USE_LMDB
	MDB_val key = {
		.mv_size = RTZ_VTXKEY_Z,
		.mv_data = vkey,
	};
	MDB_val val = {
		.mv_size = az + 1,
		.mv_data = a,
	};
	MDB_txn *txn;

	/* get us a transaction */
	mdb_txn_begin(cp->db, NULL, 0, &txn);

	if (mdb_putcat(txn, cp->dbi, &key, &val) != 0) {
		res = -1;
	}

	/* and commit */
	mdb_txn_commit(txn);

#elif defined USE_TCBDB
	res = tcbdbputcat(cp->db, vkey, RTZ_VTXKEY_Z, a, az + 1) - 1;
#endif	/* USE_*DB */

	return res;
}

static int
add_akalst(rotz_t ctx, rtz_vtxkey_t ak, const_buf_t al)
{
	int res = 0;

#if defined USE_LMDB
	MDB_val key = {
		.mv_size = RTZ_VTXKEY_Z,
		.mv_data = ak,
	};
	MDB_val val = {
		.mv_size = al.z * sizeof(*al.d),
		.mv_data = al.d,
	};
	MDB_txn *txn;

	/* get us a transaction */
	mdb_txn_begin(ctx->db, NULL, 0, &txn);

	/* first delete the old guy */
	if (mdb_del(txn, ctx->dbi, &key, NULL) != 0) {
		/* ok, we're fucked */
		res = -1;
	} else if (UNLIKELY(val.mv_size == 0U)) {
		/* leave it del'd */
		;
	} else if (mdb_put(txn, ctx->dbi, &key, &val, 0) != 0) {
		/* putting the new list failed */
		res = -1;
	}

	/* and commit */
	mdb_txn_commit(txn);

#elif defined USE_TCBDB
	size_t z;

	if (UNLIKELY((z = al.z * sizeof(*al.d)) == 0U)) {
		return tcbdbout(ctx->db, ak, RTZ_VTXKEY_Z) - 1;
	}
	res = tcbdbput(ctx->db, ak, RTZ_VTXKEY_Z, al.d, z) - 1;
#endif	/* USE_*DB */

	return res;
}

static const_buf_t
get_aliases(rotz_t cp, rtz_vtxkey_t svtx)
{
	const_buf_t res;

#if defined USE_LMDB
	MDB_val key = {
		.mv_size = RTZ_VTXKEY_Z,
		.mv_data = svtx,
	};
	MDB_val val;
	MDB_txn *txn;

	/* get us a transaction */
	mdb_txn_begin(cp->db, NULL, 0, &txn);

	if (UNLIKELY(mdb_get(txn, cp->dbi, &key, &val) < 0)) {
		res = (const_buf_t){0U};
	} else {
		res = (const_buf_t){.z = val.mv_size, .d = val.mv_data};
	}

	/* and commit */
	mdb_txn_commit(txn);

#elif defined USE_TCBDB
	const void *sp;
	int z[1];

	if (UNLIKELY((sp = tcbdbget3(cp->db, svtx, RTZ_VTXKEY_Z, z)) == NULL)) {
		return (const_buf_t){0U};
	}

	res = (const_buf_t){.z = (size_t)*z, .d = sp};
#endif	/* USE_*DB */

	return res;
}

static rtz_buf_t
get_aliases_r(rotz_t cp, rtz_vtxkey_t svtx)
{
	rtz_buf_t res;

#if defined USE_LMDB
	const_buf_t tmp = get_aliases(cp, svtx);

	res.d = malloc(res.z = tmp.z);
	memcpy(res.d, tmp.d, tmp.z);
#elif defined USE_TCBDB
	void *sp;
	int z[1];

	if (UNLIKELY((sp = tcbdbget(cp->db, svtx, RTZ_VTXKEY_Z, z)) == NULL)) {
		return (rtz_buf_t){0U};
	}
	res = (rtz_buf_t){.z = (size_t)*z, .d = sp};
#endif	/* USE_*DB */

	return res;
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
typedef const unsigned char *rtz_edgkey_t;
#define RTZ_EDGPRE	"edg"
#define RTZ_EDGKEY_Z	(sizeof(RTZ_EDGPRE) + sizeof(rtz_vtx_t))

#define const_vtxlst_t	rtz_const_vtxlst_t

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

static const_vtxlst_t
get_edges(rotz_t ctx, rtz_edgkey_t src)
{
	const_vtxlst_t res;

#if defined USE_LMDB
	MDB_val key = {
		.mv_size = RTZ_EDGKEY_Z,
		.mv_data = src,
	};
	MDB_val val;
	MDB_txn *txn;

	/* get us a transaction */
	mdb_txn_begin(ctx->db, NULL, 0, &txn);

	if (UNLIKELY(mdb_get(txn, ctx->dbi, &key, &val) < 0)) {
		res = (const_vtxlst_t){0U};
	} else {
		res = (const_vtxlst_t){.z = val.mv_size, .d = val.mv_data};
	}

	/* and commit */
	mdb_txn_commit(txn);

#elif defined USE_TCBDB
	const void *sp;
	int z[1];

	if (UNLIKELY((sp = tcbdbget3(ctx->db, src, RTZ_EDGKEY_Z, z)) == NULL)) {
		return (const_vtxlst_t){0U};
	}
	res = (const_vtxlst_t){.z = (size_t)*z / sizeof(rtz_vtx_t), .d = sp};
#endif	/* USE_*DB */

	return res;
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
	int res = 0;

#if defined USE_LMDB
	MDB_val key = {
		.mv_size = RTZ_EDGKEY_Z,
		.mv_data = src,
	};
	MDB_val val = {
		.mv_size = sizeof(to),
		.mv_data = &to,
	};
	MDB_txn *txn;

	/* get us a transaction */
	mdb_txn_begin(ctx->db, NULL, 0, &txn);

	if (mdb_putcat(txn, ctx->dbi, &key, &val) != 0) {
		res = -1;
	}

	/* and commit */
	mdb_txn_commit(txn);

#elif defined USE_TCBDB
	res = tcbdbputcat(ctx->db, src, RTZ_EDGKEY_Z, &to, sizeof(to)) - 1;
#endif	/* USE_*DB */

	return res;
}

static int
add_vtxlst(rotz_t ctx, rtz_edgkey_t src, const_vtxlst_t el)
{
	int res = 0;

#if defined USE_LMDB
	MDB_val key = {
		.mv_size = RTZ_EDGKEY_Z,
		.mv_data = src,
	};
	MDB_val val = {
		.mv_size = el.z,
		.mv_data = el.d,
	};
	MDB_txn *txn;

	/* get us a transaction */
	mdb_txn_begin(ctx->db, NULL, 0, &txn);

	/* first delete the old guy */
	if (mdb_del(txn, ctx->dbi, &key, NULL) != 0) {
		/* ok, we're fucked */
		res = -1;
	} else if (UNLIKELY(val.mv_size == 0U)) {
		/* leave it del'd */
		;
	} else if (mdb_put(txn, ctx->dbi, &key, &val, 0) != 0) {
		/* putting the new list failed */
		res = -1;
	}

	/* and commit */
	mdb_txn_commit(txn);

#elif defined USE_TCBDB
	size_t z;

	if (UNLIKELY((z = el.z * sizeof(*el.d)) == 0U)) {
		return tcbdbout(ctx->db, src, RTZ_EDGKEY_Z) - 1;
	}
	res = tcbdbput(ctx->db, src, RTZ_EDGKEY_Z, el.d, z) - 1;
#endif	/* USE_*DB */

	return res;
}

static int
rem_edges(rotz_t ctx, rtz_edgkey_t src)
{
	int res = 0;

#if defined USE_LMDB
	MDB_val key = {
		.mv_size = RTZ_EDGKEY_Z,
		.mv_data = src,
	};
	MDB_txn *txn;

	/* get us a transaction */
	mdb_txn_begin(ctx->db, NULL, 0, &txn);

	if (UNLIKELY(mdb_del(txn, ctx->dbi, &key, NULL) != 0)) {
		res = -1;
	}

	/* and commit */
	mdb_txn_commit(txn);

#elif defined USE_TCBDB
	res = tcbdbout(ctx->db, src, RTZ_EDGKEY_Z) - 1;
#endif	/* USE_*DB */

	return res;
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
			tgt.z = ++jp - tgt.d;
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

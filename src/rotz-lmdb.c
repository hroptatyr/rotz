/*** rotz-lmdb.c -- lmdb guts
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
#include <lmdb.h>

#include "rotz.h"
#include "nifty.h"

struct rotz_s {
	MDB_env *db;
	MDB_dbi dbi;
};


/* low level graph lib */
rotz_t
make_rotz(const char *db, ...)
{
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
	mdb_dbi_close(res.db, res.dbi);
out2:
out1:
	mdb_env_close(res.db);
out0:
	return NULL;
}

void
free_rotz(rotz_t ctx)
{
	mdb_close(ctx->db, ctx->dbi);
	mdb_env_sync(ctx->db, 1/*force synchronous*/);
	mdb_env_close(ctx->db);
	free(ctx);
	return;
}


/* vertex accessors */
static rtz_vtx_t
next_id(rotz_t cp)
{
	static const char nid[] = "\x1d";

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
	return (rtz_vtx_t)res;
}

static rtz_vtx_t
get_vertex(rotz_t cp, const char *v, size_t z)
{
	rtz_vtx_t res;
	MDB_val key = {
		.mv_size = z,
		.mv_data = v,
	};
	MDB_txn *txn;
	MDB_val val;

	/* get us a transaction */
	mdb_txn_begin(cp->db, NULL, MDB_RDONLY, &txn);

	if (UNLIKELY(mdb_get(txn, cp->dbi, &key, &val) != 0)) {
		res = 0U;
	} else if (UNLIKELY(val.mv_size != sizeof(res))) {
		res = 0U;
	} else {
		res = *(const rtz_vtx_t*)val.mv_data;
	}

	/* and commit */
	mdb_txn_commit(txn);
	return res;
}

static int
put_vertex(rotz_t cp, const char *a, size_t az, rtz_vtx_t v)
{
	int res = 0;
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
	return res;
}

static int
rnm_vertex(rotz_t cp, rtz_vtxkey_t vkey, const char *v, size_t z)
{
	int res = 0;
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
	return res;
}

static int
unput_vertex(rotz_t cp, const char *v, size_t z)
{
	int res = 0;
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
	return res;
}

static int
unrnm_vertex(rotz_t cp, rtz_vtxkey_t vkey)
{
	int res = 0;
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
	return res;
}

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

static int
add_alias(rotz_t cp, rtz_vtxkey_t vkey, const char *a, size_t az)
{
	int res = 0;
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
	return res;
}

static int
add_akalst(rotz_t ctx, rtz_vtxkey_t ak, const_buf_t al)
{
	int res = 0;
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
	return res;
}

static const_buf_t
get_aliases(rotz_t cp, rtz_vtxkey_t svtx)
{
	const_buf_t res;
	MDB_val key = {
		.mv_size = RTZ_VTXKEY_Z,
		.mv_data = svtx,
	};
	MDB_val val;
	MDB_txn *txn;

	/* get us a transaction */
	mdb_txn_begin(cp->db, NULL, MDB_RDONLY, &txn);

	if (UNLIKELY(mdb_get(txn, cp->dbi, &key, &val) < 0)) {
		res = (const_buf_t){0U};
	} else {
		res = (const_buf_t){.z = val.mv_size, .d = val.mv_data};
	}

	/* and commit */
	mdb_txn_commit(txn);
	return res;
}

static rtz_buf_t
get_aliases_r(rotz_t cp, rtz_vtxkey_t svtx)
{
	rtz_buf_t res;
	const_buf_t tmp = get_aliases(cp, svtx);

	res.d = malloc(res.z = tmp.z);
	memcpy(res.d, tmp.d, tmp.z);
	return res;
}


/* edge accessors */
static const_vtxlst_t
get_edges(rotz_t ctx, rtz_edgkey_t src)
{
	const_vtxlst_t res;
	MDB_val key = {
		.mv_size = RTZ_EDGKEY_Z,
		.mv_data = src,
	};
	MDB_val val;
	MDB_txn *txn;

	/* get us a transaction */
	mdb_txn_begin(ctx->db, NULL, MDB_RDONLY, &txn);

	if (UNLIKELY(mdb_get(txn, ctx->dbi, &key, &val) < 0)) {
		res = (const_vtxlst_t){0U};
	} else {
		res = (const_vtxlst_t){
			.z = val.mv_size / sizeof(rtz_vtx_t),
			.d = val.mv_data
		};
	}

	/* and commit */
	mdb_txn_commit(txn);
	return res;
}

static int
add_edge(rotz_t ctx, rtz_edgkey_t src, rtz_vtx_t to)
{
	int res = 0;
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
	return res;
}

static int
add_vtxlst(rotz_t ctx, rtz_edgkey_t src, const_vtxlst_t el)
{
	int res = 0;
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
	return res;
}

static int
rem_edges(rotz_t ctx, rtz_edgkey_t src)
{
	int res = 0;
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
	return res;
}


/* iterators */
void
rotz_vtx_iter(rotz_t ctx, int(*cb)(rtz_vtx_t, const char*, void*), void *clo)
{
	MDB_txn *txn;
	MDB_cursor *crs;
	MDB_val key = {
		.mv_size = sizeof(RTZ_VTXPRE),
		.mv_data = RTZ_VTXPRE,
	};
	MDB_val val;

	/* get us a transaction and a cursor */
	mdb_txn_begin(ctx->db, NULL, MDB_RDONLY, &txn);
	if (mdb_cursor_open(txn, ctx->dbi, &crs) != 0) {
		goto out0;
	} else if (mdb_cursor_get(crs, &key, NULL, MDB_SET_RANGE) != 0) {
		goto out1;
	} else if (mdb_cursor_get(crs, &key, &val, MDB_GET_CURRENT) != 0) {
		goto out2;
	}
	do {
		rtz_vtx_t vid;

		if (UNLIKELY(key.mv_size != RTZ_VTXKEY_Z) ||
		    UNLIKELY(!(vid = rtz_vtx(key.mv_data)))) {
			break;
		}
		/* otherwise just call the callback */
		if (UNLIKELY(cb(vid, val.mv_data, clo) < 0)) {
			break;
		}
	} while (mdb_cursor_get(crs, &key, &val, MDB_NEXT) == 0);

out2:
out1:
	/* cursor finalising */
	mdb_cursor_close(crs);
out0:
	/* and out */
	mdb_txn_abort(txn);
	return;
}

void
rotz_edg_iter(rotz_t ctx, int(*cb)(rtz_vtx_t, const_vtxlst_t, void*), void *clo)
{
	MDB_txn *txn;
	MDB_cursor *crs;
	MDB_val key = {
		.mv_size = sizeof(RTZ_EDGPRE),
		.mv_data = RTZ_EDGPRE,
	};
	MDB_val val;

	/* get us a transaction and a cursor */
	mdb_txn_begin(ctx->db, NULL, MDB_RDONLY, &txn);
	if (mdb_cursor_open(txn, ctx->dbi, &crs) != 0) {
		goto out0;
	} else if (mdb_cursor_get(crs, &key, NULL, MDB_SET_RANGE) != 0) {
		goto out1;
	} else if (mdb_cursor_get(crs, &key, &val, MDB_GET_CURRENT) != 0) {
		goto out2;
	}
	do {
		rtz_vtx_t eid;
		const_vtxlst_t cvl;

		if (UNLIKELY(key.mv_size != RTZ_EDGKEY_Z) ||
		    UNLIKELY(!(eid = rtz_edg(key.mv_data)))) {
			break;
		}
		/* ctor the vl */
		cvl = (const_vtxlst_t){
			.z = val.mv_size / sizeof(*cvl.d),
			.d = val.mv_data,
		};
		/* otherwise just call the callback */
		if (UNLIKELY(cb(eid, cvl, clo) < 0)) {
			break;
		}
	} while (mdb_cursor_get(crs, &key, &val, MDB_NEXT) == 0);

out2:
out1:
	/* cursor finalising */
	mdb_cursor_close(crs);
out0:
	/* and out */
	mdb_txn_abort(txn);
	return;
}

void
rotz_iter(
	rotz_t ctx, rtz_const_buf_t prfx_match,
	int(*cb)(rtz_const_buf_t key, rtz_const_buf_t val, void*), void *clo)
{
	MDB_txn *txn;
	MDB_cursor *crs;
	MDB_val key = {
		.mv_size = prfx_match.z,
		.mv_data = prfx_match.d,
	};
	MDB_val val;

	/* get us a transaction and a cursor */
	mdb_txn_begin(ctx->db, NULL, MDB_RDONLY, &txn);
	if (mdb_cursor_open(txn, ctx->dbi, &crs) != 0) {
		goto out0;
	} else if (mdb_cursor_get(crs, &key, NULL, MDB_SET_RANGE) != 0) {
		goto out1;
	} else if (mdb_cursor_get(crs, &key, &val, MDB_GET_CURRENT) != 0) {
		goto out2;
	}

#define B	(const_buf_t)
	do {
		const_buf_t kb;
		const_buf_t vb;

		if (UNLIKELY(memcmp(key.mv_data, prfx_match.d, prfx_match.z))) {
			break;
		}
		/* otherwise pack the bufs and call the callback */
		kb = B{key.mv_size, key.mv_data};
		vb = B{val.mv_size, val.mv_data};
		if (UNLIKELY(cb(kb, vb, clo) < 0)) {
			break;
		}
	} while (mdb_cursor_get(crs, &key, &val, MDB_NEXT) == 0);
#undef B

out2:
out1:
	/* cursor finalising */
	mdb_cursor_close(crs);
out0:
	/* and out */
	mdb_txn_abort(txn);
	return;
}

/* rotz-lmdb.c ends here */

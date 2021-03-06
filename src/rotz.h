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

typedef struct rotz_s *restrict rotz_t;
typedef unsigned int rtz_vtx_t;

typedef struct {
	size_t z;
	rtz_vtx_t *d;
} rtz_vtxlst_t;

typedef struct {
	size_t z;
	const rtz_vtx_t *d;
} rtz_const_vtxlst_t;

typedef struct {
	size_t z;
	rtz_vtx_t *d;
	unsigned int *w;
} rtz_wtxlst_t;

typedef struct {
	size_t z;
	const rtz_vtx_t *d;
	unsigned int *w;
} rtz_const_wtxlst_t;

typedef struct {
	size_t z;
	char *d;
} rtz_buf_t;

typedef struct {
	size_t z;
	const char *d;
} rtz_const_buf_t;


/* lower level graph api */
extern rotz_t make_rotz(const char *dbfile, ...);
extern void free_rotz(rotz_t);

/**
 * Return object handle for vertex V. */
extern rtz_vtx_t rotz_get_vertex(rotz_t, const char *v);

/**
 * Add vertex V to rotz database file and return its object handle. */
extern rtz_vtx_t rotz_add_vertex(rotz_t, const char *v);

/**
 * Remove vertex V from rotz database file and return its object handle. */
extern rtz_vtx_t rotz_rem_vertex(rotz_t, const char *v);

/**
 * Retrieve the name used to identify the vertex V.
 * The buffer returned resides in static space.  A second call to this
 * routine might overwrite the buffer's contents. */
extern const char *rotz_get_name(rotz_t, rtz_vtx_t v);

/**
 * Retrieve the name used to identify the vertex V safely, i.e.
 * a buffer is allocated for the contents and put into DEST.
 * The buffer's size in bytes is returned.
 * The buffer can be freed with `rotz_free_r()'. */
extern rtz_buf_t rotz_get_name_r(rotz_t, rtz_vtx_t v);

/**
 * Releases resources for buffers generically. */
extern void rotz_free_r(rtz_buf_t);

/**
 * Make ALIAS an alias for V. */
extern int rotz_add_alias(rotz_t, rtz_vtx_t v, const char *alias);

/**
 * Remove ALIAS from the list of aliases (associated with ALIAS). */
extern int rotz_rem_alias(rotz_t, const char *alias);

/**
 * Return the list of aliases of V. */
extern rtz_buf_t rotz_get_aliases(rotz_t, rtz_vtx_t v);


/**
 * Return >0 iff the edge from vertex FROM to vertex TO exists. */
extern int rotz_get_edge(rotz_t, rtz_vtx_t from, rtz_vtx_t to);

/**
 * Return (outgoing) edges from a vertex VID. */
extern rtz_vtxlst_t rotz_get_edges(rotz_t, rtz_vtx_t vid);

/**
 * Return the number of (outgoing) edges from a vertex VID. */
extern size_t rotz_get_nedges(rotz_t, rtz_vtx_t vid);

/**
 * Delete (outgoing) edges from a vertex VID. */
extern int rotz_rem_edges(rotz_t, rtz_vtx_t vid);

/**
 * Free up edge list resources as returned by `rotz_get_edges()'. */
extern void rotz_free_vtxlst(rtz_vtxlst_t);

/**
 * Free up weighted edge list resources. */
extern void rotz_free_wtxlst(rtz_wtxlst_t);

/**
 * Add an edge from vertex FROM to vertex TO. */
extern int rotz_add_edge(rotz_t, rtz_vtx_t from, rtz_vtx_t to);

/**
 * Remove an edge from vertex FROM to vertex TO. */
extern int rotz_rem_edge(rotz_t, rtz_vtx_t from, rtz_vtx_t to);


/**
 * Call CB for for every vertex in CTX, passing the vertex, its name and
 * a custom pointer to a closure object CLO.
 * The iteration stops when the callback returns values <0. */
extern void
rotz_vtx_iter(rotz_t, int(*cb)(rtz_vtx_t, const char*, void*), void *clo);

/**
 * Call CB for for every edge in CTX, passing the source vertex, the
 * vertex' adjacency list, and a custom pointer to a closure object C.
 * The iteration stops when the callback returns values <0.
 * The adjacency list passed to the callback resides in static memory
 * and must not be freed. */
extern void
rotz_edg_iter(rotz_t, int(*cb)(rtz_vtx_t, rtz_const_vtxlst_t, void*), void *C);

/**
 * Generic iterator, too secret to be documented. */
extern void
rotz_iter(
	rotz_t, rtz_const_buf_t prfx_match,
	int(*cb)(rtz_const_buf_t key, rtz_const_buf_t val, void*), void *C);


/* set operations */
/**
 * Return the union of edges X and the edges of V. */
extern rtz_vtxlst_t rotz_union(rotz_t, rtz_vtxlst_t x, rtz_vtx_t v);

/**
 * Return the intersection of edges X and the edges of V. */
extern rtz_vtxlst_t rotz_intersection(rotz_t, rtz_vtxlst_t x, rtz_vtx_t v);

/**
 * Return the union of edges X and the edges of V along with counts.
 * For every */
extern rtz_wtxlst_t rotz_munion(rotz_t, rtz_wtxlst_t x, rtz_vtx_t v);

#endif	/* INCLUDED_rotz_h_ */

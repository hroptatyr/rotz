---
title: rotz
layout: default
---

rotz
====

Rolf's Orphaned Tagging Zythepsary.

Vapourware.

Wrapper around [tokyocabinet][1] to store [tag metadata][3] of items in
a bipartite graph, much like [vertexdb][2] but with the emphasis on bulk
operations.

Currently, access via command-line is implemented, and access via http
is envisaged.

+ github page: <https://github.com/hroptatyr/rotz>
+ project homepage: <http://www.fresse.org/rotz>

Command-line tools
------------------

- `rotz-add`  Add tags to symbols
- `rotz-del`  Remove tags from symbols
- `rotz-show` Show symbols associated with tags, or tags associated with symbols
- `rotz-alias` Make a tag also known under a different name
- `rotz-rename` Rename a tag
- `rotz-search` Show tags beginning with a specified prefix
- `rotz-combine` Combine several separate tags into one
- `rotz-cloud` Display tag clouds

Examples
--------

Start out anywhere by adding some tags:

    $ rotz-add foo bar
    $ rotz-add foo baz

Now show all associations with foo

    $ rotz-show foo
    bar
    baz


Show all associations of bar

    $ rotz-show bar
    foo

Delete the association between foo and bar

    $ rotz-del foo bar
    $ rotz-show foo
    baz

Add a tag alias to foo, something more meaningful

    $ rotz-alias foo important

Find important items

    $ rotz-show important
    baz

Make bar important again

    $ rotz-add important bar
    $ rotz-show foo
    bar
    baz


API
---

The API contains the usual graph operations:

- `rtz_vtx_t rotz_get_vertex(rotz_t, const char *v)`
  Identify a vertex by name and return its object handle.
- `rtz_vtx_t rotz_add_vertex(rotz_t, const char *v)`
  Add vertex `V` to rotz database file and return its object handle.
- `rtz_vtx_t rotz_rem_vertex(rotz_t, const char *v)`
  Remove vertex `V` from rotz database file and return its former object handle.

- `int rotz_add_alias(rotz_t, rtz_vtx_t v, const char *alias)`
  Make ALIAS an alias for vertex object `V`.
- `int rotz_rem_alias(rotz_t, const char *alias)`
  Remove `ALIAS` from the list of aliases associated with `ALIAS`.
- `rtz_buf_t rotz_get_aliases(rotz_t, rtz_vtx_t v)`
  Return the list of aliases for vertex object `V`.

- `int rotz_get_edge(rotz_t, rtz_vtx_t from, rtz_vtx_t to)`
  Return non-0 iff the edge from vertex `FROM` to vertex `TO` exists.
- `int rotz_add_edge(rotz_t, rtz_vtx_t from, rtz_vtx_t to)`
  Add an edge from vertex `FROM` to vertex `TO`.
- `int rotz_rem_edge(rotz_t, rtz_vtx_t from, rtz_vtx_t to)`
  Remove an edge from vertex `FROM` to vertex `TO`.

- `rtz_vtxlst_t rotz_get_edges(rotz_t, rtz_vtx_t v)`
  Return all (outgoing) edges from vertex `V`.
- `size_t rotz_get_nedges(rotz_t, rtz_vtx_t v)`
  Return the number of (outgoing) edges from vertex `V`.
- `int rotz_rem_edges(rotz_t, rtz_vtx_t v)`
  Remove all (outgoing) edges from a vertex `V`.

  [1]: http://fallabs.com/tokyocabinet/
  [2]: https://github.com/stevedekorte/vertexdb
  [3]: http://en.wikipedia.org/wiki/Tag_%28metadata%29

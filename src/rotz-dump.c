#include <stdio.h>
#include <lmdb.h>

int
main(int argc, char *argv[])
{
	MDB_env *env;
	MDB_dbi dbi;
	MDB_val key;
	MDB_val val;
	MDB_txn *txn;
	MDB_cursor *crs;
	int rc;
	int omode = MDB_RDONLY | MDB_NOTLS | MDB_NOSUBDIR;

	rc = mdb_env_create(&env);
	rc = mdb_env_open(env, "rotz.mdb", omode, 0644);
	rc = mdb_txn_begin(env, NULL, MDB_RDONLY, &txn);
	rc = mdb_dbi_open(txn, NULL, 0, &dbi);

	rc = mdb_cursor_open(txn, dbi, &crs);

	while ((rc = mdb_cursor_get(crs, &key, &val, MDB_NEXT)) == 0) {
		fwrite(key.mv_data, 1, key.mv_size, stdout);
		fputc('\t', stdout);
		fwrite(val.mv_data, 1, val.mv_size, stdout);
		fputc('\n', stdout);
	}

	mdb_cursor_close(crs);
	mdb_dbi_close(env, dbi);
	mdb_txn_abort(txn);
	mdb_env_close(env);
	return rc;
}

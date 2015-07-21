/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996-2004
 *	Sleepycat Software.  All rights reserved.
 *
 * $Id: lock_method.c,v 1.1.1.1 2011/07/05 04:44:52 ivanzh Exp $
 */

#include "db_config.h"

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#ifdef HAVE_RPC
#include <rpc/rpc.h>
#endif

#include <string.h>
#endif

#ifdef HAVE_RPC
#include "db_server.h"
#endif

#include "db_int.h"
#include "dbinc/db_shash.h"
#include "dbinc/lock.h"

#ifdef HAVE_RPC
#include "dbinc_auto/rpc_client_ext.h"
#endif

static int __lock_get_lk_conflicts __P((DB_ENV *, const u_int8_t **, int *));
static int __lock_set_lk_conflicts __P((DB_ENV *, u_int8_t *, int));
static int __lock_get_lk_detect __P((DB_ENV *, u_int32_t *));
static int __lock_get_lk_max_lockers __P((DB_ENV *, u_int32_t *));
static int __lock_get_lk_max_locks __P((DB_ENV *, u_int32_t *));
static int __lock_get_lk_max_objects __P((DB_ENV *, u_int32_t *));
static int __lock_get_env_timeout __P((DB_ENV *, db_timeout_t *, u_int32_t));

/*
 * __lock_dbenv_create --
 *	Lock specific creation of the DB_ENV structure.
 *
 * PUBLIC: void __lock_dbenv_create __P((DB_ENV *));
 */
void
__lock_dbenv_create(dbenv)
	DB_ENV *dbenv;
{
	/*
	 * !!!
	 * Our caller has not yet had the opportunity to reset the panic
	 * state or turn off mutex locking, and so we can neither check
	 * the panic state or acquire a mutex in the DB_ENV create path.
	 */

	dbenv->lk_max = DB_LOCK_DEFAULT_N;
	dbenv->lk_max_lockers = DB_LOCK_DEFAULT_N;
	dbenv->lk_max_objects = DB_LOCK_DEFAULT_N;

#ifdef	HAVE_RPC
	if (F_ISSET(dbenv, DB_ENV_RPCCLIENT)) {
		dbenv->get_lk_conflicts = __dbcl_get_lk_conflicts;
		dbenv->set_lk_conflicts = __dbcl_set_lk_conflict;
		dbenv->get_lk_detect = __dbcl_get_lk_detect;
		dbenv->set_lk_detect = __dbcl_set_lk_detect;
		dbenv->set_lk_max = __dbcl_set_lk_max;
		dbenv->get_lk_max_lockers = __dbcl_get_lk_max_lockers;
		dbenv->set_lk_max_lockers = __dbcl_set_lk_max_lockers;
		dbenv->get_lk_max_locks = __dbcl_get_lk_max_locks;
		dbenv->set_lk_max_locks = __dbcl_set_lk_max_locks;
		dbenv->get_lk_max_objects = __dbcl_get_lk_max_objects;
		dbenv->set_lk_max_objects = __dbcl_set_lk_max_objects;

		dbenv->lock_detect = __dbcl_lock_detect;
		dbenv->lock_get = __dbcl_lock_get;
		dbenv->lock_id = __dbcl_lock_id;
		dbenv->lock_id_free = __dbcl_lock_id_free;
		dbenv->lock_put = __dbcl_lock_put;
		dbenv->lock_stat = __dbcl_lock_stat;
		dbenv->lock_stat_print = NULL;
		dbenv->lock_vec = __dbcl_lock_vec;
	} else
#endif
	{
		dbenv->get_lk_conflicts = __lock_get_lk_conflicts;
		dbenv->set_lk_conflicts = __lock_set_lk_conflicts;
		dbenv->get_lk_detect = __lock_get_lk_detect;
		dbenv->set_lk_detect = __lock_set_lk_detect;
		dbenv->set_lk_max = __lock_set_lk_max;
		dbenv->get_lk_max_lockers = __lock_get_lk_max_lockers;
		dbenv->set_lk_max_lockers = __lock_set_lk_max_lockers;
		dbenv->get_lk_max_locks = __lock_get_lk_max_locks;
		dbenv->set_lk_max_locks = __lock_set_lk_max_locks;
		dbenv->get_lk_max_objects = __lock_get_lk_max_objects;
		dbenv->set_lk_max_objects = __lock_set_lk_max_objects;
		dbenv->get_timeout = __lock_get_env_timeout;
		dbenv->set_timeout = __lock_set_env_timeout;

		dbenv->lock_detect = __lock_detect_pp;
		dbenv->lock_get = __lock_get_pp;
		dbenv->lock_id = __lock_id_pp;
		dbenv->lock_id_free = __lock_id_free_pp;
		dbenv->lock_put = __lock_put_pp;
		dbenv->lock_stat = __lock_stat_pp;
		dbenv->lock_stat_print = __lock_stat_print_pp;
		dbenv->lock_vec = __lock_vec_pp;
	}
}

/*
 * __lock_dbenv_close --
 *	Lock specific destruction of the DB_ENV structure.
 *
 * PUBLIC: int __lock_dbenv_close __P((DB_ENV *));
 */
int
__lock_dbenv_close(dbenv)
	DB_ENV *dbenv;
{
	if (dbenv->lk_conflicts != NULL) {
		__os_free(dbenv, dbenv->lk_conflicts);
		dbenv->lk_conflicts = NULL;
	}

	return (0);
}

/*
 * __lock_get_lk_conflicts
 *	Get the conflicts matrix.
 */
static int
__lock_get_lk_conflicts(dbenv, lk_conflictsp, lk_modesp)
	DB_ENV *dbenv;
	const u_int8_t **lk_conflictsp;
	int *lk_modesp;
{
	ENV_NOT_CONFIGURED(dbenv,
	    dbenv->lk_handle, "DB_ENV->get_lk_conflicts", DB_INIT_LOCK);

	if (LOCKING_ON(dbenv)) {
		/* Cannot be set after open, no lock required to read. */
		if (lk_conflictsp != NULL)
			*lk_conflictsp =
			    ((DB_LOCKTAB *)dbenv->lk_handle)->conflicts;
		if (lk_modesp != NULL)
			*lk_modesp = ((DB_LOCKREGION *)((DB_LOCKTAB *)
			    dbenv->lk_handle)->reginfo.primary)->stat.st_nmodes;
	} else {
		if (lk_conflictsp != NULL)
			*lk_conflictsp = dbenv->lk_conflicts;
		if (lk_modesp != NULL)
			*lk_modesp = dbenv->lk_modes;
	}
	return (0);
}

/*
 * __lock_set_lk_conflicts
 *	Set the conflicts matrix.
 */
static int
__lock_set_lk_conflicts(dbenv, lk_conflicts, lk_modes)
	DB_ENV *dbenv;
	u_int8_t *lk_conflicts;
	int lk_modes;
{
	int ret;

	ENV_ILLEGAL_AFTER_OPEN(dbenv, "DB_ENV->set_lk_conflicts");

	if (dbenv->lk_conflicts != NULL) {
		__os_free(dbenv, dbenv->lk_conflicts);
		dbenv->lk_conflicts = NULL;
	}
	if ((ret = __os_malloc(dbenv,
	    (size_t)(lk_modes * lk_modes), &dbenv->lk_conflicts)) != 0)
		return (ret);
	memcpy(
	    dbenv->lk_conflicts, lk_conflicts, (size_t)(lk_modes * lk_modes));
	dbenv->lk_modes = lk_modes;

	return (0);
}

static int
__lock_get_lk_detect(dbenv, lk_detectp)
	DB_ENV *dbenv;
	u_int32_t *lk_detectp;
{
	DB_LOCKTAB *lt;

	ENV_NOT_CONFIGURED(dbenv,
	    dbenv->lk_handle, "DB_ENV->get_lk_detect", DB_INIT_LOCK);

	if (LOCKING_ON(dbenv)) {
		lt = dbenv->lk_handle;
		LOCKREGION(dbenv, lt);
		*lk_detectp = ((DB_LOCKREGION *)
		    ((DB_LOCKTAB *)dbenv->lk_handle)->reginfo.primary)->detect;
		UNLOCKREGION(dbenv, lt);
	} else
		*lk_detectp = dbenv->lk_detect;
	return (0);
}

/*
 * __lock_set_lk_detect
 *	DB_ENV->set_lk_detect.
 *
 * PUBLIC: int __lock_set_lk_detect __P((DB_ENV *, u_int32_t));
 */
int
__lock_set_lk_detect(dbenv, lk_detect)
	DB_ENV *dbenv;
	u_int32_t lk_detect;
{
	DB_LOCKTAB *lt;
	DB_LOCKREGION *region;
	int ret;

	ENV_NOT_CONFIGURED(dbenv,
	    dbenv->lk_handle, "DB_ENV->set_lk_detect", DB_INIT_LOCK);

	switch (lk_detect) {
	case DB_LOCK_DEFAULT:
	case DB_LOCK_EXPIRE:
	case DB_LOCK_MAXLOCKS:
	case DB_LOCK_MAXWRITE:
	case DB_LOCK_MINLOCKS:
	case DB_LOCK_MINWRITE:
	case DB_LOCK_OLDEST:
	case DB_LOCK_RANDOM:
	case DB_LOCK_YOUNGEST:
		break;
	default:
		__db_err(dbenv,
	    "DB_ENV->set_lk_detect: unknown deadlock detection mode specified");
		return (EINVAL);
	}

	ret = 0;
	if (LOCKING_ON(dbenv)) {
		lt = dbenv->lk_handle;
		region = lt->reginfo.primary;
		LOCKREGION(dbenv, lt);
		/*
		 * Check for incompatible automatic deadlock detection requests.
		 * There are scenarios where changing the detector configuration
		 * is reasonable, but we disallow them guessing it is likely to
		 * be an application error.
		 *
		 * We allow applications to turn on the lock detector, and we
		 * ignore attempts to set it to the default or current value.
		 */
		if (region->detect != DB_LOCK_NORUN &&
		    lk_detect != DB_LOCK_DEFAULT &&
		    region->detect != lk_detect) {
			__db_err(dbenv,
	    "DB_ENV->set_lk_detect: incompatible deadlock detector mode");
			ret = EINVAL;
		} else
			if (region->detect == DB_LOCK_NORUN)
				region->detect = lk_detect;
		UNLOCKREGION(dbenv, lt);
	} else
		dbenv->lk_detect = lk_detect;

	return (ret);
}

/*
 * __lock_set_lk_max
 *	DB_ENV->set_lk_max.
 *
 * PUBLIC: int __lock_set_lk_max __P((DB_ENV *, u_int32_t));
 */
int
__lock_set_lk_max(dbenv, lk_max)
	DB_ENV *dbenv;
	u_int32_t lk_max;
{
	ENV_ILLEGAL_AFTER_OPEN(dbenv, "DB_ENV->set_lk_max");

	dbenv->lk_max = lk_max;
	dbenv->lk_max_objects = lk_max;
	dbenv->lk_max_lockers = lk_max;
	return (0);
}

static int
__lock_get_lk_max_locks(dbenv, lk_maxp)
	DB_ENV *dbenv;
	u_int32_t *lk_maxp;
{
	ENV_NOT_CONFIGURED(dbenv,
	    dbenv->lk_handle, "DB_ENV->get_lk_maxlocks", DB_INIT_LOCK);

	if (LOCKING_ON(dbenv)) {
		/* Cannot be set after open, no lock required to read. */
		*lk_maxp = ((DB_LOCKREGION *)((DB_LOCKTAB *)
		    dbenv->lk_handle)->reginfo.primary)->stat.st_maxlocks;
	} else
		*lk_maxp = dbenv->lk_max;
	return (0);
}

/*
 * __lock_set_lk_max_locks
 *	DB_ENV->set_lk_max_locks.
 *
 * PUBLIC: int __lock_set_lk_max_locks __P((DB_ENV *, u_int32_t));
 */
int
__lock_set_lk_max_locks(dbenv, lk_max)
	DB_ENV *dbenv;
	u_int32_t lk_max;
{
	ENV_ILLEGAL_AFTER_OPEN(dbenv, "DB_ENV->set_lk_max_locks");

	dbenv->lk_max = lk_max;
	return (0);
}

static int
__lock_get_lk_max_lockers(dbenv, lk_maxp)
	DB_ENV *dbenv;
	u_int32_t *lk_maxp;
{
	ENV_NOT_CONFIGURED(dbenv,
	    dbenv->lk_handle, "DB_ENV->get_lk_max_lockers", DB_INIT_LOCK);

	if (LOCKING_ON(dbenv)) {
		/* Cannot be set after open, no lock required to read. */
		*lk_maxp = ((DB_LOCKREGION *)((DB_LOCKTAB *)
		    dbenv->lk_handle)->reginfo.primary)->stat.st_maxlockers;
	} else
		*lk_maxp = dbenv->lk_max_lockers;
	return (0);
}

/*
 * __lock_set_lk_max_lockers
 *	DB_ENV->set_lk_max_lockers.
 *
 * PUBLIC: int __lock_set_lk_max_lockers __P((DB_ENV *, u_int32_t));
 */
int
__lock_set_lk_max_lockers(dbenv, lk_max)
	DB_ENV *dbenv;
	u_int32_t lk_max;
{
	ENV_ILLEGAL_AFTER_OPEN(dbenv, "DB_ENV->set_lk_max_lockers");

	dbenv->lk_max_lockers = lk_max;
	return (0);
}

static int
__lock_get_lk_max_objects(dbenv, lk_maxp)
	DB_ENV *dbenv;
	u_int32_t *lk_maxp;
{
	ENV_NOT_CONFIGURED(dbenv,
	    dbenv->lk_handle, "DB_ENV->get_lk_max_objects", DB_INIT_LOCK);

	if (LOCKING_ON(dbenv)) {
		/* Cannot be set after open, no lock required to read. */
		*lk_maxp = ((DB_LOCKREGION *)((DB_LOCKTAB *)
		    dbenv->lk_handle)->reginfo.primary)->stat.st_maxobjects;
	} else
		*lk_maxp = dbenv->lk_max_objects;
	return (0);
}

/*
 * __lock_set_lk_max_objects
 *	DB_ENV->set_lk_max_objects.
 *
 * PUBLIC: int __lock_set_lk_max_objects __P((DB_ENV *, u_int32_t));
 */
int
__lock_set_lk_max_objects(dbenv, lk_max)
	DB_ENV *dbenv;
	u_int32_t lk_max;
{
	ENV_ILLEGAL_AFTER_OPEN(dbenv, "DB_ENV->set_lk_max_objects");

	dbenv->lk_max_objects = lk_max;
	return (0);
}

static int
__lock_get_env_timeout(dbenv, timeoutp, flag)
	DB_ENV *dbenv;
	db_timeout_t *timeoutp;
	u_int32_t flag;
{
	DB_LOCKTAB *lt;
	DB_LOCKREGION *region;
	int ret;

	ENV_NOT_CONFIGURED(dbenv,
	    dbenv->lk_handle, "DB_ENV->get_env_timeout", DB_INIT_LOCK);

	ret = 0;
	if (LOCKING_ON(dbenv)) {
		lt = dbenv->lk_handle;
		region = lt->reginfo.primary;
		LOCKREGION(dbenv, lt);
		switch (flag) {
		case DB_SET_LOCK_TIMEOUT:
			*timeoutp = region->lk_timeout;
			break;
		case DB_SET_TXN_TIMEOUT:
			*timeoutp = region->tx_timeout;
			break;
		default:
			ret = 1;
			break;
		}
		UNLOCKREGION(dbenv, lt);
	} else
		switch (flag) {
		case DB_SET_LOCK_TIMEOUT:
			*timeoutp = dbenv->lk_timeout;
			break;
		case DB_SET_TXN_TIMEOUT:
			*timeoutp = dbenv->tx_timeout;
			break;
		default:
			ret = 1;
			break;
		}

	if (ret)
		ret = __db_ferr(dbenv, "DB_ENV->get_timeout", 0);

	return (ret);
}

/*
 * __lock_set_env_timeout
 *	DB_ENV->set_lock_timeout.
 *
 * PUBLIC: int __lock_set_env_timeout __P((DB_ENV *, db_timeout_t, u_int32_t));
 */
int
__lock_set_env_timeout(dbenv, timeout, flags)
	DB_ENV *dbenv;
	db_timeout_t timeout;
	u_int32_t flags;
{
	DB_LOCKTAB *lt;
	DB_LOCKREGION *region;
	int ret;

	ENV_NOT_CONFIGURED(dbenv,
	    dbenv->lk_handle, "DB_ENV->set_env_timeout", DB_INIT_LOCK);

	ret = 0;
	if (LOCKING_ON(dbenv)) {
		lt = dbenv->lk_handle;
		region = lt->reginfo.primary;
		LOCKREGION(dbenv, lt);
		switch (flags) {
		case DB_SET_LOCK_TIMEOUT:
			region->lk_timeout = timeout;
			break;
		case DB_SET_TXN_TIMEOUT:
			region->tx_timeout = timeout;
			break;
		default:
			ret = 1;
			break;
		}
		UNLOCKREGION(dbenv, lt);
	} else
		switch (flags) {
		case DB_SET_LOCK_TIMEOUT:
			dbenv->lk_timeout = timeout;
			break;
		case DB_SET_TXN_TIMEOUT:
			dbenv->tx_timeout = timeout;
			break;
		default:
			ret = 1;
			break;
		}

	if (ret)
		ret = __db_ferr(dbenv, "DB_ENV->set_timeout", 0);

	return (ret);
}
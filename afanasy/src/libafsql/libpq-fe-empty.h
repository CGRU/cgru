/*-------------------------------------------------------------------------
 *
 * libpq-fe.h
 *    This file contains definitions for structures and
 *    externs for functions used by frontend postgres applications.
 *
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/interfaces/libpq/libpq-fe.h
 *
 *-------------------------------------------------------------------------
 */

/*
	WARNING: this file is a stub in case we don't compile with PostgreSQL.
	It has been renamed to libpq-fe-empty.h.
*/

#ifndef LIBPQ_FE_EMPTY_H
#define LIBPQ_FE_EMPTY_H

#ifdef __cplusplus
extern      "C"
{
#endif

typedef unsigned int Oid;

typedef enum
{
    CONNECTION_OK,
    CONNECTION_BAD,
    /* Non-blocking mode only below here */

    /*
     * The existence of these should never be relied upon - they should only
     * be used for user feedback or similar purposes.
     */
    CONNECTION_STARTED,         /* Waiting for connection to be made.  */
    CONNECTION_MADE,            /* Connection OK; waiting to send.     */
    CONNECTION_AWAITING_RESPONSE,       /* Waiting for a response from the
                                         * postmaster.        */
    CONNECTION_AUTH_OK,         /* Received authentication; waiting for
                                 * backend startup. */
    CONNECTION_SETENV,          /* Negotiating environment. */
    CONNECTION_SSL_STARTUP,     /* Negotiating SSL. */
    CONNECTION_NEEDED           /* Internal state: connect() needed */
} ConnStatusType;

typedef enum
{
    PGRES_POLLING_FAILED = 0,
    PGRES_POLLING_READING,      /* These two indicate that one may    */
    PGRES_POLLING_WRITING,      /* use select before polling again.   */
    PGRES_POLLING_OK,
    PGRES_POLLING_ACTIVE        /* unused; keep for awhile for backwards
                                 * compatibility */
} PostgresPollingStatusType;

typedef enum
{
    PGRES_EMPTY_QUERY = 0,      /* empty query string was executed */
    PGRES_COMMAND_OK,           /* a query command that doesn't return
                                 * anything was executed properly by the
                                 * backend */
    PGRES_TUPLES_OK,            /* a query command that returns tuples was
                                 * executed properly by the backend, PGresult
                                 * contains the result tuples */
    PGRES_COPY_OUT,             /* Copy Out data transfer in progress */
    PGRES_COPY_IN,              /* Copy In data transfer in progress */
    PGRES_BAD_RESPONSE,         /* an unexpected response was recv'd from the
                                 * backend */
    PGRES_NONFATAL_ERROR,       /* notice or warning message */
    PGRES_FATAL_ERROR,          /* query failed */
    PGRES_COPY_BOTH             /* Copy In/Out data transfer in progress */
} ExecStatusType;

typedef struct pg_conn PGconn;
typedef struct pg_result PGresult;
typedef struct _PQconninfoOption
{
    char       *keyword;        /* The keyword of the option            */
    char       *envvar;         /* Fallback environment variable name   */
    char       *compiled;       /* Fallback compiled in default value   */
    char       *val;            /* Option's current value, or NULL       */
    char       *label;          /* Label for field in connect dialog    */
    char       *dispchar;       /* Indicates how to display this field in a
                                 * connect dialog. Values are: "" Display
                                 * entered value as is "*" Password field -
                                 * hide value "D"  Debug option - don't show
                                 * by default */
    int         dispsize;       /* Field size in characters for dialog  */
} PQconninfoOption;

inline PGconn *PQconnectStart(const char *conninfo) { return 0x0; }
inline PGconn *PQconnectStartParams(
	const char *const * keywords,
	const char *const * values, int expand_dbname)
{
	return 0x0;
}

inline PostgresPollingStatusType PQconnectPoll(PGconn *conn) { return PGRES_POLLING_FAILED; }
inline PGconn *PQconnectdb(const char *conninfo) { return 0x0; }
inline PGconn *PQconnectdbParams(const char *const * keywords,
                  const char *const * values, int expand_dbname)
{
	return 0x0;
}

inline PGconn *PQsetdbLogin(const char *pghost, const char *pgport,
             const char *pgoptions, const char *pgtty,
             const char *dbName,
             const char *login, const char *pwd)
{
	return 0x0;
}

#define PQsetdb(M_PGHOST,M_PGPORT,M_PGOPT,M_PGTTY,M_DBNAME)  \
    PQsetdbLogin(M_PGHOST, M_PGPORT, M_PGOPT, M_PGTTY, M_DBNAME, NULL, NULL)

/* close the current connection and free the PGconn data structure */
inline void PQfinish(PGconn *conn) { } 
inline PQconninfoOption *PQconndefaults(void) { return 0x0; }
inline PQconninfoOption *PQconninfoParse(const char *conninfo, char **errmsg) { return 0x0; }
inline void PQconninfoFree(PQconninfoOption *connOptions) { return; }
inline ConnStatusType PQstatus(const PGconn *conn) { return CONNECTION_BAD; }
inline char *PQerrorMessage(const PGconn *conn) { return 0x0; }
inline PGresult *PQexec(PGconn *conn, const char *query) { return 0x0; }

inline PGresult *PQprepare(PGconn *conn, const char *stmtName,
          const char *query, int nParams,
          const Oid *paramTypes)
{
	return 0x0;
}

inline PGresult *PQexecPrepared(PGconn *conn,
               const char *stmtName,
               int nParams,
               const char *const * paramValues,
               const int *paramLengths,
               const int *paramFormats,
               int resultFormat)
{
	return 0x0;
}

inline PGresult *PQgetResult(PGconn *conn) { return 0x0; }
inline ExecStatusType PQresultStatus(const PGresult *res) { return PGRES_EMPTY_QUERY; }
inline int  PQntuples(const PGresult *res) { return 0; }
inline int  PQnfields(const PGresult *res) { return 0; }
inline char *PQfname(const PGresult *res, int field_num) { return 0x0; }
inline char *PQgetvalue(const PGresult *res, int tup_num, int field_num) { return 0x0; }
inline void PQclear(PGresult *res) { return; }

/* Exists for backward compatibility.  bjm 2003-03-24 */
#define PQfreeNotify(ptr) PQfreemem(ptr)

#ifdef __cplusplus
}
#endif

#endif   /* LIBPQ_FE_H */

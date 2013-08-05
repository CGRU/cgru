#pragma once

#ifdef NO_POSTGRESQL
#	include "../libafsql/libpq-fe-empty.h"
#else
#	include <libpq-fe.h>
#endif

#include "../libafanasy/name_af.h"

namespace afsql
{
/// Init environment variables.
	void init();

	class DBAttr;
	class DBAttrUInt8;
	class DBAttrInt32;
	class DBAttrUInt32;
	class DBItem;
	class DBStatJob;
	class DBStatBlock;


	bool connect( PGconn * i_conn);
	bool execute( PGconn * i_conn, const std::list<std::string> * i_queries);

/// Database comlex procedures interface.
	class DBConnection;

	void ResetStat(    DBConnection * dbconnenction);
	void ResetAll(     DBConnection * dbconnenction);

	void UpdateTables( DBConnection * dbconnenction, bool showOnly = false);
}

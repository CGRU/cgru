#ifndef CN_INCLUDE_H
#define CN_INCLUDE_H

#define CN_CHECK_AND_RETURN_FALSE(val, str) \
{											\
 	if (!val){								\
		CN_ERR(str);						\
		return false;						\
	}										\
}

#endif
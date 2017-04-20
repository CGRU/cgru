#pragma once

#include "afnode.h"

class MonitorContainer;

namespace af
{
/// Afanasy Work classes have jobs (user, branch).
class Work : public Node
{
public:
	Work();
	virtual ~Work();

	//Solving:
	enum SolvingMethod{
		SolveByOrder     = 0,
		SolveByPriority  = 1
	};

    void generateInfoStream( std::ostringstream & o_str, bool full = false) const; /// Generate information.

	void jsonRead( const JSON & i_object, std::string * io_changes = NULL);
	void jsonWrite( std::ostringstream & o_str, int i_type) const;

	int calcWeight() const;       ///< Calculate and return memory size.

protected:
	void readwrite( Msg * msg);   ///< Read or write node attributes in message

protected:

private:
	std::map< std::string, int32_t > m_pools;

};
}

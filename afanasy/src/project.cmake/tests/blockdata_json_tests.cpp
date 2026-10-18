#include "../../libafanasy/blockdata.h"
#include "../../libafanasy/name_af.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>

namespace
{
bool expect(bool i_condition, const std::string & i_message)
{
	if( i_condition)
		return true;
	std::cerr << "FAILED: " << i_message << std::endl;
	return false;
}

// A numeric block (flags 1 = FNumeric) with the given extra members.
std::string blockJson(const std::string & i_members)
{
	return std::string("{\"name\":\"render\",\"service\":\"generic\",\"command\":\"cmd\",\"tasks_name\":\"task\","
		"\"flags\":1,\"frame_first\":1,\"frame_last\":1") + i_members + "}";
}

// Reads a block the way the server reads a submitted or stored one.
std::unique_ptr<af::BlockData> readBlock(const std::string & i_json)
{
	rapidjson::Document document;
	char * data = af::jsonParseData(document, i_json.c_str(), int(i_json.size()));
	if( NULL == data)
		return std::unique_ptr<af::BlockData>();
	std::unique_ptr<af::BlockData> block(new af::BlockData(document, 0));
	delete [] data;
	return block;
}

// Writes a block the way the server writes it to the job store.
std::string storeBlock(const af::BlockData & i_block)
{
	std::ostringstream str;
	i_block.jsonWrite(str, 0);
	return str.str();
}

bool testStoreKeepsCommands()
{
	std::unique_ptr<af::BlockData> submitted = readBlock(blockJson(",\"command_pre\":\"pre.sh\",\"command_post\":\"post.sh\""));
	if( false == expect(NULL != submitted.get(), "submitted block parse should succeed"))
		return false;

	std::string stored = storeBlock(*submitted);
	if( false == expect(std::string::npos != stored.find("\"command_pre\":\"pre.sh\""), "store should write command_pre"))
		return false;
	if( false == expect(std::string::npos != stored.find("\"command_post\":\"post.sh\""), "store should write command_post"))
		return false;

	std::unique_ptr<af::BlockData> restored = readBlock(stored);
	if( false == expect(NULL != restored.get(), "stored block parse should succeed"))
		return false;
	if( false == expect(restored->getCmdPre() == "pre.sh", "restored pre command mismatch"))
		return false;
	if( false == expect(restored->getCmdPost() == "post.sh", "restored post command mismatch"))
		return false;

	return true;
}

bool testReadsOldStoreKeys()
{
	std::unique_ptr<af::BlockData> block = readBlock(blockJson(",\"cmd_pre\":\"pre.sh\",\"cmd_post\":\"post.sh\""));
	if( false == expect(NULL != block.get(), "old store block parse should succeed"))
		return false;
	if( false == expect(block->getCmdPre() == "pre.sh", "cmd_pre should be read"))
		return false;
	if( false == expect(block->getCmdPost() == "post.sh", "cmd_post should be read"))
		return false;

	return true;
}

bool testCommandKeysWin()
{
	std::unique_ptr<af::BlockData> block = readBlock(blockJson(",\"cmd_post\":\"old.sh\",\"command_post\":\"new.sh\""));
	if( false == expect(NULL != block.get(), "block parse should succeed"))
		return false;
	if( false == expect(block->getCmdPost() == "new.sh", "command_post should win over cmd_post"))
		return false;

	return true;
}
}

int main()
{
	if( false == testStoreKeepsCommands())
		return 1;
	if( false == testReadsOldStoreKeys())
		return 1;
	if( false == testCommandKeysWin())
		return 1;

	std::cout << "test_blockdata_json: ok" << std::endl;
	return 0;
}

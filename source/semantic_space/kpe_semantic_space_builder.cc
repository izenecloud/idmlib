#include <boost/filesystem.hpp>

#include <idmlib/semantic_space/kpe_semantic_space_builder.h>
#include <util/scd_parser.h>

using namespace boost::filesystem;
using namespace idmlib::ssp;

bool KpeSemanticSpaceBuilder::buildInvertedIndex()
{
	std::vector<std::string> scdFileList;
	if (!getScdFileList(scdPath_, scdFileList)) {
		return false;
	}

	for (size_t i = 0; i < scdFileList.size(); i++) {
		;
	}

	return false;
}


/// Private Methods

bool KpeSemanticSpaceBuilder::getScdFileList(const std::string& scdPath, std::vector<std::string>& fileList)
{
	if ( exists(scdPath) )
	{
		if ( !is_directory(scdPath) ) {
			std::cout << "It's not a directory: " << scdPath << std::endl;
			return false;
		}

		directory_iterator iterEnd;
		for (directory_iterator iter(scdPath); iter != iterEnd; iter ++)
		{
			std::string file_name = iter->path().filename();
			//std::cout << file_name << endl;

			if (izenelib::util::ScdParser::checkSCDFormat(file_name) )
			{
				izenelib::util::SCD_TYPE scd_type = izenelib::util::ScdParser::checkSCDType(file_name);
				if( scd_type == izenelib::util::INSERT_SCD ||scd_type == izenelib::util::UPDATE_SCD )
				{
					fileList.push_back( iter->path().string() );
				}
			}
		}

		if (fileList.size() > 0) {
			return true;
		}
		else {
			std::cout << "There is no scd file in: " << scdPath << std::endl;
			return false;
		}
	}
	else
	{
		std::cout << "File path dose not existed: " << scdPath << std::endl;
		return false;
	}
}

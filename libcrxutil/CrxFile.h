#ifndef _CRXFILE_H_
#define _CRXFILE_H_

#include <boost/filesystem.hpp>
#include <string>

namespace Crx
{
	class File : public boost::filesystem::path
	{
	public:
		File(std::string strPath, bool bWindowsPath = true);
		File(boost::filesystem::path fsPath);
		
		void SetPath(std::string strPath);
		
		bool Exists();
		void ResolveToCaseSensitivePath();
		void ConvertWindowsSeperator();
		
	};
}

#endif
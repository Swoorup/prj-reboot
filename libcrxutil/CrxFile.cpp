#include "CrxFile.h"

#ifdef __unix__
#include <unistd.h>
#include <sys/stat.h>
#endif

#include <boost/algorithm/string.hpp>

namespace Crx
{

namespace fs = boost::filesystem;

File::File(std::string strPath, bool bWindowsPath)
{
    if (bWindowsPath)
        std::replace(strPath.begin(), strPath.end(), '\\', '/');

    SetPath(strPath);

    if (bWindowsPath)
        this->ResolveToCaseSensitivePath();
}

File::File(boost::filesystem::path fsPath)
{
	SetPath(fsPath.string());
}


void File::SetPath(std::string strPath)
{
    // stupid way to assign the current path by explicitly calling the operator
	fs::path fsPath(strPath);
	this->swap(fsPath);
    //this->operator=(strPath);
}

void File::ConvertWindowsSeperator()
{
	std::string strPath = this->string();
	std::replace(strPath.begin(), strPath.end(), '\\', '/');
	SetPath(strPath);
}


bool File::Exists()
{
    std::string filename = this->string();

    // http://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c
    // POSIX test is the fastest
#ifdef __unix__
    struct stat buffer;
    return (stat (filename.c_str(), &buffer) == 0);
#else
    ifstream f(filename.c_str());
    if (f.good()) {
        f.close();
        return true;
    } else {
        f.close();
        return false;
    }
#endif

}

void File::ResolveToCaseSensitivePath()
{
    fs::path path(this->string());

    fs::path finalPath(path.root_path());
    fs::path::iterator i = path.begin();

    i++;
    while (i != path.end()) {

        bool bMatch = false;
        for (fs::directory_iterator dirIt(finalPath); dirIt != fs::directory_iterator(); dirIt++)
        {
            std::string listedEntry = dirIt->path().leaf().string();
            std::string entryToCompare = i->string();

            //cerr << "Listed Entry: " << dirIt->path() << ", \t" << listedEntry << endl;
            //cerr << "Compare To: " << entryToCompare << endl;
            boost::to_lower(listedEntry);
            boost::to_lower(entryToCompare);
            if (listedEntry == entryToCompare)
            {
                bMatch = true;
                finalPath = dirIt->path();
                break;
            }
        }

        if (bMatch == false)
            assert("FUCKING PATH TEST FAILED" == "");
        i++;
    }

    SetPath(finalPath.string());
}
}

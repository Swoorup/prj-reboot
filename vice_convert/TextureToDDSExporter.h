#ifndef __TEXTURETODDSEXPORTER_H__
#define __TEXTURETODDSEXPORTER_H__

#include <string>
#include <istream>

class TXDTextureHeader;
class MaterialDatabase;
class TXDArchive;

using std::string;
using std::istream;

// use a singleton patter
class TextureToDDSExporter {
public:
    TextureToDDSExporter(MaterialDatabase* matDB);
    ~TextureToDDSExporter();


    void Export(string sourceFile, string txdName, string exportPath, bool bOverwrite = true);
    void Export(istream *sourceFile, string txdName, string exportPath, bool bOverwrite = true);
	void Export(TXDArchive* txdArchive, string txdName, string exportPath, bool bOverwrite = true);

private:
    void _exportSingleTexture(const TXDTextureHeader& texHeader, uint8_t* data, string exportPath, bool bOverwrite = true);

private:
    MaterialDatabase* m_matDB;

};

#endif

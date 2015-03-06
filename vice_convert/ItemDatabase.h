#ifndef __ITEMDATABASE_H__
#define __ITEMDATABASE_H__

#include <vector>
#include <string>
#include <istream>
#include <unordered_map>

namespace boost{
	namespace filesystem {
		class path;
	}
}
class TextureToDDSExporter;
class DffToMeshExporter;
class MaterialDatabase;
class MapItemDefination;
class IMGArchive;

using std::unordered_map;
using std::string;
using std::istream;
using std::vector;

/*
 * TODO: Ignore the dotscene Exporter for now
 * 		this goes to getting the model and texture names and updating the materials,
 * 		extracting them from the img and this also goes to main project
 * 		in creating of the scene node directly from IPLs
 *
 */

struct MapItemPlacement{
	u_int32_t itemID;
	int interior;
	float x, y, z;
	float rotX, rotY, rotZ, rotW;
	float scaleX, scaleY, scaleZ;
};

class MapItemDatabase
{

public:
    MapItemDatabase(MaterialDatabase* matDB, TextureToDDSExporter* texExporter, DffToMeshExporter* dffExporter);
    ~MapItemDatabase();

    void ParseItemDefinationFile(string ideFile);
    void ParseItemPlacementFile(string iplFile);
    void ParseFromDATFile(string datFile, boost::filesystem::path gtaPath);

    void AddImgPath(string imgFile);

	void ExportAllItems(string textureExportPath, string meshExportPath, string materialExportPath, string sceneExportPath, bool bOverwrite = true);
	
	void SetVisitorForProgressStatus(void (*visit)(string));
private:
    bool GetResourceStreamFromIMGs(string resourceName, istream*& outputStream);

    void ExportScene(string exportPath);
private:
    typedef unordered_map<uint32_t, MapItemDefination*> ItemMap;

    ItemMap m_items;
	vector<MapItemPlacement*> m_itemsPlacement;
	
    vector<IMGArchive*> m_imgs;
    MaterialDatabase* m_matDB;
    TextureToDDSExporter* m_texExporter;
    DffToMeshExporter* m_dffExporter;
};

#endif

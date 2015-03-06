#ifndef __MATERIAL_DATABASE_H__
#define __MATERIAL_DATABASE_H__

#include <OgreSingleton.h>
#include <iosfwd>
#include <unordered_map>

namespace Ogre
{
	class MaterialManager;
	class MaterialSerializer;	
}

class DFFGeometry;
class TXDTextureHeader;
class DFFMaterial;
class MapItemDefination;
class TXDArchive;

using std::string;
using std::vector;
using std::unordered_map;

typedef vector<string> MaterialList;
typedef unordered_map<string, MaterialList> MapDffMaterials;

class MaterialDatabase : public Ogre::Singleton<MaterialDatabase>
{
public:
	MaterialDatabase(Ogre::MaterialManager *matMgr, Ogre::MaterialSerializer* matserializer);
	~MaterialDatabase();

	vector<string> CreateOrRetrieveMaterial(DFFGeometry* geom, MapItemDefination* mapItem);

	void ExportAll(string exportPath);
	void AddTextureHeaderForPreProcessing(TXDTextureHeader* tex);
private:
	Ogre::Material* GetSimilarMaterial(Ogre::Material* rhsMat);
	TXDTextureHeader* GetTexHeaderForTexture(string texName);
	void AddMaterialToList(Ogre::Material* matptr);

	vector<Ogre::Material*> m_matList;
	unordered_map<string, TXDTextureHeader*> m_preProcessedTexHeaders;
	
    Ogre::MaterialManager* m_matmgr;
	Ogre::MaterialSerializer* m_matserializer;
};

#endif

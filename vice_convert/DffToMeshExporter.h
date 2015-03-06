#ifndef __DFF_TO_MESH_EXPORTER_H__
#define __DFF_TO_MESH_EXPORTER_H__

#include <iosfwd>
#include <string>
#include <istream>
#include <vector>

using std::string;
using std::istream;
using std::vector;

// use forward declarations instead
namespace Ogre
{
	class MeshManager;
	class HardwareBufferManager;
	class MeshSerializer;
}

class MaterialDatabase;
class DFFGeometry;
class DFFMesh;

/*
 * fence.mesh - 1st clump
 * fence_clump1 - 2nd clump
 * fence_clump2 - 3rd clump
 */
const string CLUMP_SUFFIX = "_clump";	/// use suffix in mesh's name if dff contains more than one geometry

class DffToMeshExporter
{
public:
    DffToMeshExporter(Ogre::MeshManager *mmgr, 
					  Ogre::HardwareBufferManager *hwbmgr, 
					  Ogre::MeshSerializer *serializer, 
					  MaterialDatabase* matDB
					 );
    ~DffToMeshExporter();


	void Export(vector<string> matList, string sourceFile, string dffStem, string exportPath, bool bOverwrite = true);
	void Export(vector<string> matList, istream *sourceFile, string dffStem, string exportPath, bool bOverwrite = true);
	void Export(vector<string> matList, DFFMesh *dffMesh, string dffStem, string exportPath, bool bOverwrite = true);
	
private:
	void _exportSingleClump(vector<string> matList, DFFGeometry *dffGeometry, string dffStem, string exportName, string exportPath, bool bOverwrite = true);

private:
    Ogre::MeshManager *m_mmgr;
    Ogre::HardwareBufferManager *m_hwbmgr;
    Ogre::MeshSerializer *m_serializer;
	MaterialDatabase* m_matDB;
};

#endif




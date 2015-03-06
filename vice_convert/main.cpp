#include <OgreDefaultHardwareBufferManager.h>
#include <OgreLogManager.h>
#include <OgreResourceGroupManager.h>
#include <OgreLodStrategyManager.h>
#include <OgreMeshManager.h>
#include <OgreMeshSerializer.h>
#include <OgreMaterialManager.h>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <gtaformats/gtaide.h>
#include <iostream>
#include <string>

#include "DffToMeshExporter.h"
#include "TextureToDDSExporter.h"
#include "MaterialDatabase.h"
#include "ItemDatabase.h"
#include "Exception.h"

using namespace std;
namespace po = boost::program_options;
/*
 * vice_convert works as in the following order
 *
 * Load the gta_vc.dat file
 * Parse all IDE to lookup table
 * Parse all IPL back to IDE
 * 		Export the DFF Mesh
 * 		Export the TXD's img to DDS
 * 		Complete the Material
 * 		Export the sceneNode
 * Export all the material
 * Questions?
 *
 *
 * Folder organization
 * Resources
 * 		Meshes
 * 		Textures
 * 		Materials
 * 		SceneFiles
 */

/*
 *
 * Ogre::Singleton cannot be replaced by static function
 * as the object has to be instantiated explicity only once
 * This allows to create runtime single instance of the object
 *
 */

// first initialize the singleton resource group manager
Ogre::ResourceGroupManager* rgmgr = nullptr;
Ogre::MeshManager* meshmgr= nullptr;
Ogre::MaterialManager* materialmgr= nullptr;
Ogre::HardwareBufferManager* hwbmgr= nullptr;
Ogre::MeshSerializer* serializer= nullptr;
Ogre::MaterialSerializer* matSerializer = nullptr;

// create singleton instance
MaterialDatabase* matDB;

void init_backend()
{
    Ogre::LogManager* lmgr = new Ogre::LogManager();
    lmgr->createLog("Ogre.log",true,false,false); // log is disabled for now
    lmgr->setLogDetail(Ogre::LL_BOREME);
	
    // first initialize the singleton resource group manager
    rgmgr = new Ogre::ResourceGroupManager();
    materialmgr = new Ogre::MaterialManager();
    hwbmgr = new Ogre::DefaultHardwareBufferManager();
    meshmgr = new Ogre::MeshManager();
    serializer = new Ogre::MeshSerializer();
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    new Ogre::LodStrategyManager();
    matSerializer = new Ogre::MaterialSerializer();
    // create singleton instance
    matDB = new MaterialDatabase(materialmgr, matSerializer);
}

int main(int ac, char* av[])
{
    // Declare the supported options.
	
    po::options_description desc(
        "Ogre Resource Exporter for Grand Theft Auto Games\n"
        "Allowed options\n"
    );

    desc.add_options()
    ("help", "produce help message")
    ("gta-vice-city-dir", po::value<string>(), "set gta vice city root directory")
    ("output", po::value<string>(), "set the exported ogre resource directory")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);
	
	

    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }

    if (vm.count("gta-vice-city-dir"))
    {
        cout << "gta-vice-city-dir was set to: " << vm["gta-vice-city-dir"].as<string>() << endl;
    }
    else
    {
        cout << "gta vice city directory not specified" << endl;
    }

    init_backend();

    // Unit testing stuff
    DffToMeshExporter meshExporter(meshmgr, hwbmgr, serializer, matDB);
    TextureToDDSExporter texExporter(matDB);

    MapItemDatabase mapDB(matDB, &texExporter, &meshExporter);
	
	
    mapDB.AddImgPath("/home/swoorup/isolated/gtavicecity/models/gta3.dir");
    mapDB.ParseFromDATFile("/home/swoorup/isolated/gtavicecity/data/gta_vc.dat", boost::filesystem::path("/home/swoorup/isolated/gtavicecity/"));
    mapDB.ExportAllItems("/home/swoorup/test/", "/home/swoorup/test/", "/home/swoorup/test/","/home/swoorup/test/", false);

    /*meshExporter.Export("/run/media/swoorup/SHARED/MyOgreApp/build/Debug/asdca.dff", "bridge", "/home/swoorup/test/");
    texExporter.Export("/run/media/swoorup/SHARED/MyOgreApp/build/Debug/dcasd.txd", "", "/home/swoorup/test/", "bridge");

    meshExporter.Export("/run/media/swoorup/SHARED/MyOgreApp/build/Debug/player.dff", "player", "/home/swoorup/test/");
    texExporter.Export("/run/media/swoorup/SHARED/MyOgreApp/build/Debug/player.txd", "", "/home/swoorup/test/", "player");


    matDB->ExportAll("/home/swoorup/test/");
    */

}

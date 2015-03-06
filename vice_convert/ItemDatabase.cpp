#include "ItemDatabase.h"

#include "ItemDefination.h"
#include "MaterialDatabase.h"
#include "TextureToDDSExporter.h"
#include "DffToMeshExporter.h"
#include <gtaformats/gtaimg.h>
#include <gtaformats/gtadff.h>
#include <gtaformats/gtatxd.h>
#include <boost/algorithm/string.hpp>
#include <gtaformats/gtaide.h>
#include <gtaformats/gtaipl.h>
#include "Exception.h"

#include <fstream>
#include <CrxFile.h>
#include <boost/filesystem.hpp>

using std::ifstream;
using std::ios;

MapItemDatabase::MapItemDatabase(MaterialDatabase* matDB, TextureToDDSExporter* texExporter, DffToMeshExporter* dffExporter)
{
    this->m_matDB = matDB;
    this->m_texExporter = texExporter;
    this->m_dffExporter = dffExporter;
}

MapItemDatabase::~MapItemDatabase()
{
    // delete all img streams
    for (IMGArchive* img : this->m_imgs)
    {
        delete (img);
    }

    // delete all map objects
    for (auto it : this->m_items)
    {
        delete it.second;
    }
}

void MapItemDatabase::AddImgPath(string imgFile)
{
    IMGArchive* img = new IMGArchive(File(CString(imgFile)));
    this->m_imgs.push_back(img);
}

void MapItemDatabase::ParseFromDATFile(string datFile, boost::filesystem::path gtaPath)
{
    ifstream datStream(datFile, ios::in);
    if (!datStream.is_open())
        throw MyException("Cannot Load DAT File");

    while (datStream.good())
    {
        string line;
        getline(datStream, line);

        string str = line.substr(0, line.find_first_of(' '));
        boost::algorithm::to_lower(str);
        boost::trim(str);

        if (str == "ide")
        {
            string linesNeeded = line.substr(4, line.length() - 1);

            boost::trim(linesNeeded);

            Crx::File idePath(gtaPath);
            idePath /= linesNeeded;

            // Dat file uses windows seperator
            idePath.ConvertWindowsSeperator();
            idePath.ResolveToCaseSensitivePath();

            this->ParseItemDefinationFile(idePath.string());
        }
        else if (str == "ipl")
        {
            string linesNeeded = line.substr(4, line.length() - 1);
            boost::trim(linesNeeded);

            Crx::File iplPath(gtaPath);
            iplPath /= linesNeeded;

            iplPath.ConvertWindowsSeperator();
            iplPath.ResolveToCaseSensitivePath();

            this->ParseItemPlacementFile(iplPath.string());
        }
    }

    datStream.close();
}



void MapItemDatabase::ParseItemDefinationFile(string ideFile)
{

    ifstream ideStream(ideFile, ios::in);
    if (!ideStream.is_open())
        throw MyException(ideFile + " is invalid or bad");
    IDEReader ide(&ideStream, false);

    IDEStatement* stmt;
    while ((stmt = ide.readStatement()) != NULL) {
        idetype_t type = stmt->getType();

        if (type == IDEType::IDETypeStaticObject) {
            IDEStaticObject* sobj = (IDEStaticObject*)stmt;

            StaticMapItemDefination* staticItem = new StaticMapItemDefination();
            staticItem->SetDFFStem(sobj->getModelName());
            staticItem->SetTexDictStem(sobj->getTXDArchiveName().get());
            staticItem->SetDrawDistance(sobj->getDrawDistances()[0]);

            string modelName = sobj->getModelName();
            boost::algorithm::to_lower(modelName);

            if (modelName.find("lod") != string::npos)
                staticItem->SetLodProperty(true);

            this->m_items[sobj->getID()] = staticItem;
            delete sobj;
        }
    }

}

void MapItemDatabase::ParseItemPlacementFile(string iplFile)
{
    ifstream iplStream(iplFile, ios::in);
    if (!iplStream.is_open())
        throw MyException(iplFile + " is invalid or bad");
    IPLReader ipl(&iplStream, false);

    IPLStatement* stmt;
    while ((stmt = ipl.readStatement()) != NULL) {

        ipltype_t type = stmt->getType();

        if (type == IPL_TYPE_INSTANCE) {
            IPLInstance* inst = (IPLInstance*)stmt;

            MapItemPlacement* itemInst = new MapItemPlacement();
            itemInst->itemID = inst->getID();
            itemInst->interior = inst->getInterior();
            inst->getPosition(itemInst->x, itemInst->y, itemInst->z);
            inst->getScale(itemInst->scaleX, itemInst->scaleY, itemInst->scaleZ);
            inst->getRotation(itemInst->rotX, itemInst->rotY, itemInst->rotZ, itemInst->rotW);

            this->m_itemsPlacement.push_back(itemInst);
            delete inst;
        }
    }
}

bool MapItemDatabase::GetResourceStreamFromIMGs(string resourceName, istream*& outputStream)
{
    for (IMGArchive* img : this->m_imgs)
    {
        outputStream = img->gotoEntry(resourceName);
        if (outputStream != NULL)
            return true;
        continue;
        IMGArchive::EntryIterator resourceEntry = img->getEntryByName(resourceName);
        if (resourceEntry != img->getEntryEnd())
        {
            outputStream = img->gotoEntry(resourceEntry);
            return true;
        }
    }
    return false;
}

void MapItemDatabase::ExportScene(string exportPath)
{
    // always overwrite the scene file
    Crx::File sceneFile(exportPath, false);
    sceneFile /= "scenePlacement.txt";

    ofstream sceneStream(sceneFile.string());
    for (MapItemPlacement* itemInst : this->m_itemsPlacement)
    {
        uint32_t id = itemInst->itemID;
        if (m_items.find(id) == m_items.end())
        {
            std::cerr << std::to_string(id) << " not found!" << std::endl;
            continue;
            //throw MyException(std::to_string(id) + " not found");
            //assert("ID not found" == "");
        }
        MapItemDefination& itemDef = *this->m_items[id];


        sceneStream
                << itemDef.GetDFFStem() + ".mesh" << ", "
                << itemDef.GetDrawDistance() << ", "
                << itemDef.IsLod() << ", "
                << itemInst->x << ", "
                << itemInst->y << ", "
                << itemInst->z << ", "
                << itemInst->rotW << ", "
                << itemInst->rotX << ", "
                << itemInst->rotY << ", "
                << itemInst->rotZ << std::endl;

    }

    sceneStream.close();
}

void MapItemDatabase::ExportAllItems(string textureExportPath, string meshExportPath, string materialExportPath, string sceneExportPath, bool bOverwrite)
{
	// export textures and preprocess textures for materials
	
	// TODO: seperate txd out into a unique list (txd tend to repeat in Item Definations)
    for (auto it : this->m_items)
    {
        MapItemDefination& mapItem = *(it.second);
		
        istream* txdStream = nullptr;
        // preprocess for materials?
        if (!this->GetResourceStreamFromIMGs(mapItem.GetTexDictStem() + ".txd", txdStream))
            throw new MyException("TXD NOT FOUND");
		
		TXDArchive txdArchive(txdStream);
		for (int i = 0; i < txdArchive.getTextureCount(); i++)
		{
			TXDTextureHeader* texHeader = txdArchive.getHeader(i);
			m_matDB->AddTextureHeaderForPreProcessing(texHeader);
		}
		
		this->m_texExporter->Export(&txdArchive, mapItem.GetTexDictStem(), textureExportPath, bOverwrite);
    }
    
    // export the DFFs
    for (auto it : this->m_items)
    {
        MapItemDefination& mapItem = *(it.second);


        istream* dffStream = nullptr;
        if (!this->GetResourceStreamFromIMGs(mapItem.GetDFFStem() + ".dff", dffStream))
            throw new MyException("DFF NOT FOUND");
		
		DFFLoader loader;
		DFFMesh* mesh = loader.loadMesh(dffStream);
		
		// TODO: Add material for Subclumps
		vector<string> matList = this->m_matDB->CreateOrRetrieveMaterial(mesh->getGeometry(0), &mapItem);

		this->m_dffExporter->Export(matList, mesh, mapItem.GetDFFStem(), meshExportPath, bOverwrite);
		delete mesh;
    }

    this->m_matDB->ExportAll(materialExportPath);
    this->ExportScene(sceneExportPath);

}

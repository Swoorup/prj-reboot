#include "MaterialDatabase.h"

#include <OgreMaterial.h>
#include <OgreMaterialManager.h>
#include <OgreMaterialSerializer.h>

#include <gtaformats/gtadff.h>
#include <gtaformats/gtatxd.h>
#include <CrxFile.h>
#include <OgreTechnique.h>
#include <boost/algorithm/string.hpp>
#include <string>
#include <vector>

#include "ItemDefination.h"
#include "Exception.h"

template<> MaterialDatabase *Ogre::Singleton<MaterialDatabase>::msSingleton = 0;

MaterialDatabase::~MaterialDatabase()
{
	for (Ogre::Material* matptr : m_matList)
		delete matptr;
}

bool MaterialCompare(Ogre::Material* matA,  Ogre::Material* matB)
{
    if (matA->getNumTechniques() != matB->getNumTechniques())
        return false;

    for (int i = 0; i < matA->getNumTechniques(); i++)
    {
        Ogre::Technique* techA = matA->getTechnique(i);
        Ogre::Technique* techB = matB->getTechnique(i);

        if (techA->getNumPasses() != techB->getNumPasses())
            return false;

        for (int j = 0; i < techA->getNumPasses(); i++)
        {
            Ogre::Pass* passA = techA->getPass(j);
            Ogre::Pass* passB = techB->getPass(j);


            if (
                passA->getNumTextureUnitStates() != passB->getNumTextureUnitStates() ||
                passA->getVertexColourTracking() != passB->getVertexColourTracking() ||
                passA->getDepthCheckEnabled() != passB->getDepthCheckEnabled() ||
                passA->getDepthWriteEnabled() != passB->getDepthWriteEnabled() ||
                passA->getLightingEnabled() != passB->getLightingEnabled()

            )
                return false;

            for (int k = 0; i < passA->getNumTextureUnitStates(); i++)
            {
                Ogre::TextureUnitState* texUnitA = passA->getTextureUnitState(k);
                Ogre::TextureUnitState* texUnitB = passB->getTextureUnitState(k);

                if (texUnitA->getTextureName() != texUnitB->getTextureName())
                    return false;
            }
        }
    }
    return true;
}

Ogre::Material* MaterialDatabase::GetSimilarMaterial(Ogre::Material* matToCompare)
{
	for (Ogre::Material* material : m_matList)
    {
		if (MaterialCompare(matToCompare, material))
			return material;
    }

    return nullptr;
}

void MaterialDatabase::AddTextureHeaderForPreProcessing(TXDTextureHeader* tex)
{
    TXDTextureHeader* texHeader = new TXDTextureHeader(*tex);
    texHeader->setDiffuseName(tex->getDiffuseName());
    texHeader->setAlphaChannelUsed(tex->isAlphaChannelUsed());
    string texName = texHeader->getDiffuseName();
    //boost::trim(texName);
    m_preProcessedTexHeaders[texName] = texHeader;
}


// texture names must be unique regardless of txd archives throughout the whole archives;
// check txd Source
TXDTextureHeader* MaterialDatabase::GetTexHeaderForTexture(string texName)
{
    TXDTextureHeader* retTex = nullptr;

    //boost::trim(texName);

    if (m_preProcessedTexHeaders.find(texName) != m_preProcessedTexHeaders.end())
	{
		retTex = m_preProcessedTexHeaders[texName];
		assert (retTex->getDiffuseName() == texName);
	}
	

    if(retTex == nullptr)
    {
        std::cout << texName << " Texture Not Found" << std::endl;

    }
    return retTex;
}

//TODO: Add culling, texture scrolling support
vector<string> MaterialDatabase::CreateOrRetrieveMaterial(DFFGeometry* geom, MapItemDefination* mapItem)
{
    vector<string> dffMatList;

    for (int i = 0; i < geom->getPartCount(); i++)
    {
		DFFMaterial* dffMaterial = geom->getPart(i)->getMaterial();
		
        string tempMaterialName = "Mat" + std::to_string(m_matList.size());
		Ogre::MaterialPtr tempMaterial = this->m_matmgr->create(tempMaterialName, "General");

        uint8_t r, g, b, a;
        dffMaterial->getColor(r, g, b, a);

		Ogre::Technique* matTechnique = tempMaterial->createTechnique();
        Ogre::Pass * pass = matTechnique->createPass();

		pass->setDiffuse(r / 255.0f, g/255.0f, b/255.0f, a/255.0f);
		pass->setLightingEnabled(true);
		
		// enable proper lightening shading
		//pass->setAmbient(Ogre::ColourValue(0.2, 0.2, 0.2));
		//pass->setCullingMode(Ogre::CULL_CLOCKWISE);
		//pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
		//pass->setDepthCheckEnabled(true);
		
        if (geom->getVertexColors() != NULL)
			pass->setVertexColourTracking(Ogre::TrackVertexColourEnum::TVC_DIFFUSE);

		if (dffMaterial->getTextureCount() > 1)
		{
			std::cout << tempMaterialName << " has more than 1 texture unit states!" << std::endl;
		}
		
		for (int j = 0; j < dffMaterial->getTextureCount(); j++)
		{
			string dffTexName = dffMaterial->getTexture(j)->getDiffuseName();
			TXDTextureHeader* texHeader = this->GetTexHeaderForTexture(dffTexName);
			
			Ogre::TextureUnitState* texUnit = nullptr;
			texUnit = pass->createTextureUnitState(dffTexName);
			
			if (texHeader)
			{
				if (texHeader->isAlphaChannelUsed())
				{
					pass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
					pass->setDepthWriteEnabled(false);
				}
			}
			else
				texUnit->setTextureNameAlias("NON_EXISTANT TEXTURE");
			
		}

        
		//check if material in our List has identical properties		
		Ogre::Material* existingMaterial = GetSimilarMaterial(&*tempMaterial);
		
		if (existingMaterial)
        {
			dffMatList.push_back(existingMaterial->getName());
			this->m_matmgr->remove(tempMaterialName);
            
        }
        else
        {
			dffMatList.push_back(tempMaterialName);
			this->AddMaterialToList(&*tempMaterial);
        };
    }

    return dffMatList;
}

MaterialDatabase::MaterialDatabase(Ogre::MaterialManager *matMgr, Ogre::MaterialSerializer* matserializer)
{
    this->m_matmgr = matMgr;
    this->m_matserializer = matserializer;
}

void MaterialDatabase::AddMaterialToList(Ogre::Material* matptr)
{
    this->m_matList.push_back(matptr);
}

void MaterialDatabase::ExportAll(string exportPath)
{
    Crx::File matpath(exportPath, false);
    matpath /= "global.material";

	for (Ogre::Material* matptr : m_matList)
    {
		if (!this->m_matmgr->resourceExists(matptr->getHandle()))
            throw new MyException("Material Resource Does not Exists!");

		if (matptr->getNumTechniques() > 0)
        {

			Ogre::Technique* matTechnique = matptr->getTechnique(0);

            if (matTechnique->getNumPasses() > 0)
            {

                Ogre::Pass* matPass = matTechnique->getPass(0);

                if (matPass->getNumTextureUnitStates() > 0)
                {

                    Ogre::TextureUnitState* matTexUnit = matPass->getTextureUnitState(0);
                    string matTexName = matTexUnit->getTextureName();
                    //boost::trim(matTexName);
					
					// rename the material to use .dds textures
                    matTexUnit->setTextureName(matTexName + ".dds");
                }
            }
        }

		this->m_matserializer->queueForExport(Ogre::MaterialPtr(matptr));
    }
    /*
    this->m_matserializer->exportQueued(path.string());
    this->m_matserializer->clearQueue();
    */
    // TODO delete the material now


    // export all materials into the same file
    this->m_matserializer->exportQueued(matpath.string());
    this->m_matserializer->clearQueue();
}



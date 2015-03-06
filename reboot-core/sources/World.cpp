#include "World.h"

#include <string>
#include <fstream>
#include <OgreEntity.h>
#include <OgreViewport.h>
#include <OgreMesh.h>
#include <OgreLodStrategy.h>
#include <OgreVector3.h>
#include <OgreSceneManager.h>
#include <OgreMeshManager.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

namespace reboot {
using namespace std;
using boost::lexical_cast;

template<> World *Ogre::Singleton<World>::msSingleton = 0;

void fix_gta_coord(Ogre::Quaternion& quat)
{
    swap(quat.y, quat.z);
    Ogre::Radian angle;
    Ogre::Vector3 axis;
    quat.ToAngleAxis(angle, axis);
    //swap(axis.y, axis.z);
    axis = Ogre::Quaternion(Ogre::Degree(-90), Ogre::Vector3::UNIT_X) * axis;
    quat.FromAngleAxis(angle, axis);
}

World::World(Ogre::SceneManager* sceneMgr, Ogre::Camera* camera, Ogre::Viewport* viewPort, Ogre::Root* root)
{
    this->m_sceneMgr = sceneMgr;
    this->mCamera = camera;
    this->mViewPort = viewPort;
    this->mRoot = root;
}

bool World::frameStarted(const Ogre::FrameEvent& evt)
{
    mWater->Update(evt.timeSinceLastFrame);
    return true;
}

void World::Initialize()
{
    // Set some camera params
    mCamera->setFarClipDistance(99999*6);
    mCamera->setOrientation(Ogre::Quaternion(0.487431, -0.0391184, 0.869485, 0.0697797));

    mRoot->addFrameListener(this);

    mSkyBox = new Skybox(m_sceneMgr, mCamera);
    mSkyBox->Initialize();

    mWater = new Water(m_sceneMgr, mCamera);
    mWater->SetViewport(mViewPort);
    mWater->EnableHydrax(false);
    mWater->Initialize();
	Ogre::MeshManager::getSingleton().createManual("NULL", "General");

    //m_sceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);
}

void World::LoadSceneFile(string sceneFile)
{

    Ogre::LogManager::getSingleton().setLogDetail(Ogre::LL_LOW);
    Ogre::SceneNode* worldNode = m_sceneMgr->getRootSceneNode()->createChildSceneNode();
    worldNode->rotate(Ogre::Vector3::UNIT_X, Ogre::Degree(-90.0f));

    string meshName;
    float fDrawDistance;
    bool bIsLod;
    Ogre::Vector3 position;
    Ogre::Quaternion rotation;

    ifstream sceneStream(sceneFile);
    // skip the title and the table title of the file
    string strline;

    int i = 0;
    while (!sceneStream.eof())
    {
        getline(sceneStream, strline);

        if (strline.length() == 0)
            continue;

        std::istringstream ss(strline);
        std::string token;

        getline(ss, token, ',');
        boost::trim(token);
        meshName = token;

        getline(ss, token, ',');
        boost::trim(token);
        fDrawDistance = lexical_cast<float>(token);

        getline(ss, token, ',');
        boost::trim(token);
        bIsLod = lexical_cast<bool>(token);

        getline(ss, token, ',');
        boost::trim(token);
        position.x = lexical_cast<float>(token);
        getline(ss, token, ',');
        boost::trim(token);
        position.y = lexical_cast<float>(token);
        getline(ss, token, ',');
        boost::trim(token);
        position.z = lexical_cast<float>(token);

        getline(ss, token, ',');
        boost::trim(token);
        rotation.w = lexical_cast<float>(token);
        getline(ss, token, ',');
        boost::trim(token);
        rotation.x = lexical_cast<float>(token);
        getline(ss, token, ',');
        boost::trim(token);
        rotation.y = lexical_cast<float>(token);
        getline(ss, token, ',');
        boost::trim(token);
        rotation.z = lexical_cast<float>(token);

		
        Ogre::MeshPtr mesh;
		Ogre::Entity* entity;
	
		Ogre::Entity* realEntity = m_sceneMgr->createEntity(meshName);
		
		if (fDrawDistance > 300.0f && !bIsLod)
			continue;
		
        if (bIsLod)
        {
            mesh = Ogre::MeshManager::getSingleton().createManual(meshName + "lod" + std::to_string(i), "General"); // empty mesh
			mesh->createManualLodLevel(0, "NULL");
			mesh->createManualLodLevel(700, meshName);
			mesh->_setBounds( realEntity->getMesh()->getBounds());
			mesh->_setBoundingSphereRadius(realEntity->getMesh()->getBoundingSphereRadius());
			mesh->load();
			entity->setRenderingDistance(fDrawDistance);
			entity = m_sceneMgr->createEntity(mesh->getName());
        }
        else
        {
            mesh = Ogre::MeshManager::getSingleton().getByName(meshName);
			entity = m_sceneMgr->createEntity(meshName);
			entity->setRenderingDistance(fDrawDistance);
        }
        m_sceneMgr->destroyEntity(realEntity);


        Ogre::SceneNode* sceneNode = worldNode->createChildSceneNode();
        sceneNode->attachObject(entity);

        fix_gta_coord(rotation);

        sceneNode->rotate(rotation);
        sceneNode->setPosition(position);
        i++;
    }

    worldNode->setScale(5.0f, 5.0f, 5.0f);

    sceneStream.close();
    Ogre::LogManager::getSingleton().setLogDetail(Ogre::LL_NORMAL);
}

}

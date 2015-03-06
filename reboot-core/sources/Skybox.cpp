#include "Skybox.h"
#include <OgreLight.h>
#include <OgreSceneManager.h>

namespace reboot {
	
	#define _def_SkyBoxNum 5
	
	Ogre::String mSkyBoxes[_def_SkyBoxNum] = {
		"Sky/ClubTropicana",
		"Sky/Stormy",
		"Sky/EarlyMorning",
		"Sky/Evening",
		"Sky/Clouds"
	};
	
	Ogre::Vector3 mSunPosition[_def_SkyBoxNum] = {
		Ogre::Vector3(0,10000,-90000),
		Ogre::Vector3(0,10000,-90000)/2.5,
		Ogre::Vector3(13000,0,120000)/3,
		Ogre::Vector3(-50000,-5000,50000),
		Ogre::Vector3(0,0,0)
	};
	
	Ogre::Vector3 mSunColor[_def_SkyBoxNum] = {
		Ogre::Vector3(1, 0.9, 0.6)/5,
		Ogre::Vector3(0.75, 0.65, 0.45)/2,
		Ogre::Vector3(1,0.6,0.4),
		Ogre::Vector3(1,0.4,0.1),
		Ogre::Vector3(0,0,0)
	};

Skybox::Skybox(Ogre::SceneManager* scenemgr, Ogre::Camera* camera)
{
	m_sceneMgr = scenemgr;
}


void Skybox::Initialize()
{
	m_sceneMgr->setSkyBox(true, mSkyBoxes[0], 99999*3, true);
	
	// create some light
	Ogre::Light *mLight = m_sceneMgr->createLight("Light0");
	mLight->setPosition(mSunPosition[4]);
	mLight->setDiffuseColour(1, 1, 1);
	mLight->setSpecularColour(mSunColor[1].x,mSunColor[1].y,mSunColor[1].z);
	mLight->setCastShadows(true);
}

void Skybox::Update()
{

}

}
#ifndef __WORLD_H__
#define __WORLD_H__

#include <iosfwd>
#include <vector>
#include <OgreFrameListener.h>
#include <OgreSingleton.h>
#include <OgrePrerequisites.h>
#include "Water.h"
#include "Skybox.h"

using std::vector;

namespace reboot
{
class World : public Ogre::FrameListener
{
public:
	World(Ogre::SceneManager* sceneMgr, Ogre::Camera* camera, Ogre::Viewport* viewPort, Ogre::Root* root);
	
	// frame listener overrides
	virtual bool frameStarted(const Ogre::FrameEvent& evt);
	
	void Initialize();
	void LoadSceneFile(std::string sceneFile);
private:
	Ogre::SceneManager* m_sceneMgr;
	Ogre::Camera* mCamera;
	Ogre::Viewport* mViewPort;
	Ogre::Root* mRoot;
	
	Water* mWater;
	Skybox* mSkyBox;
	
	vector<Ogre::SceneNode*> m_itemSceneNodes;
};
}
#endif

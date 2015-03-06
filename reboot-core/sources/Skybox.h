#ifndef __SKYBOX_H__
#define __SKYBOX_H__

#include <OgrePrerequisites.h>

namespace reboot
{
class Skybox
{
public:
	Skybox(Ogre::SceneManager* scenemgr, Ogre::Camera* camera);
	void Initialize();
	void Update();
private:
	Ogre::Camera* m_camera;
	Ogre::SceneManager* m_sceneMgr;
};
}
#endif
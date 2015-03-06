#ifndef __WATER_H__
#define __WATER_H__

#include <iosfwd>
#include <OgreSharedPtr.h>
#include <OgrePrerequisites.h>

namespace Hydrax{
	class Hydrax;
}

using std::string;

namespace reboot
{
	class Water 
	{
	public:
		Water(Ogre::SceneManager* scenemgr, Ogre::Camera* camera);
		
		void SetCamara(Ogre::Camera* camera);
		void SetViewport(Ogre::Viewport* viewport);
		void EnableHydrax(bool bHydrax);
		
		void Initialize();
		void Update(const Ogre::Real timeSinceLastUpdate);
	private:
		
		bool m_bUseHydrax;
		Hydrax::Hydrax* m_hydrax;
		
		Ogre::Camera* m_camera;
		Ogre::Viewport* m_viewport;
		Ogre::SceneManager* m_scenemgr;
	};
}

#endif
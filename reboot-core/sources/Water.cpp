#include "Water.h"
#include <../CommunityHydrax/include/Noise/FFT/FFT.h>

#include <OgreSceneManager.h>
#include <OgreViewport.h>

#include <Hydrax/Hydrax.h>
#include <Hydrax/MaterialManager.h>
#include <Hydrax/Noise/Perlin/Perlin.h>
#include <Hydrax/Modules/SimpleGrid/SimpleGrid.h>
#include <Hydrax/Modules/ProjectedGrid/ProjectedGrid.h>

#define _def_PGComplexity 32

Hydrax::Module::ProjectedGrid::Options mPGOptions[] = {
    Hydrax::Module::ProjectedGrid::Options(_def_PGComplexity),
    Hydrax::Module::ProjectedGrid::Options(_def_PGComplexity, 45.5f, 7.0f, false),
    Hydrax::Module::ProjectedGrid::Options(_def_PGComplexity, 32.5f, 7.0f, false),
    Hydrax::Module::ProjectedGrid::Options(_def_PGComplexity, 32.5f, 7.0f, false),
    Hydrax::Module::ProjectedGrid::Options(_def_PGComplexity, 20.0f, 7.0f, false)
};

Hydrax::Noise::Perlin::Options mPerlinOptions[] = {
    Hydrax::Noise::Perlin::Options(8, 0.085f, 0.49f, 1.4f, 1.27),
    Hydrax::Noise::Perlin::Options(8, 0.085f, 0.49f, 1.4f, 1.27),
    Hydrax::Noise::Perlin::Options(8, 0.085f, 0.49f, 1.4f, 1.27),
    Hydrax::Noise::Perlin::Options(8, 0.075f, 0.49f, 1.4f, 1.27),
    Hydrax::Noise::Perlin::Options(8, 0.085f, 0.49f, 1.4f, 1.27)
};

namespace reboot {

Water::Water(Ogre::SceneManager* scenemgr, Ogre::Camera* camera)
{
    m_scenemgr = scenemgr;
    m_camera = camera;

    EnableHydrax(false);
    m_hydrax = nullptr;
}

void Water::EnableHydrax(bool bHydrax)
{
    m_bUseHydrax = bHydrax;
}

void Water::SetViewport(Ogre::Viewport* viewport)
{
    m_viewport = viewport;
}

#include <OgreOverlay.h>
void Water::SetCamara(Ogre::Camera* camera)
{
    m_camera = camera;
}

void Water::Initialize()
{
    assert(m_viewport != nullptr && m_camera != nullptr);

    if (!m_viewport)
        m_viewport = m_camera->getViewport();

    if (m_bUseHydrax)
    {
        m_hydrax = new Hydrax::Hydrax(m_scenemgr, m_camera);

        m_hydrax->setRttOptions(
            Hydrax::RttOptions(// Reflection tex quality
                Hydrax::TEX_QUA_1024,
                // Refraction tex quality
                Hydrax::TEX_QUA_1024,
                // Depth tex quality
                Hydrax::TEX_QUA_1024));

        // Set components
		
        m_hydrax->setComponents(
            static_cast<Hydrax::HydraxComponent>(
                    Hydrax::HYDRAX_COMPONENT_DEPTH ));

                  
        Hydrax::Module::ProjectedGrid *mModule =
            new Hydrax::Module::ProjectedGrid(
            m_hydrax,
            new Hydrax::Noise::Perlin(),
            Ogre::Plane(Ogre::Vector3(0,1,0), Ogre::Vector3(0,0,0)),
            Hydrax::Module::ProjectedGrid::Options(_def_PGComplexity, 5.5f, 7.0f, false)
        );

        m_hydrax->setModule(static_cast<Hydrax::Module::Module*>(mModule));
        //m_hydrax->setWaterColor(Ogre::Vector3(1.0f,1.0f, 1.0f));
        m_hydrax->setShaderMode(Hydrax::MaterialManager::SM_CG);
        //m_hydrax->loadCfg("water.hdx");
        // Create water
		m_hydrax->setFullReflectionDistance(99999997952.0);
		m_hydrax->setGlobalTransparency(0);
		m_hydrax->setDepthLimit(50);
		m_hydrax->setDepthColor(Ogre::Vector3(0.04,0.135,0.185));
		m_hydrax->create();
        m_hydrax->setPosition(Ogre::Vector3(0,0,0));
        m_hydrax->setPlanesError(37.5);
        
        //m_hydrax->setSunPosition(mSunPosition[mCurrentSkyBox]);
        //m_hydrax->setSunColor(mSunColor[mCurrentSkyBox]);
        m_hydrax->setNormalDistortion(0.025);
        
        m_hydrax->setSmoothPower(5);
        m_hydrax->setCausticsScale(12);
        m_hydrax->setGlobalTransparency(0.1);
        
        
        m_hydrax->setPolygonMode(0);
    }
    else
    {
    }
}

void Water::Update(const Ogre::Real timeSinceLastUpdate)
{
    if (m_bUseHydrax)
        m_hydrax->update(timeSinceLastUpdate);
    else
    {
    }
}
}

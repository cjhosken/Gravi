//
// Created by s5605094 on 14/04/25.
//

#include <pxr/usd/sdf/path.h>

#include "Light.h"

#include <pxr/imaging/hd/sceneDelegate.h>

PXR_NAMESPACE_OPEN_SCOPE

//-------------------------------------------------------------------------
// Construction / Destruction
//-------------------------------------------------------------------------

HdGraviLight::HdGraviLight(SdfPath const& _id, TfToken const& _lightType) : HdLight(_id), m_lightType(_lightType) {}

//-------------------------------------------------------------------------
// Hydra Light Interface
//-------------------------------------------------------------------------

void HdGraviLight::Sync(HdSceneDelegate* _sceneDelegate, HdRenderParam* _renderParam, HdDirtyBits* _dirtyBits)
{
    HD_TRACE_FUNCTION();
    HF_MALLOC_TAG_FUNCTION();

    const SdfPath& id = GetId();


    //std::cout << "\n=== Syncing Light: " << id.GetText() << " ===" << std::endl;
    //std::cout << "Dirty bits: " << bits << std::endl;
    //std::cout << "Light type: " << m_lightType << std::endl;

    HdDirtyBits bits = *_dirtyBits;
    *_dirtyBits = HdChangeTracker::Clean;

    if (bits & HdChangeTracker::DirtyParams) {
        m_transform = _sceneDelegate->GetTransform(id);
        // Required light parameters
        m_intensity = _sceneDelegate->GetLightParamValue(id, HdLightTokens->intensity)
            .GetWithDefault(1.0f);
        //std::cout << "Intensity: " << m_intensity << std::endl;

        m_exposure = _sceneDelegate->GetLightParamValue(id, HdLightTokens->exposure)
            .GetWithDefault(0.0f);
        //std::cout << "Exposure: " << m_exposure << std::endl;

        m_color = _sceneDelegate->GetLightParamValue(id, HdLightTokens->color)
            .GetWithDefault(GfVec3f(1.0f));
        //std::cout << "Color: (" << m_color[0] << ", "
         //                     << m_color[1] << ", "
        //                      << m_color[2] << ")" << std::endl;

        // Light-type specific parameters
        if (m_lightType == HdPrimTypeTokens->rectLight ||
            m_lightType == HdPrimTypeTokens->diskLight) {
            m_width = _sceneDelegate->GetLightParamValue(id, HdLightTokens->width)
                .GetWithDefault(1.0f);
            //std::cout << "Width: " << m_width << std::endl;

            m_height = _sceneDelegate->GetLightParamValue(id, HdLightTokens->height)
                .GetWithDefault(1.0f);
            //std::cout << "Height: " << m_height << std::endl;
        }
        else if (m_lightType == HdPrimTypeTokens->distantLight) {
            m_angle = _sceneDelegate->GetLightParamValue(id, HdLightTokens->angle)
                .GetWithDefault(0.53f);
            //std::cout << "Angle: " << m_angle << " radians ("
            //         << GfRadiansToDegrees(m_angle) << " degrees)" << std::endl;
        }
        else if (m_lightType == HdPrimTypeTokens->sphereLight) {
            m_radius = _sceneDelegate->GetLightParamValue(id, HdLightTokens->radius)
                .GetWithDefault(0.5f);
            //std::cout << "Radius: " << m_radius << std::endl;
        }
        else if (m_lightType == HdPrimTypeTokens->cylinderLight) {
            m_radius = _sceneDelegate->GetLightParamValue(id, HdLightTokens->radius)
                .GetWithDefault(0.5f);
            //std::cout << "Cylinder Radius: " << m_radius << std::endl;

            m_length = _sceneDelegate->GetLightParamValue(id, HdLightTokens->length)
                .GetWithDefault(1.0f);
            //std::cout << "Cylinder Length: " << m_length << std::endl;
        }
    }

    //std::cout << "=== Sync Complete ===\n" << std::endl;
}

HdDirtyBits HdGraviLight::GetInitialDirtyBitsMask() const { return HdLight::AllDirty; }

//-------------------------------------------------------------------------
// Light Properties
//-------------------------------------------------------------------------

TfToken HdGraviLight::GetLightType() const { return m_lightType; }

float HdGraviLight::GetIntensity() const { return m_intensity; }

float HdGraviLight::GetExposure() const { return m_exposure; }

GfVec3f HdGraviLight::GetColor() const { return m_color; }

GfMatrix4d HdGraviLight::GetTransform() const { return m_transform; }

float HdGraviLight::GetAngle() const { return m_angle; }

float HdGraviLight::GetWidth() const { return m_width; }

float HdGraviLight::GetHeight() const { return m_height; }

float HdGraviLight::GetRadius() const { return m_radius; }

float HdGraviLight::GetLength() const { return m_length; }

PXR_NAMESPACE_CLOSE_SCOPE
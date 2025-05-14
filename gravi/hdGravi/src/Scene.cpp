///
/// @file Scene.cpp
/// @brief The Gravi rendering scene. Data is fed into here to be rendered.

#include <pxr/base/gf/ray.h>
#include <pxr/base/gf/rotation.h>

#include "HdGraviPrim.h"
#include "Light.h"
#include "gravityWell.h"

#include "Scene.h"

PXR_NAMESPACE_OPEN_SCOPE

//-------------------------------------------------------------------------
// Construction / Destruction
//-------------------------------------------------------------------------

HdGraviScene::HdGraviScene(HdRenderIndex *_index)
{
    const SdfPathVector rprimIds = _index->GetRprimIds();

    for (const SdfPath &rprimId : rprimIds)
    {
        // Retrieve the Rprim object from the render index using the rprimId
        const HdRprim* rprim = _index->GetRprim(rprimId);

        // If not, treat it as a basic HdGraviPrim (if it is one)
        if (auto prim = dynamic_cast<const HdGraviPrim*>(rprim)) {
            m_prims.push_back(prim);
        }

        if (auto prim = dynamic_cast<const HdGravityWell*>(rprim)) {
            m_wells.push_back(prim);
        }
    }
}

//-------------------------------------------------------------------------
// Scene Operations
//-------------------------------------------------------------------------

void HdGraviScene::BuildBVH()
{
    if (m_prims.empty()) return;

    std::vector<const HdGraviPrim*> prims = m_prims;

    m_bvhRoot = BuildBVHRecursive(prims);
}

HitData HdGraviScene::Intersect(const GfRay& _ray, int _numBounces, double _lightStepSize, double _minLightStepSize, double _maxLightStepSize, double _maxLightDistance, int _maxLightSteps)
{
    GfRay newRay = _ray;

    IntersectData closestIT {
        std::numeric_limits<float>::infinity(),
            GfVec3f(0.0f, 0.0f, 0.0f),
            GfVec4f(0.0f, 0.0f, 0.0f, 1.0f),
        0
    };

    if (!m_wells.empty())
    {
        closestIT = IntersectGravity(newRay, closestIT, _lightStepSize, _minLightStepSize, _maxLightStepSize, _maxLightDistance, _maxLightSteps);
    } else {
        closestIT = IntersectBVH(newRay, m_bvhRoot, closestIT);
    }

    if (closestIT.m_t > 0 && closestIT.m_t < std::numeric_limits<float>::infinity())
    {
        return HitData{
            true,
            GetCd(closestIT, newRay, _numBounces, _lightStepSize, _minLightStepSize, _maxLightStepSize, _maxLightDistance, _maxLightSteps),
            closestIT.m_N,
            GfVec3f(newRay.GetPoint(closestIT.m_t)),
            closestIT.m_t,
            closestIT.m_id
        };
    }
    else
    {
        return HitData{
            false,
            GfVec4f(0.0f, 0.0f, 0.0f, 1.0f),
            GfVec3f(0.0f, 0.0f, 0.0f),
            GfVec3f(0.0f, 0.0f, 0.0f),
            std::numeric_limits<float>::infinity()
        };
    }
}

std::vector<const HdGraviPrim*> HdGraviScene::GetRprims() { return m_prims; }

//-------------------------------------------------------------------------
// Light Management
//-------------------------------------------------------------------------

void HdGraviScene::SetLights(std::vector<const HdGraviLight*>& _lights) { m_lights = _lights; }

void HdGraviScene::ClearLights() { m_lights.clear(); }

//-------------------------------------------------------------------------
// BVH Construction
//-------------------------------------------------------------------------

BVHNode *HdGraviScene::BuildBVHRecursive(std::vector<const HdGraviPrim *> &_prims)
{
    if (_prims.size() == 1)
    {
        // Create a leaf node with the single mesh
        BVHNode *node = new BVHNode();
        node->m_bbox = _prims[0]->GetBVH();
        node->m_rprim = _prims[0];
        return node;
    }

    // Calculate the bounding box of all meshes in this node
    GfRange3d brange;
    for (const HdGraviPrim *prim : _prims)
    {
        brange.UnionWith(prim->GetBVH().GetBox());
    }

    GfBBox3d bbox(brange);

    // Split the meshes along the middle of their bounding boxes
    // For simplicity, let's split along the X-axis. You could split in other ways.
    // Determine the axis with largest extent
    GfVec3d extents = brange.GetMax() - brange.GetMin();
    int axis = 0; // Default to x-axis
    if (extents[1] > extents[axis]) axis = 1; // y-axis is larger
    if (extents[2] > extents[axis]) axis = 2; // z-axis is larger
    std::sort(_prims.begin(), _prims.end(), [axis](const HdGraviPrim* a, const HdGraviPrim* b) {
        return a->GetBVH().GetBox().GetMin()[axis] < b->GetBVH().GetBox().GetMin()[axis];
    });

    size_t mid = _prims.size() / 2;
    std::vector<const HdGraviPrim *> leftMeshes(_prims.begin(), _prims.begin() + mid);
    std::vector<const HdGraviPrim *> rightMeshes(_prims.begin() + mid, _prims.end());

    // Create internal nodes
    BVHNode *node = new BVHNode();
    node->m_bbox = bbox;
    node->m_left = BuildBVHRecursive(leftMeshes);
    node->m_right = BuildBVHRecursive(rightMeshes);
    return node;
}

//-------------------------------------------------------------------------
// Intersection Testing
//-------------------------------------------------------------------------

static float randomFloat() { return ((float)(rand()) / (float)(RAND_MAX)-0.5f) * 2.0f; }

// Helper function to generate a random vector within a hemisphere
static GfVec3f RandomHemisphereDirection(const GfVec3f &normal)
{
    // Create a random vector in the tangent plane
    GfVec3f randomDirection;

    while (true)
    {
        randomDirection = GfVec3f(randomFloat(), randomFloat(), randomFloat());

        if (randomDirection.GetLength() < 1.0f)
        {
            randomDirection = randomDirection.GetNormalized();
            break;
        }
    }

    // If the normal is pointing upwards (or along a specific axis), make sure the random vector is in the hemisphere
    if (randomDirection * normal < 0.0f)
    {
        randomDirection = -randomDirection; // Reflect the vector to ensure it's in the correct hemisphere
    }

    return randomDirection;
};

IntersectData HdGraviScene::IntersectBVH(const GfRay& _ray, BVHNode *_node, IntersectData _closestIT)
{
    if (!_node) {return _closestIT;}

    if (!_ray.Intersect(_node->m_bbox)) {return _closestIT;} // If node is null, return the closest intersection found so far


    // If it's a leaf node, check intersection with the mesh
    if (_node->IsLeaf())
    {
        GfRay newRay = _ray;
        newRay.Transform(_node->m_rprim->GetTransform().GetInverse());

        IntersectData it = _node->m_rprim->Intersect(newRay);

        // If a valid intersection is found (t >= 0), and it's closer than the previous closest, update
        if (it.m_t >= 0.0 && it.m_t < _closestIT.m_t)
        {
            _closestIT = it;
        }
        return _closestIT; // Return the closest intersection found so far
    }

    // Otherwise, recursively check left and right children
    _closestIT = IntersectBVH(_ray, _node->m_left, _closestIT);
    _closestIT = IntersectBVH(_ray, _node->m_right, _closestIT);

    return _closestIT; // Return the closest intersection after checking both children
}

GfVec4f HdGraviScene::_SampleLight(const HdGraviLight* _light, IntersectData _it, GfVec3d _hitPnt, double _lightStepSize, double _minLightStepSize, double _maxLightStepSize, double _maxLightDistance, int _maxLightSteps)
{
    GfVec4f luminance(0.0f, 0.0f, 0.0f, 1.0f);
    float intensity = _light->GetIntensity();

    if (intensity <= 0.0001f) {
        return luminance;
    }

    // Common light parameters
    float exposure = _light->GetExposure();
    TfToken type = _light->GetLightType();
    GfMatrix4d transform = _light->GetTransform();
    GfVec3f color = _light->GetColor();

    // Calculate final color with exposure
    GfVec3f finalColor = color * intensity * powf(2.0f, exposure);

    // Extract light position and direction from transform
    GfVec3f lightPos(transform.ExtractTranslation());

    GfVec3f lightDir(transform.ExtractRotation().TransformDir(GfVec3f(0, 0, 1))); // Negative Z-axis

    // Vector from hit point to light
    GfVec3f toLight;
    float distance;
    float attenuation = 1.0f;

    auto testShadow = [this, _lightStepSize, _minLightStepSize, _maxLightStepSize, _maxLightDistance, _maxLightSteps](GfRay _ray) -> bool {
        IntersectData closestIT {
            std::numeric_limits<float>::infinity(),
                GfVec3f(0.0f, 0.0f, 0.0f),
                GfVec4f(0.0f, 0.0f, 0.0f, 1.0f),
            0,
            nullptr,
            false
        };

        if (!m_wells.empty())
        {
            closestIT = IntersectGravity(_ray, closestIT,_lightStepSize, _minLightStepSize, _maxLightStepSize, _maxLightDistance, _maxLightSteps);
        } else {
            closestIT = IntersectBVH(_ray, m_bvhRoot, closestIT);
        }

        if (closestIT.m_died) {
            return true;
        }

        return closestIT.m_t > 0.0001 && closestIT.m_t < std::numeric_limits<float>::infinity();
    };

    if (type == HdPrimTypeTokens->light || type == HdPrimTypeTokens->sphereLight) {
        // Point light calculation
        toLight = lightPos - GfVec3f(_hitPnt);
        distance = toLight.GetLength();

        float attenuation = 1.0 / (distance * distance);

        float NdotL = GfMax(0.0f, GfDot(_it.m_N, toLight));

        if (NdotL == 0.0f) {
            return luminance;
        }

        if (testShadow(GfRay(_hitPnt, toLight))) {
            return luminance;
        }

        GfVec3f lightContrib = finalColor * attenuation * NdotL;
        luminance = GfVec4f(lightContrib[0], lightContrib[1], lightContrib[2], 1.0f);
    }
    else if (type == HdPrimTypeTokens->distantLight) {

        // Directional light - constant illumination
        float angle = _light->GetAngle();
        float angularDiameter = angle * 0.5f; // Convert to half-angle

        // Soft directional light approximation
        float NdotL = GfMax(0.0f, GfDot(_it.m_N, lightDir));
        //float softFactor = cosf(angularDiameter * M_PI/180.0f);
        //float lighting = smoothstep(softFactor, 1.0f, NdotL);

        if (NdotL == 0.0f) {
            return luminance;
        }

        if (testShadow(GfRay(_hitPnt, lightDir))) {
            return luminance;
        }

        GfVec3f lum(NdotL);
        luminance = GfVec4f(lum[0], lum[1], lum[2], 1.0f);
    }
    else if (type == HdPrimTypeTokens->domeLight) {
        GfVec3f dir = RandomHemisphereDirection(_it.m_N);

        if (testShadow(GfRay(_hitPnt, dir))) {
            return luminance;
        };

        luminance = GfVec4f(finalColor[0], finalColor[1], finalColor[2], 1.0f);
    }
    else if (type == HdPrimTypeTokens->rectLight || type == HdPrimTypeTokens->diskLight) {
        // Area light sampling
        float width = _light->GetWidth();
        float height = _light->GetHeight();

        // Calculate light plane basis vectors
        GfVec3f right(transform[0][0], transform[0][1], transform[0][2]);
        GfVec3f up(transform[1][0], transform[1][1], transform[1][2]);

        // Sample random point on light surface
        GfVec2f uv(randomFloat() - 0.5f, randomFloat() - 0.5f);
        GfVec3f lightSamplePos = lightPos +
                               (right * width * uv[0]) +
                               (up * height * uv[1]);

        toLight = lightSamplePos - GfVec3f(_hitPnt);
        distance = toLight.GetLength();
        toLight.Normalize();

        float NdotL = GfMax(0.0f, GfDot(_it.m_N, toLight));

        if (NdotL == 0.0f) {
            return luminance;
        }

        if (testShadow(GfRay(_hitPnt, toLight))) {
            return luminance;
        }

        // Area light attenuation
        float area = width * height;
        attenuation = 1.0f / (distance * distance);
        attenuation *= area;

        // Lambertian shading with light normal
        float LdotN = GfMax(0.0f, GfDot(-toLight, lightDir));

        GfVec3f lum = finalColor * attenuation * NdotL * LdotN;
        luminance = GfVec4f(lum[0], lum[1], lum[2], 1.0f);
    }
    else if (type == HdPrimTypeTokens->cylinderLight) {
        // Cylinder light sampling
        float radius = _light->GetWidth() * 0.5f; // Assuming width = diameter
        float length = _light->GetHeight();

        // Sample random point along cylinder
        float t = randomFloat() * length - (length * 0.5f);
        GfVec3f axisDir(transform[2][0], transform[2][1], transform[2][2]);
        GfVec3f lightSamplePos = lightPos + axisDir * t;

        // Sample random point around circumference
        float angle = randomFloat() * 2.0f * M_PI;
        GfVec3f right(transform[0][0], transform[0][1], transform[0][2]);
        GfVec3f up(transform[1][0], transform[1][1], transform[1][2]);
        lightSamplePos += (right * cosf(angle) + up * sinf(angle)) * radius;

        toLight = lightSamplePos - GfVec3f(_hitPnt);
        distance = toLight.GetLength();
        toLight.Normalize();

        float NdotL = GfMax(0.0f, GfDot(_it.m_N, toLight));

        if (NdotL == 0.0f) {
            return luminance;
        }

        if (testShadow(GfRay(_hitPnt, toLight))) {
            return luminance;
        }

        // Cylinder light attenuation
        float area = 2.0f * M_PI * radius * length;
        attenuation = 1.0f / (distance * distance);
        attenuation *= area;

        // Lambertian shading
        GfVec3f lum = finalColor * attenuation * NdotL;
        luminance = GfVec4f(lum[0], lum[1], lum[2], 1.0f);
    }

    return luminance;
}

IntersectData HdGraviScene::IntersectGravity(GfRay& _ray, IntersectData _closestIT, double _lightStepSize, double _minLightStepSize, double _maxLightStepSize, double _maxLightDistance, int _maxLightSteps)
{
    const GfVec3f rayOrigin = GfVec3f(_ray.GetStartPoint());
    GfVec3f dir = GfVec3f(_ray.GetDirection());
    GfVec3f currentPos = rayOrigin;

    double totalDistance = 0;

    struct GravityWell {
        GfVec3d m_position;
        float m_force;
    };

    std::vector<GravityWell> wells;

    for (auto well : m_wells ) {
        wells.push_back({
            well->GetTransform().ExtractTranslation(),
            well->GetForce()
        });
    }

    // Segment stepping through influence zones
    for (int seg = 0; seg < _maxLightSteps; ++seg) {
        bool isInside = false;
        GfVec3f gravity(0.0f);


        for (auto well : wells) {
            GfVec3f toWell = GfVec3f(well.m_position) - currentPos;
            float dist = toWell.GetLength();
            float influenceDist = std::sqrt(std::abs(well.m_force)/0.01f);

            if (dist - influenceDist <= 0.01f) {
                isInside = true;

                auto factor = 1.0f - (dist / influenceDist);

                GfVec3f g = toWell.GetNormalized() * (-well.m_force / toWell.GetLengthSq());
                gravity += g * factor;
            }
        }

        if (isInside) {
            double adaptiveStepSize = _lightStepSize;
            if (gravity.GetLength() > 0.0f) {
                adaptiveStepSize /= std::max(1.0f, gravity.GetLength());
            }

            adaptiveStepSize = std::min(std::max(_minLightStepSize, adaptiveStepSize), _maxLightStepSize);

            // 2. Apply deflection
            dir += gravity * adaptiveStepSize;
            dir.Normalize();

            // 4. Test segment
            _ray = GfRay(currentPos, dir);
            _closestIT = IntersectBVH(_ray, m_bvhRoot, _closestIT);

            currentPos += dir * adaptiveStepSize;
            totalDistance += adaptiveStepSize;

            if (_closestIT.m_t > 0.0001 && _closestIT.m_t <= adaptiveStepSize) {
                _closestIT.m_died = false;
                return _closestIT;
            }

        } else {
            _closestIT = IntersectBVH(_ray, m_bvhRoot, _closestIT);
            _ray = GfRay(currentPos, dir);

            for (auto well : wells) {
                double t;
                float radius = std::sqrt(std::abs(well.m_force)/0.01f);

                bool hit = _ray.Intersect(well.m_position, radius, &t);

                if (hit && t > 0.001 && t < _closestIT.m_t) {
                    currentPos += dir * t;
                    totalDistance += t;
                } else {
                    return _closestIT;
                }
            }
        }

        if (totalDistance >= _maxLightDistance) {
            _closestIT.m_died = true;
            return _closestIT;
        }
    }

    _closestIT.m_died = true;
    return _closestIT;
}

GfVec4f HdGraviScene::GetCd(IntersectData _it, const GfRay &_ray, int _bounces, double _lightStepSize, double _minLightStepSize, double _maxLightStepSize, double _maxLightDistance, int _maxLightSteps)
{
    GfVec4f Cd(0.0, 0.0, 0.0, 1.0);

    GfVec3d hitPnt = _ray.GetPoint(_it.m_t);

    if (_bounces <= 0) {
        return Cd;
    }

    if (dynamic_cast<const HdGravityWell*>(_it.m_prim)) {
        // Special handling for gravity wells
        return GfVec4f(0.0, 0.0, 0.0, 1.0); // Magenta color for visualization
    }

    GfVec4f baseCd = _it.m_Cd;

    if (!m_lights.empty()) {
        for (auto light : m_lights) {
            auto sample = _SampleLight(light, _it, hitPnt, _lightStepSize, _minLightStepSize, _maxLightStepSize, _maxLightDistance, _maxLightSteps);

            GfVec4f diffuse(
                baseCd[0] * sample[0],
                baseCd[1] * sample[1],
                baseCd[2] * sample[2],
                baseCd[3] * sample[3]
            );

            Cd += diffuse;
        }
    } else {
        Cd += baseCd;
    }

    float reflectance = std::clamp(baseCd.GetLength(), 0.0f, 1.0f);

    if (reflectance > 0.0f) {
        GfVec3f randomDirection = RandomHemisphereDirection(_it.m_N);
        GfRay bounceRay(hitPnt, randomDirection);

        HitData bounce = Intersect(bounceRay, _bounces - 1, _lightStepSize, _minLightStepSize, _maxLightStepSize, _maxLightStepSize, _maxLightDistance);

        GfVec4f bounceCd(0.0f, 0.0f, 0.0f, 1.0f);

        if (bounce.m_hit) {
            bounceCd = bounce.m_Cd;
        }

        float invLaw = 1 / (bounce.m_z * bounce.m_z);

        Cd += bounceCd * reflectance * std::min(invLaw, 1.0f);
    }

    return Cd;
}

PXR_NAMESPACE_CLOSE_SCOPE
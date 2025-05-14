/// @file Scene.h
/// @brief Core scene representation for Gravi renderer
///
/// Maintains complete scene representation including:
/// - Geometry storage and management
/// - BVH acceleration structure
/// - Ray intersection testing
/// - Light transport simulation
/// - Gravity well effects
/// - Scene traversal operations
///
/// @author Christopher Hosken
/// @version 1.0
/// @date 07/05/2025 Pre-release cleanup
/// @ingroup hdGravi

#ifndef HDGRAVI_HDGRAVISCENE_H_
#define HDGRAVI_HDGRAVISCENE_H_

#include <vector>

#include "Mesh.h"
#include "Light.h"
#include "HdGraviPrim.h"
#include "HdGravityWell.h"

PXR_NAMESPACE_OPEN_SCOPE

//-----------------------------------------------------------------------------
/// @struct BVHNode
/// @brief Node in Bounding Volume Hierarchy acceleration structure
///
/// Represents a node in the BVH acceleration structure with:
/// - Bounding volume containing child geometry
/// - Primitive reference for leaf nodes
/// - Child node pointers for internal nodes
//-----------------------------------------------------------------------------
struct BVHNode {
    GfBBox3d m_bbox;            ///< Bounding box containing child geometry
    const HdGraviPrim* m_rprim; ///< Primitive reference (leaf nodes only)
    BVHNode* m_left = nullptr;  ///< Left child node
    BVHNode* m_right = nullptr; ///< Right child node

    /// @brief Check if node is a leaf
    /// @return True if node contains a primitive (no children)
    [[nodiscard]] bool IsLeaf() const {
        return m_rprim != nullptr;
    }
};

//-----------------------------------------------------------------------------
/// @class HdGraviScene
/// @brief Core scene representation and renderer for Gravi
///
/// Maintains all renderable primitives and implements:
/// - BVH acceleration structure construction
/// - Ray intersection testing
/// - Scene traversal operations
/// - Primitive management
/// - Light transport simulation
/// - Gravity well effects
//-----------------------------------------------------------------------------
class HdGraviScene final
{
public:
    //-------------------------------------------------------------------------
    /// @name Construction / Destruction
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Default constructor
    HdGraviScene() = default;

    /// @brief Construct with render index
    /// @param _index Render index containing scene data
    explicit HdGraviScene(HdRenderIndex* _index);

    /// @}

    //-------------------------------------------------------------------------
    /// @name Scene Operations
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Build/rebuild the BVH acceleration structure
    ///
    /// Constructs an optimized BVH for efficient ray intersection
    void BuildBVH();

    /// @brief Intersect a ray with the scene
    /// @param _ray Ray to test
    /// @param _numBounces Number of bounces remaining
    /// @param _lightStepSize Initial light sampling step size
    /// @param _minLightStepSize Minimum light sampling step size
    /// @param _maxLightStepSize Maximum light sampling step size
    /// @param _maxLightDistance Maximum light sampling distance
    /// @param _maxLightSteps Maximum number of light sampling steps
    /// @return Hit data including color, normal and position
    [[nodiscard]] HitData Intersect(const GfRay& _ray,
                                   int _numBounces,
                                   double _lightStepSize,
                                   double _minLightStepSize,
                                   double _maxLightStepSize,
                                   double _maxLightDistance,
                                   int _maxLightSteps);

    /// @brief Get all primitives in the scene
    /// @return Vector of primitive pointers
    [[nodiscard]] std::vector<const HdGraviPrim*> GetRprims();

    /// @}

    //-------------------------------------------------------------------------
    /// @name Light Management
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Set scene lights
    /// @param _lights Vector of light pointers
    void SetLights(std::vector<const HdGraviLight*>& _lights);

    /// @brief Clear all lights from scene
    void ClearLights();

    /// @}

private:
    //-------------------------------------------------------------------------
    /// @name BVH Construction
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Recursive BVH builder
    /// @param _prims Primitives to include in subtree
    /// @return Root node of constructed subtree
    static BVHNode* BuildBVHRecursive(std::vector<const HdGraviPrim*>& _prims);

    /// @}

    //-------------------------------------------------------------------------
    /// @name Intersection Testing
    //-------------------------------------------------------------------------
    /// @{

    /// @brief Intersect ray with BVH subtree
    /// @param _ray Ray to test
    /// @param _node Current BVH node
    /// @param _closestIT Current closest intersection
    /// @return Updated closest intersection data
    IntersectData IntersectBVH(const GfRay& _ray,
                             BVHNode* _node,
                             IntersectData _closestIT);

    /// @brief Sample light contribution
    /// @param _light Light to sample
    /// @param _it Intersection data
    /// @param _hitPnt Hit position
    /// @param _lightStepSize Initial light sampling step size
    /// @param _minLightStepSize Minimum light sampling step size
    /// @param _maxLightStepSize Maximum light sampling step size
    /// @param _maxLightDistance Maximum light sampling distance
    /// @param _maxLightSteps Maximum number of light sampling steps
    /// @return Light contribution
    GfVec4f _SampleLight(const HdGraviLight* _light,
                        IntersectData _it,
                        GfVec3d _hitPnt,
                        double _lightStepSize,
                        double _minLightStepSize,
                        double _maxLightStepSize,
                        double _maxLightDistance,
                        int _maxLightSteps);

    /// @brief Intersect ray with gravity wells
    /// @param _ray Ray to test
    /// @param _closestIT Current closest intersection
    /// @param _lightStepSize Initial light sampling step size
    /// @param _minLightStepSize Minimum light sampling step size
    /// @param _maxLightStepSize Maximum light sampling step size
    /// @param _maxLightDistance Maximum light sampling distance
    /// @param _maxLightSteps Maximum number of light sampling steps
    /// @return Updated intersection data
    IntersectData IntersectGravity(GfRay& _ray,
                                 IntersectData _closestIT,
                                 double _lightStepSize,
                                 double _minLightStepSize,
                                 double _maxLightStepSize,
                                 double _maxLightDistance,
                                 int _maxLightSteps);

    /// @brief Get color at intersection
    /// @param _it Intersection data
    /// @param _ray Incident ray
    /// @param _depth Recursion depth
    /// @param _lightStepSize Initial light sampling step size
    /// @param _minLightStepSize Minimum light sampling step size
    /// @param _maxLightStepSize Maximum light sampling step size
    /// @param _maxLightDistance Maximum light sampling distance
    /// @param _maxLightSteps Maximum number of light sampling steps
    /// @return Surface color
    GfVec4f GetCd(IntersectData _it,
                 const GfRay& _ray,
                 int _depth,
                 double _lightStepSize,
                 double _minLightStepSize,
                 double _maxLightStepSize,
                 double _maxLightDistance,
                 int _maxLightSteps);

    /// @}

    //-------------------------------------------------------------------------
    /// @name Private Members
    //-------------------------------------------------------------------------
    /// @{

    std::vector<const HdGraviPrim*> m_prims;    ///< All scene primitives
    std::vector<const HdGravityWell*> m_wells;  ///< All gravity wells
    std::vector<const HdGraviLight*> m_lights;  ///< All scene lights
    BVHNode* m_bvhRoot = nullptr;               ///< Root of BVH structure

    /// @}
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // HDGRAVI_HDGRAVISCENE_H_
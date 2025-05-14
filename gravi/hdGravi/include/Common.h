/// @file Common.h
/// @brief Common data structures and definitions for the Gravi renderer
///
/// This header contains shared types and structures used throughout Gravi,
/// including ray intersection results and shading data.
///
/// @author Christopher Hosken
/// @version 1.0
/// @date 07/05/2025 Pre-release cleanup
/// @ingroup hdGravi

#ifndef HDGRAVI_COMMON_H_
#define HDGRAVI_COMMON_H_

#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec4f.h>

PXR_NAMESPACE_OPEN_SCOPE

//-----------------------------------------------------------------------------
/// @struct HitData
/// @brief Complete shading data for a ray hit
///
/// Contains all geometric and shading information required for surface shading,
/// including surface properties and hit location data.
//-----------------------------------------------------------------------------
struct HitData
{
    bool     m_hit;  ///< Whether the ray hit a surface
    GfVec4f  m_Cd;   ///< Color (RGB) and opacity (A) at hit point
    GfVec3f  m_N;    ///< Surface normal (world space)
    GfVec3f  m_P;    ///< Hit position (world space)
    float    m_z;    ///< Ray depth (parametric t-value)
    int32_t  m_id;   ///< Primitive identifier
};

//-----------------------------------------------------------------------------
/// @struct IntersectData
/// @brief Minimal intersection data for ray hits
///
/// Contains essential data for intersection testing and visibility calculations,
/// optimized for fast ray traversal.
//-----------------------------------------------------------------------------
struct IntersectData
{
    float        m_t;    ///< Ray distance to hit (parametric t-value)
    GfVec3f      m_N;    ///< Surface normal (world space)
    GfVec4f      m_Cd;   ///< Base color (RGB) at hit point
    int32_t      m_id;   ///< Primitive identifier
    const HdRprim* m_prim;  ///< Pointer to the hit primitive
    bool m_died = false; ///< Flag indicating ray termination
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // HDGRAVI_COMMON_H_
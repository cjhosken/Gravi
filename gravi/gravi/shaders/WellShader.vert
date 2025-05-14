#version 410 core  // Specifies the GLSL version (4.10) and the core profile

// Input vertex attribute at location 0: a 3D position vector
layout(location = 0) in vec3 position;

// Uniform 4x4 matrix for the view transformation (camera position and orientation)
uniform mat4 view;

// Uniform 4x4 matrix for the projection transformation (perspective or orthographic)
uniform mat4 proj;

// Uniform 4x4 matrix for the model transformation (object's position, rotation, and scale)
uniform mat4 model;

void main()
{
        // Apply the full transformation to the vertex:
        // 1. Model matrix: Transforms the object from local space (object space) to world space.
        // 2. View matrix: Transforms from world space to camera (view) space.
        // 3. Projection matrix: Projects the transformed coordinates from camera space to clip space.
        // Finally, gl_Position is the result of this combined transformation.
        gl_Position = proj * view * model * vec4(position, 1.0);
}

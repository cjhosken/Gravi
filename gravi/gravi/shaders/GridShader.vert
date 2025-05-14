#version 410 core  // Specifies the GLSL version (4.10) and the core profile

// Input vertex attribute at location 0: a 3D position vector
layout(location = 0) in vec3 position;

// Uniform 4x4 matrix for the view transformation (camera position and orientation)
uniform mat4 view;

// Uniform 4x4 matrix for the projection transformation (e.g., perspective or orthographic)
uniform mat4 proj;

void main()
{
    // Convert the 3D position to a 4D homogeneous coordinate (w = 1.0),
    // then apply the view and projection transformations to compute the final position
    // in clip space. The order is important: first view, then projection.
    gl_Position = proj * view * vec4(position, 1.0);
}

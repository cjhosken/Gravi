#version 410 core  // Specifies the GLSL version (4.10) and the core profile

// Define a uniform variable 'color' of type vec4 (4-component vector: RGBA)
// This value is passed in from the application and is the same for all fragments
uniform vec4 color;

// Define the output of the fragment shader
out vec4 fragColor;

void main()
{
    // Set the output fragment color to the uniform 'color'
    fragColor = color;
}

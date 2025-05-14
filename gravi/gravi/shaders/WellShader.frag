#version 410 core  // Specifies the GLSL version (4.10) and the core profile

out vec4 fragColor;  // The output color for the fragment

void main()
{
    // Get the texture coordinates of the current fragment within the point
    // 'gl_PointCoord' gives coordinates ranging from (0,0) to (1,1) for the current point.
    // Transform these coordinates to a range of (-1, 1) for easier distance calculation.
    vec2 coord = gl_PointCoord * 2.0 - 1.0;

    // Calculate the distance squared from the center of the point (0,0)
    // Using dot product (coord.x * coord.x + coord.y * coord.y) gives the squared distance.
    float dist = dot(coord, coord);

    // Check if the fragment is inside the circle with radius 1 (the point shape)
    if (dist <= 1.0)
    {
        // Compute the Fresnel effect based on the distance
        // smoothstep creates a smooth transition between two values, giving a gradient effect.
        // The closer the fragment is to the center (lower dist), the more intense the effect.
        float fresnel = smoothstep(0.7, 1.0, dist);

        // Mix between black (vec3(0.0)) and white (vec3(1.0)) based on the Fresnel value
        // This creates a glowing effect where the center is black (or dark) and the edges become white
        vec3 color = mix(vec3(0.0), vec3(1.0), fresnel);

        // Set the fragment color with full opacity (alpha = 1.0)
        fragColor = vec4(color, 1.0);
    }
    else
    {
        // If the fragment is outside the circle (dist > 1.0), discard it
        // This avoids drawing fragments outside the defined point shape.
        discard;
    }
}

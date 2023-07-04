#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D loadingTex;
uniform float near_plane;
uniform float far_plane;

void main()
{    
    vec3 color = texture(loadingTex, TexCoords).rgb;
    // FragColor = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0); // perspective
    FragColor = vec4(color, 1.0); // orthographic
}



#version 330 core

in vec2 TexCoords;

uniform sampler2D diffuseTexture;

void main()
{   
    float alpha = texture(diffuseTexture, TexCoords).a;
    if (alpha == 0.0){
        discard;
    }
    gl_FragDepth = gl_FragCoord.z;
}
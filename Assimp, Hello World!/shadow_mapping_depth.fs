#version 330 core

in vec2 TexCoords;

uniform sampler2D diffuseTexture;
uniform float soglia;

void main()
{   
    float alpha = texture(diffuseTexture, TexCoords).a;
    if (alpha < soglia){
        discard;
    }
    gl_FragDepth = gl_FragCoord.z;
}
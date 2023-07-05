#version 330 core

in vec2 TexCoords;

uniform sampler2D diffuseTexture;
uniform float soglia;
uniform float alpha;

void main()
{   
    float alphaBase = texture(diffuseTexture, TexCoords).a;
    alphaBase = alpha * alphaBase;
    if (alphaBase < soglia){
        discard;
    }
    gl_FragDepth = gl_FragCoord.z;
}
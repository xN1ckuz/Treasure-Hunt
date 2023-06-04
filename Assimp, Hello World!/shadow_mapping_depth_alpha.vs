#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

uniform sampler2D textureAlpha;

void main()
{   
    float alpha = texture(textureAlpha, aTexCoords).a;
    if(alpha == 1.0){
        gl_Position = lightSpaceMatrix * model * vec4(0.0, 0.0, 0.0, 0.0);
    }else{
        gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
    }
    
}
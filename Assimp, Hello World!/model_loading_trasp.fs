#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec3 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform samplerCube texture_diffuse1;

void main()
{   
    vec3 color = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    FragColor = vec4(color, 0.3);
}
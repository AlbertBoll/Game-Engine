#version 430 core

uniform vec3 u_Albedo;

out vec4 FragColor;

void main()
{
    FragColor = vec4(u_Albedo, 1.0);
}
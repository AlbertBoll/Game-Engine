#version 430 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in vec4 a_Tangent;

uniform mat4 u_ViewProj;
uniform mat4 u_Model;


void main()
{
    gl_Position = u_ViewProj * u_Model * vec4(a_Position, 1.0);
}
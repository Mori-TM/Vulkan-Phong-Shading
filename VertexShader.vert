#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) in vec3 Pos;
layout(location = 1) in vec4 Color;
layout(location = 2) in vec2 inUVcoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec2 FragUVcoord;
layout(location = 2) out vec3 FragNormal;
layout(location = 3) out vec3 FragViewVec;
layout(location = 4) out vec3 FragLightVec;

layout(binding = 0) uniform UBO
{
	mat4 Model;
	mat4 View;
	mat4 Projection;
	vec3 LightPos;
} ubo;

void main()
{
	gl_Position = ubo.Projection * ubo.View * ubo.Model * vec4(Pos, 1.0);
	vec4 WorldPos = ubo.Model * vec4(Pos, 1.0);
	
	FragColor = Color;
	FragUVcoord = inUVcoord;
	FragNormal = mat3(ubo.Model) * inNormal;
	FragViewVec = (ubo.View * WorldPos).xyz;
	FragLightVec = ubo.LightPos - vec3(WorldPos);
}

#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 FragColor;
layout(location = 1) in vec2 FragUVcoord;
layout(location = 2) in vec3 FragNormal;
layout(location = 3) in vec3 FragViewVec;
layout(location = 4) in vec3 FragLightVec;

layout(location = 0) out vec4 OutColor;

layout(binding = 1) uniform sampler2D tex;

void main()
{
//	OutColor = texture(tex, FragUVcoord);
	
	vec3 N = normalize(FragNormal);
	vec3 V = normalize(FragViewVec);
	vec3 L = normalize(FragLightVec);
	vec3 R = reflect(L, N);
	
	vec3 Ambient = FragColor * 0.1;
	vec3 Diffuse = max(dot(N, L), 0.0) * FragColor;
	vec3 Specular = pow(max(dot(V, R), 0.0), 16.0) * vec3(2.0);
	
	vec4 result = vec4(Ambient + Diffuse + Specular, 1.0);
	
//	OutColor = result;
	OutColor = texture(tex, FragUVcoord) * result;
}

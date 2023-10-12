#version 450
#extension GL_ARB_separate_shader_objects : enable

 //// For image sampler
out gl_PerVertex {
	vec4 gl_Position;
};

layout(binding = 0) uniform UniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inCoord;
layout(location = 0) out vec3 outColor;
layout(location = 1) out vec2 outCoord;

// Colors for quads for when they don't have an image to sample.
vec3 xColors[4] = vec3[](
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0),
	vec3(1.0, 1.0, 1.0)
);

void main() {
	outColor    = xColors[gl_VertexIndex % 4];
	outCoord    = inCoord;
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);	
}
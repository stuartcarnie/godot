#[vertex]

#version 450

#VERSION_DEFINES

layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;

layout(push_constant, std140) uniform UBO {
	mat4 MVP;
}
global;

layout(location = 0) out vec2 vTexCoord;

void main() {
	vTexCoord = TexCoord;
	gl_Position = global.MVP * Position;
}

#[fragment]

#version 450

#VERSION_DEFINES

layout(push_constant, std140) uniform UBO {
	mat4 MVP;
}
global;

layout(set = 0, binding = 0) uniform sampler2D tex;

layout(location = 0) in vec2 vTexCoord;

layout(location = 0) out vec4 color;


void main() {
	color = texture(tex, vTexCoord);
}

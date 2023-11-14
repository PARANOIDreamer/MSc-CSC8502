#version 330 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 textureMatrix;

in vec3 position;
in vec2 texCoord;

out Vertex {
	vec2 texCoord;
} OUT;

void main() {
	//OUT.texCoord = ( textureMatrix * vec4(texCoord , 0.0 , 1.0)). xy;
	OUT.texCoord = texCoord;
	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
	vec4 worldPos = (modelMatrix * vec4(position, 1));
	gl_Position = (projMatrix * viewMatrix) * worldPos;
}
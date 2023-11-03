#version 330 core

uniform sampler2D diffuseTex;

in Vertex {
vec2 texCoord;
}IN;

out vec4 fragColour;
void main (void) {
fragColour = texture(diffuseTex, IN.texCoord);
//fragColour = texture(diffuseTex, IN.texCoord).rgba;
//fragColour = texture(diffuseTex, IN.texCoord).xyzw;
//fragColour = texture(diffuseTex, IN.texCoord).rgzw;
//fragColour = texture(diffuseTex, IN.texCoord).bgra;
//fragColour = texture(diffuseTex, IN.texCoord).xxxw;
}
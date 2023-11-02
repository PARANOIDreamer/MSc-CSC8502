# version 330 core

in Vertex {
vec4 colour ;
} IN ;

out vec4 fragColour ;
void main (void) {
//if(IN.colour.x<= 0.5 && IN.colour.y <=0.5 && IN.colour.z<= 0.5)discard;
//if(IN.colour.x<= 0.5 && IN.colour.z <=0.5)discard;
//if(IN.colour.x + IN.colour.y <= 0.9 && IN.colour.y + IN.colour.z <=0.9 && IN.colour.z + IN.colour.x <= 0.9)discard;
fragColour=IN.colour;
}
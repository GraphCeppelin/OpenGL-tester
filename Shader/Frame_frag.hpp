#version 330 core

in vec2 UV;

out vec4 color;

uniform sampler2D fbo_texture;

void main(){
    vec2 coord = vec2(UV.x, UV.y);
    vec4 colr = texture2D( fbo_texture, abs(coord) ).rgba ;
    color = colr ;
}
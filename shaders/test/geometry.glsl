#version 430 core

layout(triangles) in; 
layout(triangle_strip, max_vertices = 3) out; 

in vec3 color_out[];
out vec3 color_out_frag; 

void main() {
    for(int i = 0; i < 3; i++) {
        color_out_frag = color_out[i];
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
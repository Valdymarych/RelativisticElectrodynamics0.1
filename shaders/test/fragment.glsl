#version 430
in vec3 color_out_frag; 
out vec4 outColor;
void main() {
    vec3 c = color_out_frag;
    //c = (c/max(max(c.x,c.y),c.z) + c/sqrt(dot(c,c)))/2;
    c = vec3(1.)-c;
    c = vec3(1.)-vec3( c.x*c.x, c.y*c.y, c.z *c.z );
    outColor = vec4(c,1.0);
}

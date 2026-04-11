#version 430
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
out vec3 color_out;

layout(std140, binding = 0) uniform Uniforms {
    int buffer_offset;
    int history_element_size;
    int amount_of_spheres;
    int history_size;

    float c;
    float k;
    float arrow_size;
    float arrow_transparency_factor;

    float amplifying_factor;
    float display_ratio;
    float angle_of_rotation;
    float angle_of_rotation_2;

    int display_mode;
    int pad1;
    int pad2;
    int pad3;
} uf;



vec3 apply_rotation(vec3 pos){
    vec3 uv_proto=vec3(pos.x*cos(uf.angle_of_rotation)+pos.z*sin(uf.angle_of_rotation),pos.y,pos.z*cos(uf.angle_of_rotation)-pos.x*sin(uf.angle_of_rotation));
    return vec3(uv_proto.x,uv_proto.y*cos(uf.angle_of_rotation_2)+uv_proto.z*sin(uf.angle_of_rotation_2),uv_proto.z*cos(uf.angle_of_rotation_2)-uv_proto.y*sin(uf.angle_of_rotation_2));
}
 
vec3 to_uv(vec3 pos){
    vec3 uv_proto = apply_rotation(pos);
    return vec3(uv_proto.x/(2.+uv_proto.z)*uf.display_ratio*uf.amplifying_factor,uv_proto.y/(2.+uv_proto.z)*uf.amplifying_factor, (uv_proto.z+2.)/10.);
}

void main() {
    color_out = color*abs(sin(uf.buffer_offset/50.));
    gl_Position = vec4(to_uv(position), 1.0);
}

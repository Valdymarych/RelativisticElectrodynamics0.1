#version 430
layout(location = 0) in vec3 position_mesh;
layout(location = 1) in vec3 position;
layout(location = 2) in vec4 color_in;

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

    uint display_mode;
    uint grid_size_x;
    uint grid_size_y;
    uint grid_size_z;

    float factor_E;
    float factor_B;
    float factor_P;
    float padding;

    float time;
    float time_per_frame;
} uf;

vec3 apply_rotation(vec3 pos){
    vec3 uv_proto=vec3(pos.x*cos(uf.angle_of_rotation)+pos.z*sin(uf.angle_of_rotation),pos.y,pos.z*cos(uf.angle_of_rotation)-pos.x*sin(uf.angle_of_rotation));
    return vec3(uv_proto.x,uv_proto.y*cos(uf.angle_of_rotation_2)+uv_proto.z*sin(uf.angle_of_rotation_2),uv_proto.z*cos(uf.angle_of_rotation_2)-uv_proto.y*sin(uf.angle_of_rotation_2));
}

vec3 to_uv(vec3 pos){
    vec3 uv_proto = apply_rotation(pos);
    return vec3(uv_proto.x/(2.+uv_proto.z)*uf.display_ratio*uf.amplifying_factor,uv_proto.y/(2.+uv_proto.z)*uf.amplifying_factor, (uv_proto.z+2.)/5.);
}

out vec3 color_out;
void main() {
    color_out = color_in.xyz*clamp(dot(position_mesh,vec3(1.,1.,-1.)*0.8),0.2,1);
    vec3 uv=to_uv(position_mesh*color_in.w+position.xyz);
    gl_Position = vec4(uv, 1.);
}
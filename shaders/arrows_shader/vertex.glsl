#version 430
layout(location = 0) in vec3 position;

struct FieldData {
    vec4 E;
    vec4 B;
    vec4 P;
};

layout(std430, binding = 1) buffer FieldDataBuffer {
    FieldData arrows[];
};

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
    float factor_common;

    float time;
    float time_per_frame;
} uf;

vec3 apply_rotation(vec3 pos){
    vec3 uv_proto=vec3(pos.x*cos(uf.angle_of_rotation)+pos.z*sin(uf.angle_of_rotation),pos.y,pos.z*cos(uf.angle_of_rotation)-pos.x*sin(uf.angle_of_rotation));
    return vec3(uv_proto.x,uv_proto.y*cos(uf.angle_of_rotation_2)+uv_proto.z*sin(uf.angle_of_rotation_2),uv_proto.z*cos(uf.angle_of_rotation_2)-uv_proto.y*sin(uf.angle_of_rotation_2));
}
 
vec3 to_uv(vec3 pos){
    vec3 uv_proto = apply_rotation(pos);
    return vec3(uv_proto.x/(2.+uv_proto.z)*uf.display_ratio*uf.amplifying_factor,uv_proto.y/(2.+uv_proto.z)*uf.amplifying_factor, (uv_proto.z+2.)/10.);
}

out vec4 color_out;
void main() {
    uint instanceID = gl_InstanceID;
    uint index = instanceID/3;
    uint arrow_type = instanceID%3;

    if ((uf.display_mode&(1u<<arrow_type))==0u){
        gl_Position = vec4(-2,-2,-2,-2);
        return;
    }

    uint x = index % uf.grid_size_x;
    uint y = (index / uf.grid_size_x) % uf.grid_size_y;
    uint z = index / (uf.grid_size_x * uf.grid_size_y);
    vec3 pos_grid = vec3(x,y,z)*2./uf.grid_size_x-vec3(1.,1.,1.);

    vec4 arrow;
    vec3 basic_color;
    float factor;
    if (arrow_type==0){arrow=arrows[index].E;basic_color=vec3(1.,0.,0.);factor=uf.factor_E;}
    if (arrow_type==1){arrow=arrows[index].B;basic_color=vec3(0.,0.,1.);factor=uf.factor_B;}
    if (arrow_type==2){arrow=arrows[index].P;basic_color=vec3(1.,1.,1.);factor=uf.factor_P;}



    vec3 tempUp = abs(arrow.y) < 0.9 ? vec3(0, 1, 0) : vec3(1, 0, 0);

    vec3 right = normalize(cross(tempUp, arrow.xyz));
    vec3 up = cross(arrow.xyz, right);
    mat3 transform = mat3(right, up, arrow.xyz);

    vec3 final_pos = pos_grid+transform*position*uf.arrow_size;

    vec3 uv = to_uv(final_pos);

    color_out = clamp(arrow.w*factor*uf.factor_common,0.,1.)*vec4(basic_color,uf.arrow_transparency_factor);
    gl_Position = vec4(uv, 1.0);

}
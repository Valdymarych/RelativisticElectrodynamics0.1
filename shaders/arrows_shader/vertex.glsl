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

    mat4 view;
    mat4 projection;

    vec4 cameraPos;

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

layout(std430, binding = 4) buffer debuggerBuffer {
    float debugger[];
};

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
    vec3 pos_grid = 2.*vec3(
            (x+1.)/(uf.grid_size_x+1.),
            (y+1.)/(uf.grid_size_y+1.),
            (z+1.)/(uf.grid_size_z+1.)
        )-vec3(1.,1.,1.);
    vec4 arrow;
    vec3 basic_color;
    float factor;
    if (arrow_type==0){arrow=arrows[index].E;basic_color=vec3(1.,0.,0.);factor=uf.factor_E;}
    if (arrow_type==1){arrow=arrows[index].B;basic_color=vec3(0.,0.,1.);factor=uf.factor_B;}
    if (arrow_type==2){arrow=arrows[index].P;basic_color=vec3(1.,1.,1.);factor=uf.factor_P;}

    vec3 toCamera = normalize(uf.cameraPos.xyz - pos_grid);
    vec3 along = normalize(arrow.xyz);
    vec3 tempUp = cross(toCamera,along);
    vec3 end_pos = pos_grid + (along*position.z + tempUp*position.y)*uf.arrow_size;

    vec4 uv = uf.projection*uf.view*vec4(end_pos,1.);
    color_out = clamp(arrow.w*factor*uf.factor_common,0.,1.)*vec4(basic_color,uf.arrow_transparency_factor);
    gl_Position = uv;
}
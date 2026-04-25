#version 430
layout(location = 0) in vec3 position_mesh;

struct StaticSphericalData {
    float m;
    float q;
    float r;
    int moving_mode;

    vec4 color;
    vec4 param1;
    vec4 param2;
    vec4 param3;
};

struct SphereState {
    vec4 pos;
    vec4 vel;
    vec4 acc;
};

layout(std430, binding = 2) buffer staticSphericalDataBuffer {
    StaticSphericalData staticSphericalData[];
};

layout(std430, binding = 3) buffer presentSphericalDataBuffer {
    SphereState presentSphericalData[];
};

layout(std430, binding = 4) buffer debuggerBuffer {
    float debugger[];
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
    float padding;

    float time;
    float time_per_frame;
} uf;


vec4 to_uv(vec3 pos){
    return uf.projection * uf.view * vec4(pos,1.);
}

out vec3 color_out;
void main() {
    SphereState presentData = presentSphericalData[gl_InstanceID];
    StaticSphericalData staticData = staticSphericalData[gl_InstanceID];
    color_out = staticData.color.xyz*clamp(dot(position_mesh,vec3(1.,1.,-1.)*0.8),0.2,1.);
    
    vec3 raw_pos = position_mesh*staticData.r+presentData.pos.xyz;
    
    vec4 uv=to_uv(raw_pos);
    gl_Position = uv;
}
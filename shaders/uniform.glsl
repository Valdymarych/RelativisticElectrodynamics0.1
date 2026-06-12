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
    float magnetic_permeability_inv;
    int history_size_log;

    float k_c_3;  // k/c^3
    float k_c_4;
    float k_c_5;
    float _c_2;  // 1/c^2
} uf;
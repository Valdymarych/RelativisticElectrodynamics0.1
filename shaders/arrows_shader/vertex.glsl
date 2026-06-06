layout(location = 0) in vec3 position;

layout(std430, binding = 1) buffer FieldDataBuffer {
    FieldData arrows[];
};

layout(std430, binding = 4) buffer debuggerBuffer {
    float debugger[];
};

layout(std430, binding = 5) buffer PivotPointsBuffer {
    PivotPoint pivot_points[];
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

    vec3 pos_grid = pivot_points[index].pos.xyz;


    vec4 arrow;
    vec3 basic_color;
    float factor;
    FieldData field_data = arrows[index];
    if (arrow_type==0){arrow=field_data.E;basic_color=vec3(1.,0.,0.);factor=uf.factor_E;}
    if (arrow_type==1){arrow=field_data.B;basic_color=vec3(0.,0.,1.);factor=uf.factor_B;}
    if (arrow_type==2){arrow=field_data.P;basic_color=vec3(1.,1.,1.);factor=uf.factor_P;}

    vec3 toCamera = normalize(uf.cameraPos.xyz - pos_grid);
    vec3 along = normalize(arrow.xyz);
    vec3 tempUp = cross(toCamera,along);
    vec3 end_pos = pos_grid + (along*position.z + tempUp*position.y)*uf.arrow_size;

    vec4 uv = uf.projection*uf.view*vec4(end_pos,1.);
    color_out = clamp(arrow.w*factor*uf.factor_common,0.,1.)*vec4(basic_color,uf.arrow_transparency_factor);
    gl_Position = uv;
}
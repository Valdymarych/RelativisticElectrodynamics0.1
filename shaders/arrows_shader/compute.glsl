layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 5) buffer PivotPointsBuffer {
    PivotPoint pivot_points[];
};

layout(std430, binding = 1) buffer FieldDataBuffer {
    FieldData arrows[];
};

void main() {
    uvec3 pos = gl_GlobalInvocationID;
    uint index = pos.x;
    //if (index >= uf.grid_size_x * uf.grid_size_y * uf.grid_size_z) return;
    vec3 position = pivot_points[index].pos.xyz;

    Field f = get_field(position,-1);
    FieldData F;
    float l;
    
    l=sqrt(dot(f.e,f.e));
    if (l==0. || ((uf.display_mode & 1u) == 0u)){
        F.E=vec4(0.);
    } else {
        F.E=vec4(f.e/l,l);
    }
    
    l=sqrt(dot(f.b,f.b));
    if (l==0. || ((uf.display_mode & 2u) == 0u)){
        F.B=vec4(0.);
    } else {
        F.B=vec4(f.b/l,l);
    }

    l=sqrt(dot(f.p,f.p));
    if (l==0. || ((uf.display_mode & 4u) == 0u)){
        F.P=vec4(0.);
    } else {
        F.P=vec4(f.p/l,l);
    }

    arrows[index]=F;
}
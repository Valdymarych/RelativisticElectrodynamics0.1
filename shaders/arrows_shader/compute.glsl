#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

struct SphereState {
    vec4 pos;
    vec4 vel;
    vec4 acc;
};

struct Field {
    vec3 e;
    vec3 b;
    vec3 p;
};

struct Arrow {
    vec3 n;
    float color;
};

struct FieldData {
    vec4 E;
    vec4 B;
    vec4 P;
};


layout(std430, binding = 1) buffer FieldDataBuffer {
    FieldData arrows[];
};

layout(std430, binding = 0) buffer SphereHistory {
    SphereState history[];
} spheres;

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

Field get_field(vec3 pos){
    vec3 E = vec3(0.);
    vec3 B = vec3(0.);
    vec3 dE = vec3(0.);
    int l;
    int r;
    int mid;
    int mid_index;
    vec4 sp_pos_4;
    vec3 sp_pos;
    float sp_time;
    vec3 betta;
    vec3 betta_dot;
    float gamma_squared;
    vec3 radius_vector;
    vec3 direction;
    float helpful_value1;
    float helpful_value2;

    SphereState last_sphere;
    SphereState next_sphere;
    SphereState average_sphere;
    float koef;
    float koef2;

    for (int sp=0; sp<uf.amount_of_spheres; sp++){
        l=0;
        r=uf.history_size;

        while (r>l+1){
            mid = (r+l)/2;
            mid_index = ((mid+uf.buffer_offset)%uf.history_size*uf.amount_of_spheres)+sp;
            sp_pos_4 = spheres.history[mid_index].pos;
            sp_pos=sp_pos_4.xyz-pos;
            sp_time=uf.time-sp_pos_4.w;
            if (
                uf.c*uf.c*sp_time*sp_time >= dot(sp_pos,sp_pos)
            ) {
                l=mid;
            } else {
                r=mid;
            }
        }

        last_sphere = spheres.history[((l+uf.buffer_offset)%uf.history_size*uf.amount_of_spheres)+sp];
        next_sphere = spheres.history[((r+uf.buffer_offset)%uf.history_size*uf.amount_of_spheres)+sp];
        koef = 1.;//-(uf.c*(uf.time-next_sphere.pos.w)-length(next_sphere.pos.xyz-pos))/(uf.c*(next_sphere.pos.w-last_sphere.pos.w)-length(last_sphere.pos.xyz-pos)+length(next_sphere.pos.xyz-pos));
        koef2 = 0.;//1-koef;

        average_sphere=SphereState(
            last_sphere.pos*koef+next_sphere.pos*koef2,
            last_sphere.vel*koef+next_sphere.vel*koef2,
            last_sphere.acc*koef+next_sphere.acc*koef2
        );

        betta = average_sphere.vel.xyz/uf.c;
        betta_dot = average_sphere.acc.xyz/uf.c;
        gamma_squared = 1/(1-dot(betta,betta));
        radius_vector = pos - average_sphere.pos.xyz;

        if (dot(radius_vector, radius_vector)==0.){
            direction = vec3(0.,0.,0.);
        } else {
            direction = normalize(radius_vector);
        }

        helpful_value1 = 1-dot(direction,betta);
        helpful_value2 = 1/(helpful_value1*helpful_value1*helpful_value1);
        dE = last_sphere.vel.w * (
            (direction - betta)*helpful_value2/(gamma_squared*dot(radius_vector,radius_vector))
            +
            ((direction - betta)*dot(direction,betta_dot)-betta_dot*helpful_value1)*helpful_value2/(sqrt(dot(radius_vector,radius_vector))*uf.c)
        );
        E = E + dE;
        B = B + cross(dE,direction)/uf.c;
    }
    E = E*uf.k;
    B = B*uf.k;
    vec3 P = cross(B,E)*(uf.c*uf.c)/(12.56*uf.k);
    return Field(E, B, P);
}

void main() {
    uvec3 pos = gl_GlobalInvocationID;
    if (pos.x >= uf.grid_size_x || pos.y >= uf.grid_size_y || pos.z >= uf.grid_size_z) return;
    vec3 position = pos*2./uf.grid_size_x-vec3(1.,1.,1.);
    uint index = pos.z * (uf.grid_size_x * uf.grid_size_y) + (pos.y * uf.grid_size_x) + pos.x;

    Field f = get_field(position);
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
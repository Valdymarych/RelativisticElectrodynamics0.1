#version 430 core
layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;

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



layout(std430, binding = 0) buffer SphereHistory {
    SphereState history[];
} spheres;

layout(std430, binding = 2) buffer staticSphericalDataBuffer {
    StaticSphericalData staticSphericalData[];
};

layout(std430, binding = 3) buffer presentSphericalDataBuffer {
    SphereState presentSphericalData[];
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

Field get_field_without_itself(vec3 pos, uint chargeId){
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
        if (sp!=chargeId){
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
            koef = -(uf.c*(uf.time-next_sphere.pos.w)-length(next_sphere.pos.xyz-pos))/(uf.c*(next_sphere.pos.w-last_sphere.pos.w)-length(last_sphere.pos.xyz-pos)+length(next_sphere.pos.xyz-pos));
            koef2 = 1-koef;

            average_sphere=SphereState(
                last_sphere.pos*koef+last_sphere.pos*koef2,
                last_sphere.vel*koef+last_sphere.vel*koef2,
                last_sphere.acc*koef+last_sphere.acc*koef2
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
    }
    E = E*uf.k;
    B = B*uf.k;
    vec3 P = cross(B,E)*(uf.c*uf.c)/(12.56*uf.k);
    return Field(E, B, P);
}


void main() {
    uint chargeId = gl_GlobalInvocationID.x;
    if (chargeId >= uf.amount_of_spheres) return;


    StaticSphericalData staticData = staticSphericalData[chargeId];
    SphereState sphere_now = presentSphericalData[chargeId];

    SphereState sphere_next;

    if (staticData.moving_mode == 0){
        Field f = get_field_without_itself(sphere_now.pos.xyz,chargeId);
        
        vec3 v = sphere_now.vel.xyz;
        float gamma = inversesqrt(1.0 - dot(v,v) / (uf.c*uf.c));
        float m = sphere_now.acc.w;
        float q = sphere_now.vel.w;
        vec3 p = gamma * m * v;

        vec3 total_field  = f.e + cross(v,f.b);
        vec3 force = q * total_field;
                   -  2.*pow(q,4)*gamma/(3*pow(uf.c,5)*pow(m,3))  * v * (dot(total_field,total_field)-pow(dot(v,f.e)/uf.c,2)); 
        p = p + force; // dt = 1;
        float p2 = dot(p, p);
        vec3 new_v = p*uf.c*inversesqrt(m*m*uf.c*uf.c+p2);
        
        sphere_next = SphereState(
            vec4(sphere_now.pos.xyz+new_v, sphere_now.pos.w+1),  //dt=1;
            vec4(new_v, q),
            vec4(new_v-v,m)                  //dt=1;
        );
    } else {
        if (staticData.moving_mode == 1){
            float phase = staticData.param3.x*uf.time+staticData.param3.y;
            sphere_next
            = SphereState(
                vec4(staticData.param1.xyz + staticData.param2.xyz * sin(phase), uf.time),
                vec4(staticData.param2.xyz*staticData.param3.x*cos(phase),sphere_now.vel.w),
                vec4(-staticData.param2.xyz*staticData.param3.x*staticData.param3.x*sin(phase),sphere_now.acc.w)
            );

        }
        if (staticData.moving_mode == 2){
            vec3 param2 = staticData.param2.xyz;
            vec3 normal1 = vec3(1.,0.,0.);
            if (1.-param2.x<0.1){
                normal1 = vec3(0.,1.,0.);
            }
            vec3 basis1 = staticData.param3.z*cross(normal1,param2);
            vec3 basis2 = cross(basis1, param2);
            float w = staticData.param3.x;
            float phase = w*uf.time+staticData.param3.y;
            sphere_next
            = SphereState(
                vec4(staticData.param1.xyz + basis1 * sin(phase) + basis2 * cos(phase), uf.time),
                vec4(w * basis1 * cos(phase) - w * basis2 * sin(phase),sphere_now.vel.w),
                vec4(- w * w* basis1 * sin(phase) - w * w * basis2 * cos(phase),sphere_now.acc.w)
            );
        }
        if (staticData.moving_mode == 3){
            sphere_next
            = SphereState(
                vec4(staticData.param1.xyz + staticData.param2.xyz * uf.time, uf.time),
                vec4(staticData.param2.xyz ,sphere_now.vel.w),
                vec4(0.,0.,0.,sphere_now.acc.w)
            );
        }
    }
    presentSphericalData[chargeId] = sphere_next;
    spheres.history[uf.buffer_offset%uf.history_size*uf.amount_of_spheres+chargeId] = sphere_next;
}
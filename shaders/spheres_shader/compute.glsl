layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;

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

layout(std430, binding = 2) buffer staticSphericalDataBuffer {
    StaticSphericalData staticSphericalData[];
};

layout(std430, binding = 3) buffer presentSphericalDataBuffer {
    SphereState presentSphericalData[];
};


void main() {
    int chargeId =int(gl_GlobalInvocationID.x);
    if (chargeId >= uf.amount_of_spheres) return;


    StaticSphericalData staticData = staticSphericalData[chargeId];
    SphereState sphere_now = presentSphericalData[chargeId];

    SphereState sphere_next;

// needed for free charges;

    Field f;
    vec3 v;
    float gamma;
    float gamma2;
    float v_dot_E;

    vec3 f_0;
    vec3 f_2;
    vec3 f_3;
    vec3 force;

    vec3 p;
    float p2;
    vec3 new_v;
    
// needed for elsewhere
    float phase;
    float angular_frequency;
    vec3 basis1;
    vec3 basis2;

    if (staticData.moving_mode == 0){
        f = get_field(sphere_now.pos.xyz,chargeId);
        
        v = sphere_now.vel.xyz;
        gamma = inversesqrt(1.0 - dot(v,v) / (uf.c*uf.c));
        gamma2 = gamma*gamma;
        v_dot_E = dot(v,f.e);

        f_0 = f.e+cross(v,f.b);  // lorenz without e
        //f_1 =  !TO HARD!
        f_2 = uf.k_c_4*staticData.param1.y*(cross(f.e,f.b)+f.b*dot(f.b,v)-v*dot(f.b,f.b)+uf._c_2*f.e*v_dot_E);
        f_3 = -uf.k_c_5*staticData.param1.y*gamma2*v*(dot(f_0,f_0)-uf._c_2*v_dot_E*v_dot_E);
        force = f_0*staticData.q+f_2+f_3;


        p = gamma * staticData.m * v;
        p = p + force; // dt = 1;
        p2 = dot(p, p);
        new_v = p*uf.c*inversesqrt(staticData.m*staticData.m*uf.c*uf.c+p2);
        
        sphere_next = SphereState(
            vec4(sphere_now.pos.xyz+new_v, sphere_now.pos.w+1),  //dt=1;
            vec4(new_v, staticData.q),
            vec4(new_v-v,staticData.m)                  //dt=1;
        );
    }
    if (staticData.moving_mode == 1){    // simple occilation
        angular_frequency = staticData.param3.x;
        phase = staticData.param3.x*uf.time+staticData.param3.y;
        sphere_next
        = SphereState(
            vec4(staticData.param1.xyz + staticData.param2.xyz * sin(phase), uf.time),
            vec4(staticData.param2.xyz*angular_frequency*cos(phase),sphere_now.vel.w),
            vec4(-staticData.param2.xyz*angular_frequency*angular_frequency*sin(phase),sphere_now.acc.w)
        );

    }
    if (staticData.moving_mode == 2){    // circular occilation
        angular_frequency=staticData.param2.w;
        phase = angular_frequency*uf.time+staticData.param3.w;
        basis1 = staticData.param2.xyz;
        basis2 = staticData.param3.xyz;
        sphere_next
        = SphereState(
            vec4(staticData.param1.xyz + basis1 * sin(phase) + basis2 * cos(phase), uf.time),
            vec4(angular_frequency * basis1 * cos(phase) - angular_frequency * basis2 * sin(phase),sphere_now.vel.w),
            vec4(- angular_frequency * angular_frequency* basis1 * sin(phase) - angular_frequency * angular_frequency * basis2 * cos(phase),sphere_now.acc.w)
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

    presentSphericalData[chargeId] = sphere_next;
    spheres.history[uf.buffer_offset%uf.history_size*uf.amount_of_spheres+chargeId] = sphere_next;
}
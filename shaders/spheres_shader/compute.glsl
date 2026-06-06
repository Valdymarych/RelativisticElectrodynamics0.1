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

    if (staticData.moving_mode == 0){
        Field f = get_field(sphere_now.pos.xyz,chargeId);
        
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
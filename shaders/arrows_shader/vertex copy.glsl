#version 430
layout(location = 0) in vec3 position;
out vec3 arrow_1;
out vec4 color_1;

out vec3 arrow_2;
out vec4 color_2;

out vec3 arrow_3;
out vec4 color_3;
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

    int display_mode;
    int pad1;
    int pad2;
    int pad3;
} uf;

vec3 apply_rotation(vec3 pos){
    vec3 uv_proto=vec3(pos.x*cos(uf.angle_of_rotation)+pos.z*sin(uf.angle_of_rotation),pos.y,pos.z*cos(uf.angle_of_rotation)-pos.x*sin(uf.angle_of_rotation));
    return vec3(uv_proto.x,uv_proto.y*cos(uf.angle_of_rotation_2)+uv_proto.z*sin(uf.angle_of_rotation_2),uv_proto.z*cos(uf.angle_of_rotation_2)-uv_proto.y*sin(uf.angle_of_rotation_2));
}
 
vec3 to_uv(vec3 pos){
    vec3 uv_proto = apply_rotation(pos);
    return vec3(uv_proto.x/(2.+uv_proto.z)*uf.display_ratio*uf.amplifying_factor,uv_proto.y/(2.+uv_proto.z)*uf.amplifying_factor, (uv_proto.z+2.)/10.);
}

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
    for (int sp=0; sp<uf.amount_of_spheres; sp++){
        l=0;
        r=uf.history_size;

        while (r>l+1){
            mid = (r+l)/2;
            mid_index = ((mid+uf.buffer_offset)%uf.history_size*uf.amount_of_spheres)+sp;
            sp_pos_4 = spheres.history[mid_index].pos;
            sp_pos=sp_pos_4.xyz-pos;
            sp_time=sp_pos_4.w;
            if (
                uf.c*uf.c*(uf.buffer_offset-sp_time)*(uf.buffer_offset-sp_time) >= dot(sp_pos,sp_pos)
            ) {
                l=mid;
            } else {
                r=mid;
            }
        }

        last_sphere = spheres.history[((l+uf.buffer_offset)%uf.history_size*uf.amount_of_spheres)+sp];

        betta = last_sphere.vel.xyz/uf.c;
        betta_dot = last_sphere.acc.xyz/uf.c;
        gamma_squared = 1/(1-dot(betta,betta));
        radius_vector = pos - last_sphere.pos.xyz;

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
    return Field(E, B, cross(B,E));
}


Arrow proceed(vec3 raw_arrow, vec3 uv) {
    float len = length(raw_arrow);
    vec4 arrow = vec4(normalize(raw_arrow)*uf.arrow_size,clamp(len*len/2000.,0.005, 0.1));//spheres.history[(uf.buffer_offset%uf.history_size)*uf.amount_of_spheres].pos.xyz - position;
    float color_rare = clamp(arrow.w*10.,0.,1.);
    color_rare=color_rare*color_rare*color_rare*color_rare*color_rare;

    return Arrow(vec3(to_uv(position+arrow.xyz).xy-uv.xy,uv.z),color_rare);
}

void main() {

    vec3 uv=to_uv(position);
    if (uv.z<0.1){
        arrow_1 = vec3(0.0,0.,2.);
        color_1 = vec4(0.);
        arrow_2 = vec3(0.0,0.,2.);
        color_2 = vec4(0.);
        arrow_3 = vec3(0.0,0.,2.);
        color_3 = vec4(0.);
        gl_Position = vec4(0.,0.,2.,1.0);
    } else {
            
        Field f = get_field(position);
        
        Arrow ar1=proceed(f.p,uv);
        Arrow ar2=proceed(f.b,uv);
        Arrow ar3=proceed(f.e,uv);


        color_1 = ar1.color*vec4(1.,1.,1.,uf.arrow_transparency_factor);
        arrow_1 = ar1.n;
        
        color_2 = ar2.color*vec4(0.,0.,1.,uf.arrow_transparency_factor);
        arrow_2 = ar2.n;
        
        color_3 = 1000.*ar3.color*vec4(1.,0.,0.,uf.arrow_transparency_factor);
        arrow_3 = ar3.n;
        gl_Position = vec4(uv, 1.0);
    }

}
// потрібно буде добавити history_size_log
// добавити константу для потоку енергії (uf.c*uf.c)/(12.56*uf.k)


Field get_field(vec3 observer_pos, int observer_index){

// блок величин для бінарного пошуку
    int l;
    int s;
    int m;
    int m_index;
    vec4 sp_pos_4;            
    vec3 sp_pos;
    float sp_time;
    float sp_dist;


//   блок величин для інтерполяції
    SphereState l_s;
    SphereState n_s;
    SphereState state;
    vec4 pos_l;
    vec4 pos_d;
    float a;
    float half_b;
    float c;
    float q;
    float k;


// блок величин для розрахунку поля     
    vec3 R;
    float R_mag;
    vec3 n;
    vec3 u;
    vec3 numerator;
    float denominator;
    vec3 dE;

// для відсієння поля від самого себе:
    float shouldCount = abs(sign(sp-observer_index))

    for (int sp=0; sp<uf.amount_of_spheres; sp++){
        
//  бінарний пошук
        l=0;
        s=uf.history_size;
        for (int j; j<uf.history_size_log; j++){
            s=s>>1;
            m=l+s;
            m_index = ((m+uf.buffer_offset)%uf.history_size*uf.amount_of_spheres)+sp;
            sp_pos_4 = spheres.history[m_index].pos;
            
            sp_pos=sp_pos_4.xyz-observer_pos;
            sp_time=(uf.time-sp_pos_4.w)*c;

            sp_dist = sp_time*sp_time-sp_pos*sp_pos;

            l = l + (m-l) * int(step(0.0, sp_dist))          
        }

        
//  інтерполяція
        l_s = spheres.history[((l+uf.buffer_offset)%uf.history_size*uf.amount_of_spheres)+sp];
        n_s = spheres.history[((l+1+uf.buffer_offset)%uf.history_size*uf.amount_of_spheres)+sp];
        
        pos_l = l_s.pos;
        pos_d = n_s-l.s;
        
        a = pos_d.w*pos_d.w-dot(pos_d.xyz,pos_d.xyz);
        half_b = pos_l.w*pos_d.w-dot(pos_l.xyz,pos_d.xyz);
        c = pos_l.w*pos_l.w-dot(pos_l.xyz,pos_l.xyz);
        

        q = -half_b - sign(b) * sqrt(max(0.,half_b * half_b - a * c)); // чисельник що враховує 0.25 від дискримінанта
        k = c / (q + 1e-20);    // спражій коофіцєент k

        state=SphereState(
            l_s.pos+pos_d*koef,
            l_s.vel+(n_s.vel-l_s.vel)*koef2,
            l_s.vel+(n_s.vel-l_s.vel)*koef2,
        );


// розрахунок полів
        R = observer_pos - state.pos.xyz;
        R_mag = length(R)+1.-shouldCount;
        n = R/R_mag;
        u = uf.c*n - state.vel.xyz;
        

        numerator = shouldCount * state.acc.w*R_mag*((uf.c*2-length(state.vel.xyz)+dot(R,state.acc.xyz))*u-dot(R,u)*state.acc.xyz);
        denominator = pow(dot(u,R),3);

        dE = numerator/denominator;

        E = E + dE;
        B = B + cross(n,dE)/uf.c;
    }
    E = E*uf.k;
    B = B*uf.k;
    vec3 P = cross(B,E)*uf.energy_k;
    return Field(E, B, P);
}
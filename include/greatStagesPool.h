#include "greatCommon.h"
#include <iostream>
void addHerzt(std::vector<MyCharge>& charges, Vec3 position, float lambda, Vec3 amplitude, float phase, float c, float charge, float radius){
    float v = amplitude.length()*2*PI*c/lambda;
    float w = v/std::sqrt(amplitude.x*amplitude.x+amplitude.y*amplitude.y+amplitude.z*amplitude.z);
    MyCharge charge1(position, {0.,0.,0.},radius,charge,-1.,{1.,0.,0.}, 1, {position.x, position.y, position.z, 0.}, {amplitude.x, amplitude.y, amplitude.z, 0.},{w,phase,0.,0.});
    MyCharge charge2(position,{0.,0.,0.},radius,-charge,-1.,{0.,0.,1.}, 1, {position.x, position.y, position.z, 0.}, {-amplitude.x, -amplitude.y, -amplitude.z, 0.},{w,phase,0.,0.});
    charges.push_back(charge1);
    charges.push_back(charge2);
}

void addAntenna(std::vector<MyCharge>& charges, int amount_of_herzt, Vec3 position, Vec3 direction, float lambda, float amplitude, float c, float power, float radius,bool with_binomialCoefficient){
    Vec3 dir = direction*(1/direction.length());
    Vec3 dir2 = {0,0,1};
    if (dir.z>0.99){
        dir2={0,1,0};
    }
    Vec3 amplitude_vec = (Vec3::crossProduct(dir,dir2))*amplitude;
    float koef=1;
    float charge=std::sqrt(power)*lambda*lambda/c/c/amplitude/amount_of_herzt;
    for (int i=0; i<amount_of_herzt; i++){
        if (with_binomialCoefficient){
            koef = 1.*binomialCoefficient(amount_of_herzt-1,i)/binomialCoefficient(amount_of_herzt-1,amount_of_herzt/2);
        }
        addHerzt(charges, position+dir*(lambda/4.*i),lambda,amplitude_vec,PI/2.*i,c,charge*koef,radius);
    }
}


void defineStage1(std::vector<MyCharge>& charges_vector, State& state){
    
    state.grid_size_x= 64;
    state.grid_size_y= 64;
    state.grid_size_z= 64;
    state.c = 0.01;
    state.k = 0.0002;

    state.log_factor_E=0.;
    state.log_factor_B=0.235;
    state.log_factor_P=0.;
    state.log_factor_common=0.;
    state.arrow_size=0.065;
    state.arrow_transparency_factor=0.5;
    state.transparency_rendering_style=1;
    state.show_E=false;
    state.show_B=false;
    state.show_P=true;


    float lambda = 2./5.;
    float A = 0.1/5.;

    state.const_log_factor_E=std::log10(1/(2.*state.k*36*A/lambda/lambda));
    state.const_log_factor_B=state.const_log_factor_E;
    state.const_log_factor_P=state.const_log_factor_B+state.const_log_factor_E+std::log10(12.*state.k/state.c/state.c)-1;
    state.const_log_factor_common=-3.;

    addAntenna(charges_vector,  3, {0.,0.,0.}, {2.,0.,0.},lambda,2.*A,state.c,.000000005,0.1/5.,true);
}

void defineStage2(std::vector<MyCharge>& charges_vector, State& state){
    
    state.grid_size_x = 70;
    state.grid_size_y = 70;
    state.grid_size_z = 70;
    state.c = 0.01;
    state.k = 0.0002;

    state.arrow_size=0.066;
    state.log_factor_E=0.;
    state.log_factor_B=0.235;
    state.log_factor_P=0.622;
    state.log_factor_common=0.;
    state.arrow_transparency_factor=0.5;
    state.transparency_rendering_style=1;
    state.show_E=false;
    state.show_B=false;
    state.show_P=true;


    float lambda = 2./10.;
    float A = 0.1/10.;

    state.const_log_factor_E=std::log10(1/(2.*state.k*36*A/lambda/lambda));
    state.const_log_factor_B=state.const_log_factor_E;
    state.const_log_factor_P=state.const_log_factor_B+state.const_log_factor_E+std::log10(12.*state.k/state.c/state.c);
    state.const_log_factor_common=-3.;

    addAntenna(charges_vector,  5, {.5,.5,0.5}, {2.,0.,0.},lambda,2.*A,state.c,.0000000005,0.1,true);
    addAntenna(charges_vector,  5, {.5,-.5,0.5}, {2.,0.,0.},lambda,2.*A,state.c,.0000000005,0.1,true);
}

void defineStage3(std::vector<MyCharge>& charges_vector, State& state){
    
    state.grid_size_x = 30;
    state.grid_size_y = 30;
    state.grid_size_z = 30;
    state.c = 0.01;
    state.k = 0.0002;

    state.log_factor_E=0.24;
    state.log_factor_B=0.524;
    state.log_factor_P=-0.2;
    state.log_factor_common=-0.387;
    state.arrow_size=0.067;
    state.transparency_rendering_style=1;
    state.arrow_transparency_factor=1.;
    state.show_E=true;
    state.show_B=false;
    state.show_P=false;

    state.const_log_factor_E=1.84f;
    state.const_log_factor_B=-0.16f;
    state.const_log_factor_P=4.06f;
    state.const_log_factor_common=0.;

    //addAntenna(charges_vector,  1, {0.,0.,0.}, {2.,0.,0.},lambda,2.*A,state.c,.0000000005,0.02,true);
    for(int i=0; i<2; i++){
        charges_vector.emplace_back(Vec3{(i-1)*0.05f,0.f,0.f},Vec3{0.f,0.f,0.f},0.02f,2.f,300.f,Vec3{1.f,0.f,0.f});
    }
}

void defineStage4(std::vector<MyCharge>& charges_vector, State& state){
    
    state.grid_size_x = 10;
    state.grid_size_y = 10;
    state.grid_size_z = 10;
    state.c = 0.01;
    state.k = 0.0002;

    float lambda = 2.;
    float A = 0.1;

    state.log_factor_E=0.;
    state.log_factor_B=0.;
    state.log_factor_P=0.;
    state.log_factor_common=-0.387;
    state.arrow_size=0.127;
    state.transparency_rendering_style=0;
    state.arrow_transparency_factor=1.;
    state.show_E=false;
    state.show_B=false;
    state.show_P=true;



    state.const_log_factor_E=3.;
    state.const_log_factor_B=1.;
    state.const_log_factor_P=5.;
    state.const_log_factor_common=0.;

    MyCharge charge(Vec3{0.f,0.f,0.f}, Vec3{0.f,0.f,0.f}, 0.1f,1.f,-1.f,Vec3{1.f,0.f,0.f},3,Vec4{-1.f,0.f,0.f,0.f},Vec4{0.0085f,0.f,0.f,0.f},Vec4{0.f,0.f,0.f,0.f});
    charges_vector.push_back(charge);
}

void defineStage5(std::vector<MyCharge>& charges_vector, State& state){
    state.grid_size_x = 30;
    state.grid_size_y = 30;
    state.grid_size_z = 1;
    state.c = 0.01;
    state.k = 0.0002;


    state.log_factor_E=0.;
    state.log_factor_B=0.;
    state.log_factor_P=0.;
    state.log_factor_common=0.;
    state.arrow_size=0.127;
    state.transparency_rendering_style=0;
    state.arrow_transparency_factor=1.;
    state.show_E=true;
    state.show_B=false;
    state.show_P=false;


    state.const_log_factor_E=1.6+5.;
    state.const_log_factor_B=5.;
    state.const_log_factor_P=10.;
    state.const_log_factor_common=-3.;


    int N=3;
    float radius = 0.05;
    float w = -0.002/radius;
    for (int n=0; n<N; n+=1){
        charges_vector.emplace_back(Vec3{0.,0.,0.}, Vec3{0.,0.,0.}, 0.02,1.,-1.,Vec3{1.,0.,0.},2,Vec4{0.,0.,0.,0.},Vec4{radius,0.,0.,w},Vec4{0.,radius,0.,2*PI*n/N});
        charges_vector.emplace_back(Vec3{0.,0.,0.}, Vec3{0.,0.,0.}, 0.02,-1.,-1.,Vec3{0.,0.,1.},2,Vec4{0.,0.,0.,0.},Vec4{radius,0.,0.,-w},Vec4{0.,radius,0.,2*PI*n/N});
    }

    //charges_vector.emplace_back(Vec3{0.,0.,0.}, Vec3{0.,0.,0.}, 0.02,1.,1.,Vec3{1.,0.,0.},2,Vec4{0.,0.,0.,0.},Vec4{0.,0.,-1.,0.},Vec4{w,0.,radius,0.});


}

void defineStage6(std::vector<MyCharge>& charges_vector, State& state){
    
    state.grid_size_x = 30;
    state.grid_size_y = 30;
    state.grid_size_z = 30;
    state.c = 0.01;
    state.k = 0.0002;

    float lambda = 2./20.;
    float A = 0.05/20.;

    state.log_factor_E=0.;
    state.log_factor_B=0.;
    state.log_factor_P=0.;
    state.log_factor_common=0.;
    state.arrow_size=0.127;
    state.transparency_rendering_style=1;
    state.arrow_transparency_factor=1.;
    state.show_E=true;
    state.show_B=false;
    state.show_P=false;


    state.const_log_factor_E=0.495+std::log10(1/(2.*state.k*36*A/lambda/lambda));
    state.const_log_factor_B=0.462+std::log10(1/(2.*state.k*36*A/lambda/lambda));
    state.const_log_factor_P=0.062+2*std::log10(1/(2.*state.k*36*A/lambda/lambda))+std::log10(12.*state.k/state.c/state.c);
    state.const_log_factor_common=-3.387;


    int N=12;
    int K=4;
    float alpha = 2*(float)PI/K;
    float sin_;
    for (int n=0; n<N; n+=1){
        addHerzt(charges_vector, Vec3{n*lambda/K,0.,0.}, lambda, Vec3{0.,std::cos(n*alpha)*A,std::sin(n*alpha)*A}, n*alpha, state.c, 1., 0.02);
    }
}

void defineStage7(std::vector<MyCharge>& charges_vector, State& state){
    
    state.grid_size_x = 30;
    state.grid_size_y = 30;
    state.grid_size_z = 30;
    state.c = 0.01;
    state.k = 0.0002;

    state.log_factor_E=0.167;
    state.log_factor_B=-0.276;
    state.log_factor_P=0.68;
    state.log_factor_common=-0.387;
    state.arrow_size=0.067;
    state.transparency_rendering_style=1;
    state.arrow_transparency_factor=1.;
    state.show_E=false;
    state.show_B=false;
    state.show_P=true;

    state.const_log_factor_E=1.84f;
    state.const_log_factor_B=-0.16f;
    state.const_log_factor_P=4.06f;
    state.const_log_factor_common=0.;
    
    std::cout<<state.const_log_factor_E<<" "<<state.const_log_factor_B<<" "<<state.const_log_factor_P<<std::endl;
    float v = 0.002;
    float e = 1.;
    float m = 1000.;
    float r = state.k * e * e/ 4 / v / v / m;
    //addAntenna(charges_vector,  1, {0.,0.,0.}, {2.,0.,0.},lambda,2.*A,state.c,.0000000005,0.02,true);
    charges_vector.emplace_back(Vec3{r,0.,0.},Vec3{0.,v,0.},0.02,e,m,Vec3{1.,0.,0.});
    charges_vector.emplace_back(Vec3{-r,0.,0.},Vec3{0.,-v,0.},0.02,-e,m,Vec3{0.,0.,1.});
}

void defineStage8(std::vector<MyCharge>& charges_vector, State& state){
    
    state.grid_size_x = 30;
    state.grid_size_y = 30;
    state.grid_size_z = 30;
    state.c = 0.01;
    state.k = 0.0002;

    state.log_factor_E=0.167;
    state.log_factor_B=-0.276;
    state.log_factor_P=0.68;
    state.log_factor_common=-0.387;
    state.arrow_size=0.067;
    state.transparency_rendering_style=1;
    state.arrow_transparency_factor=1.;
    state.show_E=false;
    state.show_B=false;
    state.show_P=true;

    state.const_log_factor_E=1.84f;
    state.const_log_factor_B=-0.16f;
    state.const_log_factor_P=4.06f;
    state.const_log_factor_common=0.;
    
    float v = 0.002;
    float e = 1.;
    float m = 1000.;
    float r = 1000.;
    float R = state.k * e * e / v / v / m * r/(r+1.);
    //addAntenna(charges_vector,  1, {0.,0.,0.}, {2.,0.,0.},lambda,2.*A,state.c,.0000000005,0.02,true);

    charges_vector.emplace_back(Vec3{-R/(r+1),0.,0.},Vec3{0.,-v/r,0.},0.04,e,r*m,Vec3{1.,0.,0.});
    charges_vector.emplace_back(Vec3{R*r/(r+1),0.,0.},Vec3{0.,v,0.},0.02,-e,m,Vec3{0.,0.,1.});
}

void defineStage9(std::vector<MyCharge>& charges_vector, State& state){
    
    state.grid_size_x = 30;
    state.grid_size_y = 30;
    state.grid_size_z = 30;
    state.c = 0.01;
    state.k = 0.0002;

    float lambda = 1.;
    float A = 0.05;

    state.log_factor_E=0.24;
    state.log_factor_B=0.524;
    state.log_factor_P=-0.2;
    state.log_factor_common=-0.387;
    state.arrow_size=0.067;
    state.transparency_rendering_style=1;
    state.arrow_transparency_factor=1.;
    state.show_E=true;
    state.show_B=false;
    state.show_P=false;

    state.const_log_factor_E=2.+std::log10(1/(2.*state.k*36*A/lambda/lambda));
    state.const_log_factor_B=std::log10(1/(2.*state.k*36*A/lambda/lambda));
    state.const_log_factor_P=2*std::log10(1/(2.*state.k*36*A/lambda/lambda))+std::log10(12.*state.k/state.c/state.c);
    state.const_log_factor_common=-3.;
    std::cout<<state.const_log_factor_E<<" "<<state.const_log_factor_B<<" "<<state.const_log_factor_P<<std::endl;
    addAntenna(charges_vector,  1, {0.,0.,0.}, {2.,0.,0.},lambda,2.*A,state.c,.0000000005,0.02,true);
}


void defineStage10(std::vector<MyCharge>& charges_vector, State& state){
    
    state.grid_size_x = 30;
    state.grid_size_y = 30;
    state.grid_size_z = 30;
    state.c = 0.01;
    state.k = 0.0002;

    state.log_factor_E=0.24;
    state.log_factor_B=0.524;
    state.log_factor_P=-0.2;
    state.log_factor_common=-0.387;
    state.arrow_size=0.067;
    state.transparency_rendering_style=1;
    state.arrow_transparency_factor=1.;
    state.show_E=true;
    state.show_B=false;
    state.show_P=false;

    state.const_log_factor_E=1.84f;
    state.const_log_factor_B=-0.16f;
    state.const_log_factor_P=4.06f;
    state.const_log_factor_common=0.;

    //addAntenna(charges_vector,  1, {0.,0.,0.}, {2.,0.,0.},lambda,2.*A,state.c,.0000000005,0.02,true);
    int n = 5;
    float R = 0.04f;
    for(int i=0; i<n; i++){
        charges_vector.emplace_back(Vec3{cos(i*2.f*PI/n)*2*R,sin(i*2.f*PI/n)*2*R,0.f},Vec3{cos(i*2.f*PI/n),sin(i*2.f*PI/n),0.f}*(-0.001),0.02f,.2f,300.f,Vec3{1.f,0.f,0.f});
    }
    //for(int i=0; i<n; i++){
    //    charges_vector.emplace_back(Vec3{cos(i*2.f*PI/n)*R,sin(i*2.f*PI/n)*R,0.f},Vec3{0.f,0.f,0.f},0.02f,-0.05f,30000.f,Vec3{0.f,0.f,1.f});
    //}
}
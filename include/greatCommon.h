#pragma once
#include <cmath>
#include <vector>
#include <functional>
#include "greatCommon.h"
const float PI = 3.14159265358979323846;
struct Vec3 {
    float x=0.,y=0.,z=0.;
    Vec3()=default;
    Vec3(float x0,float y0,float z0):x(x0),y(y0),z(z0){}
    Vec3 operator+(const Vec3& other) {return { x + other.x, y + other.y, z + other.z };}
    Vec3 operator-(const Vec3& other) {return { x - other.x, y - other.y, z - other.z };}
    Vec3 operator*(float scalar) {return {x*scalar, y*scalar, z*scalar};}
    Vec3 operator/(float scalar) {return {x/scalar, y/scalar, z/scalar};}
    float length(){return std::sqrt(x*x+y*y+z*z);}
    static Vec3 crossProduct(const Vec3& a, const Vec3& b) {
        return {
            a.y * b.z - a.z * b.y,  // x = ay*bz - az*by
            a.z * b.x - a.x * b.z,  // y = az*bx - ax*bz
            a.x * b.y - a.y * b.x   // z = ax*by - ay*bx
        };
    }
    static float dotProduct(const Vec3& a, const Vec3& b) {
        return {
            a.x * b.x + a.y * b.y + a.z * b.z
        };
    }
};

struct Vec4 {
    float x=0.,y=0.,z=0.,w=0.;
    Vec4()=default;
    Vec4(float x0,float y0,float z0,float w0):x(x0),y(y0),z(z0),w(w0){}
};

struct HistoryElement {
    Vec3 pos;
    float t;
    Vec3 vel;
    float q;
    Vec3 acc;
    float m;
};

struct StaticSphericalData {
    float m;
    float q;
    float r;
    int moving_mode;

    Vec4 color;
    Vec4 param1;
    Vec4 param2;
    Vec4 param3;
};

struct Matrix {
    public:
        float elements[16];

    static Matrix getViewMatrix(Vec3 cameraPos, Vec3 target, Vec3 up){
        Matrix view;
        Vec3 forward = (target - cameraPos)/(target-cameraPos).length();
        Vec3 right = Vec3::crossProduct(forward,up);
        right = right/right.length();
        Vec3 newUp = Vec3::crossProduct(right,forward);
        view.elements[0]=right.x;    view.elements[4]=right.y; view.elements[8]=right.z; view.elements[12]=-Vec3::dotProduct(right,cameraPos);
        view.elements[1]=newUp.x;    view.elements[5]=newUp.y; view.elements[9]=newUp.z; view.elements[13]=-Vec3::dotProduct(newUp,cameraPos);
        view.elements[2]=-forward.x;    view.elements[6]=-forward.y; view.elements[10]=-forward.z; view.elements[14]=Vec3::dotProduct(forward,cameraPos);
        view.elements[3]=0;   view.elements[7]=0; view.elements[11]=0; view.elements[15]=1;
        return view;
    }

    static Vec4 getCameraPos(Vec3 target, float distance, float angle_of_vertical, float angle_of_horizontal){
        Vec3 cameraDisplacement = {
            distance*std::cos(angle_of_vertical)*std::sin(angle_of_horizontal),
            distance*std::sin(angle_of_vertical),
            - distance*std::cos(angle_of_vertical)*std::cos(angle_of_horizontal)
        };
        Vec3 cameraPos = target + cameraDisplacement;
        return Vec4{cameraPos.x,cameraPos.y,cameraPos.z,1.};
    }


    static Matrix getViewMatrix(Vec3 target, float distance, float angle_of_vertical, float angle_of_horizontal){
        Vec3 cameraDisplacement = {
            distance*std::cos(angle_of_vertical)*std::sin(angle_of_horizontal),
            distance*std::sin(angle_of_vertical),
            - distance*std::cos(angle_of_vertical)*std::cos(angle_of_horizontal)
        };
        Vec3 cameraPos = target + cameraDisplacement;

        float cosVertical = std::cos(angle_of_vertical);
        float sinVertical = std::sin(angle_of_vertical);
        float cosHorizontal = std::cos(angle_of_horizontal);
        float sinHorizontal = std::sin(angle_of_horizontal);
        

        Vec3 up = {
            -sinVertical*sinHorizontal,
            cosVertical,
            sinVertical*cosHorizontal
        };
        return getViewMatrix(cameraPos, target, up);

    }

    static Matrix getProjectionMatrix(float field_of_view, float aspect_ratio, float near, float far){
        Matrix projection;
        float S = 1/std::tan(field_of_view/2.0f);
        
        projection.elements[0]=S/aspect_ratio;    projection.elements[4]=0; projection.elements[8]=0; projection.elements[12]=0;
        projection.elements[1]=0;    projection.elements[5]=S; projection.elements[9]=0; projection.elements[13]=0;
        projection.elements[2]=0;    projection.elements[6]=0; projection.elements[10]=(far+near)/(near-far); projection.elements[14]=(2*far*near)/(near-far);
        projection.elements[3]=0;   projection.elements[7]=0; projection.elements[11]=-1; projection.elements[15]=0;
        return projection;
    }

};

struct Uniforms {
    int buffer_offset;
    int history_element_size;
    int amount_of_spheres;
    int history_size;

    float c;
    float k;
    float arrow_size;
    float arrow_transparency_factor=1.;

    Matrix view;
    Matrix projection;

    Vec4 cameraPos;

    u_int display_mode=0;
    u_int grid_size_x;
    u_int grid_size_y;
    u_int grid_size_z;

    float factor_E;
    float factor_B;
    float factor_P;
    float factor_common;

    float time;
    float time_per_frame;
};

struct StageDefiner;

struct State {
    bool show_E = false;
    bool show_B = false;
    bool show_P = true;
    bool show_spheres = true;

    float arrow_size = 0.15;
    float arrow_transparency_factor = 1.;
    float log_factor_E = 0.;
    float log_factor_B = 0.;
    float log_factor_P = 0.;
    float log_factor_common = 0.;

    float angle_of_vertical = 0.;
    float angle_of_horizontal = 0.;
    float amplifying_factor = 1.;

    int buffer_offset = 0.;
    int last_buffer_offset = -1;
    bool paused = true;
    float time = 0.;
    float time_per_frame = 1.;
    //float log_time_per_frame = 0.;
  
    
    float c = 0.01;
    float k = 0.0002;

    float const_log_factor_E = 0.;
    float const_log_factor_B = 0.;
    float const_log_factor_P = 0.;
    float const_log_factor_common = 0.;

    int charge_detailing = 1000;
    int history_size = 512;
    u_int grid_size_x = 10u;
    u_int grid_size_y = 10u;
    u_int grid_size_z = 10u;

    int display_x=1920;
    int display_y=1080;
    float display_ratio = 16./9.;
    int transparency_rendering_style = 1;

    int amount_of_charges=0;

    std::vector<StageDefiner> stages;
    int currect_stage=0;
    int intended_to_be_selected_stage=-1;
    bool show_context_menu = false;
};

class MyCharge{
    private :

    public :
        Vec3 pos;
        Vec3 vel;
        Vec3 acc;
        Vec3 color;
        float r;
        float q;
        float m;
        int pattern;
        Vec4 param1;
        Vec4 param2;
        Vec4 param3;

        MyCharge(Vec3 pos_, Vec3 vel_, float r_, float q_, float m_, Vec3 color_, int pattern=0, Vec4 param1 = {0.,0.,0.,0.}, Vec4 param2 = {0.,0.,0.,0.}, Vec4 param3 = {0.,0.,0.,0.})
            :r(r_),
            q(q_),
            m(m_),
            pos(pos_),
            vel(vel_),
            color(color_),
            pattern(pattern),
            param1(param1),
            param2(param2),
            param3(param3)
        {

        }
        ~MyCharge(){

        }
};


void replaceBit(u_int& number, int n, bool newValue) {
    number &= ~(1 << n);
    number |= (static_cast<u_int>(newValue) << n);
}

long long binomialCoefficient(int n, int k) {
    if (k < 0 || k > n) return 0;
    if (k == 0 || k == n) return 1;
    if (k > n / 2) k = n - k; 

    long long res = 1;
    for (int i = 1; i <= k; ++i) {
        res = res * (n - i + 1) / i;
    }
    return res;
}

struct StageDefiner{
    std::function<void(std::vector<MyCharge>&, State&)> defineStage;
    std::string name;
};
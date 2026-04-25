#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <string>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>
#include <cmath>
#include <functional>
#include <memory>
#include <filesystem>
const double PI = 3.14159265358979323846;

class Logger{
    public:
        inline static int depth=0;
        inline static int off_depth=20;
        template<typename... Args>
        static void begin(Args... args){
            if (Logger::off_depth>Logger::depth){
                for (int i=0;i<Logger::depth;i++){
                    std::cout<<" - ";
                }
                (std::cout << ... << args)<<std::endl;
            }
            Logger::depth++;
        }
        static void off(){
            Logger::off_depth=Logger::depth;
        }
        template<typename... Args>
        static void log(Args... args){
            if (Logger::off_depth>Logger::depth){
                Logger::flog(args...);
            }
        }
        template<typename... Args>
        static void flog(Args... args){
            for (int i=0;i<Logger::depth;i++){
                std::cout<<" - ";
            }
            (std::cout << ... << args)<<std::endl;
        }
        template<typename... Args>
        static void end(Args... args){
            if (Logger::depth>0){
                Logger::depth--;
            }
            if (Logger::off_depth>Logger::depth){
                Logger::off_depth=20;
                for (int i=0;i<Logger::depth;i++){
                    std::cout<<" - ";
                }
                (std::cout << ... << args)<<std::endl;
            }
            if (Logger::off_depth==Logger::depth){
                Logger::off_depth=20;
            }
        }
};


bool fileExists(const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}
std::string get_text_from_file(std::string path){
    std::string text;
    std::ifstream file;
    std::stringstream stream;
    if (!fileExists(path)){
        std::cerr<<"File "<<path<<" doesnt exist!"<<std::endl;
        return "";
    }
    file.open(path);
    stream << file.rdbuf();
    file.close();
    text = stream.str();
    return text;
}
void replaceBit(u_int& number, int n, bool newValue) {
    number &= ~(1 << n);
    number |= (static_cast<u_int>(newValue) << n);
}
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
        Vec3 up = {
            std::cos(angle_of_vertical+M_PI/2)*std::sin(angle_of_horizontal),
            std::sin(angle_of_vertical+M_PI/2),
            - std::cos(angle_of_vertical+M_PI/2)*std::cos(angle_of_horizontal)
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


class ProgramHandler {
    private:
        unsigned int ID;
        bool hasComputeShader=false;
        unsigned int compute_ID;

        void checkErrorsShaders(unsigned int shader, std::string type) {
            int success;
            char infoLog[1024];
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cerr << "ERROR: Помилка компіляції шейдера: " << type << "\n" << infoLog << std::endl;
            }
        }

        void checkErrorsProgram(unsigned int ID, std::string type){
            if (!glIsProgram(ID)){
                Logger::log("Program doesnt exist!");
            }
            int success;
            char infoLog[1024];
            glGetProgramiv(ID, GL_LINK_STATUS, &success);
            if (success == GL_TRUE) {
                Logger::log("Program (",type,") is valid and linked!");
            } else {
                Logger::log("Program (",type,") exists but LINK FAILED!");
            }
        }

        unsigned int get_shader(std::string path, GLenum type){

            const std::string source = get_text_from_file(path);
            const char* souce_for_opengl = source.c_str();
            unsigned int shader = glCreateShader(type);
            glShaderSource(shader, 1, &souce_for_opengl, NULL);
            glCompileShader(shader);
            checkErrorsShaders(shader, path);
            return shader;
        }

    public:
        ProgramHandler(){

        }
        ProgramHandler(std::string path, std::vector<bool> shader_types){
            init(path,shader_types);
        }
        ProgramHandler(std::string path, std::vector<bool> shader_types, bool hasComputeShader_):ProgramHandler(path,shader_types){
            init(path,shader_types,hasComputeShader_);
        }
        void init(std::string path, std::vector<bool> shader_types){
            unsigned int vertShader;
            unsigned int geomShader;
            unsigned int fragShader;
            if (shader_types[0]){vertShader = get_shader(path+"/vertex.glsl",GL_VERTEX_SHADER);}
            if (shader_types[1]){geomShader = get_shader(path+"/geometry.glsl",GL_GEOMETRY_SHADER);}
            if (shader_types[2]){fragShader = get_shader(path+"/fragment.glsl",GL_FRAGMENT_SHADER);}
            ID = glCreateProgram();
            if (shader_types[0]){glAttachShader(ID, vertShader);}
            if (shader_types[1]){glAttachShader(ID, geomShader);}
            if (shader_types[2]){glAttachShader(ID, fragShader);}
            glLinkProgram(ID);
            checkErrorsProgram(ID, "render");
            if (shader_types[0]){glDeleteShader(vertShader);}
            if (shader_types[1]){glDeleteShader(geomShader);}
            if (shader_types[2]){glDeleteShader(fragShader);}
        }
        void init(std::string path, std::vector<bool> shader_types, bool hasComputeShader_){
            init(path,shader_types);
            hasComputeShader=hasComputeShader_;
            if(hasComputeShader){
                unsigned int compShader = get_shader(path+"/compute.glsl",GL_COMPUTE_SHADER);
                compute_ID = glCreateProgram();
                glAttachShader(compute_ID, compShader);
                glLinkProgram(compute_ID);
                checkErrorsProgram(compute_ID,"compute");
                glDeleteShader(compShader);
            }
        }
        ~ProgramHandler(){
            glDeleteProgram(ID);
            if (hasComputeShader){
                glDeleteProgram(compute_ID);
            }
        }
        void use() { 
            glUseProgram(ID);
        }
        void compute(unsigned int x, unsigned int y, unsigned int z) {
            if (!hasComputeShader){
                Logger::log("Програма не має compute shader");
            }
            glUseProgram(compute_ID);
            glDispatchCompute(x,y,z);
        }
        static void stop() {
            glUseProgram(0);
        }
};

class SSBOHandler{
    private:
        unsigned int ID;
    public:
        SSBOHandler(){

        }
        ~SSBOHandler(){
            glDeleteBuffers(1,&ID);
            ID=0;
        }
        void init(float data[], size_t data_size, GLuint index, GLenum usage){
            glGenBuffers(1,&ID);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID);
            glBufferData(GL_SHADER_STORAGE_BUFFER, data_size, data, usage);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, ID);
            Logger::log("(SSBO) buffer created: ",data_size," (BYTES)");
        }
        void update(float data[], size_t data_size, int start){
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, start, data_size, data);
        }
        void read(float data[], size_t data_size, int start){
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID);
            glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, start, data_size, data);
        }
        void bind(){
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID);
        }
};

class UBOHandler{
    private:
        unsigned int ID;
    public:
        Uniforms uniforms;
        UBOHandler (){
        }
        UBOHandler(Uniforms uniforms_init){
            init(uniforms_init);
        }
        void init(Uniforms uniforms_init) {
            glGenBuffers(1,&ID);
            uniforms=uniforms_init;
            glBindBuffer(GL_UNIFORM_BUFFER, ID);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(uniforms), NULL, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, 0, ID);
        }
        ~UBOHandler(){
            glDeleteBuffers(1,&ID);
            ID=0;
        }
        void update(){
            glBindBuffer(GL_UNIFORM_BUFFER, ID);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(uniforms), &uniforms);
        }
};

class VBOHandler{   
    private :
        struct Config{
            int layout, size, stride, offset, divisor;
            Config(int layout_, int size_, int stride_, int offset_, int divisor_){
                layout=layout_;
                size=size_;
                stride=stride_;
                offset=offset_;
                divisor=divisor_;
            }
        };
        unsigned int ID;
        std::vector<Config> configs;
    public : 
        VBOHandler(){
            
        }
        VBOHandler (float data[], size_t data_size, GLenum usage){
            init(data,data_size,usage);
        }
        void init(float data[], size_t data_size, GLenum usage){
            glGenBuffers(1,&ID);
            Logger::log("buffer created: ",data_size," (BYTES)");
            glBindBuffer(GL_ARRAY_BUFFER,ID);
            glBufferData(GL_ARRAY_BUFFER,data_size,data,usage);
            glBindBuffer(GL_ARRAY_BUFFER,0);
        }
        ~VBOHandler (){
            glDeleteBuffers(1,&ID);
            ID=0;
        }
        void bind(){
            glBindBuffer(GL_ARRAY_BUFFER,ID);
        }
        static void unbind(){
            glBindBuffer(GL_ARRAY_BUFFER,0);
        }
        void bind_configuration(){
            glBindBuffer(GL_ARRAY_BUFFER,ID);
            for (int config_i=0; config_i<configs.size(); config_i++){
                glEnableVertexAttribArray(configs[config_i].layout);
                glVertexAttribPointer(configs[config_i].layout,configs[config_i].size,GL_FLOAT, GL_FALSE,configs[config_i].stride,(void*)(uintptr_t) (configs[config_i].offset));
                glVertexAttribDivisor(configs[config_i].layout, configs[config_i].divisor);
            }
            
        }
        void update(float data[], size_t data_size, int start){
            glBindBuffer(GL_ARRAY_BUFFER,ID);
            glBufferSubData(GL_ARRAY_BUFFER, start, data_size, data);
        }
        void configure(int layout, int size, int stride, int offset, int divisor){
            Config config_ = Config(layout,size,stride,offset,divisor);  // divisor = 0 -  вершина  devisor = 1 - обєкт
            configs.push_back(config_);
        }
    
};

class VAOHandler{
    private:
        unsigned int ID;
    
    public:
        VAOHandler (){
            glGenVertexArrays(1, &ID);
        }
        ~VAOHandler (){
            glDeleteVertexArrays(1, &ID);
            ID=0;
        }
        void bind(){
            glBindVertexArray(ID);
        }
        static void unbind(){
            glBindVertexArray(0);
        }
};

class MyTriangle{
    private:
        VAOHandler vao;
        ProgramHandler program;

    public:
        MyTriangle(std::string path): program(path,{1,1,1}){
            float data[] = {
                0.0f,  0.5f, 0.0f,    1.0f, 0.0f, 0.0f,
                -0.5f, -0.5f, 0.0f,    0.0f, 1.0f, 0.0f,
                0.5f, -0.5f, 0.0f,    0.0f, 0.0f, 1.0f ,
            };
            VBOHandler vbo(data, sizeof(data), GL_STATIC_DRAW);
            vbo.configure(0,3,24,0,0);
            vbo.configure(1,3,24,12,0);
            vao.bind();
            vbo.bind_configuration();
            VAOHandler::unbind();
        }
        ~MyTriangle(){

        }
        void draw(){
            vao.bind();
            program.use();
            
            gl_Draw();

            VAOHandler::unbind();
            ProgramHandler::stop();
        }
        void gl_Draw(){
            glDrawArrays(GL_TRIANGLES, 0,3);
        }
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

class MyCharges{
    private :
        static Vec3 to_xyz(int i, int j, int max_i, int max_j){
            float phi=(float) i/max_i*PI;
            float theta=(float) j/max_j*2*PI;
            float x=std::sin(phi)*std::cos(theta);
            float y=std::sin(phi)*std::sin(theta);
            float z=std::cos(phi);
            return {x,y,z};
        };

        static std::vector<Vec3> getSphereVertices(int points_amount){
            std::vector<Vec3> sphere;
            int vertical_subdivision = std::floor(std::sqrt(points_amount));
            int horizontal_subdivision = vertical_subdivision;
            for (int i=0; i<vertical_subdivision; i++){
                for (int j=0; j<horizontal_subdivision; j++){
                    sphere.push_back(to_xyz(i, j, vertical_subdivision, horizontal_subdivision));
                    sphere.push_back(to_xyz(i+1, j, vertical_subdivision, horizontal_subdivision));
                    sphere.push_back(to_xyz(i, j+1, vertical_subdivision, horizontal_subdivision));
                    sphere.push_back(to_xyz(i+1, j, vertical_subdivision, horizontal_subdivision));
                    sphere.push_back(to_xyz(i+1, j+1, vertical_subdivision, horizontal_subdivision));
                    sphere.push_back(to_xyz(i, j+1, vertical_subdivision, horizontal_subdivision));
                }
            }
            return sphere;
        };
        

        std::vector<HistoryElement> getSphereHistoryElements(float t){
            std::vector<HistoryElement> elements;
            for (int i=0; i<charges.size(); i++){
                elements.push_back(
                    {
                        charges[i].pos,
                        t,
                        charges[i].vel,
                        charges[i].q,
                        charges[i].acc,
                        charges[i].m
                    }
                );
            }
            return elements;
        }

        std::vector<HistoryElement> getSphereHistoryInit(float t, int history_length){
            std::vector<HistoryElement> history;
            std::vector<HistoryElement> elements;
            history.reserve(history_length*charges.size());
            for (int i=-history_length; i<0; i++){
                elements = getSphereHistoryElements(i+t);
                history.insert(history.end(),elements.begin(),elements.end());
            }
            return history;
        }

        std::vector<StaticSphericalData> getSphereStaticInit(){
            std::vector<StaticSphericalData> data;
            data.reserve(charges.size());
            for (int i=0; i<charges.size(); i++){
                data.push_back({
                    charges[i].m,
                    charges[i].q,
                    charges[i].r,
                    charges[i].pattern,
                    Vec4{charges[i].color.x,charges[i].color.y,charges[i].color.z,1.},
                    charges[i].param1,
                    charges[i].param2,
                    charges[i].param3
                });
            }
            return data;
        }


        VBOHandler vbo_sphere_vertices;
        SSBOHandler ssbo_history;
        SSBOHandler ssbo_present;
        SSBOHandler ssbo_static;
        SSBOHandler ssbo_debugger;
        VAOHandler vao;
        ProgramHandler program;
        int vertices_amount;
        int charges_amount;

    public:
        std::vector<MyCharge> charges;
        int history_length;
        MyCharges(){

        }
        MyCharges(int points_amount_, int history_length, std::string path_, std::vector<MyCharge> charges_)
        {
            init(points_amount_,history_length,path_,charges_);
        }
        void init(int points_amount_, int history_length_, std::string path_, std::vector<MyCharge> charges_){
            charges=charges_;
            
            std::vector<Vec3> sphereVertices = MyCharges::getSphereVertices(points_amount_);
            vbo_sphere_vertices.init((float*) sphereVertices.data(), sphereVertices.size()*3*sizeof(float),GL_STATIC_DRAW);

            program.init(path_,{1,0,1},true);
            vertices_amount=sphereVertices.size();
            charges_amount=charges.size();
            history_length=history_length_;

            vbo_sphere_vertices.configure(0,3,0,0,0);
            
            vao.bind();
            vbo_sphere_vertices.bind_configuration();

            VAOHandler::unbind();

            std::vector<HistoryElement> history = getSphereHistoryInit(0,history_length);
            ssbo_history.init((float*) history.data(),history.size()*sizeof(HistoryElement), 0, GL_STATIC_DRAW);
            std::vector<StaticSphericalData> static_data = getSphereStaticInit();
            ssbo_static.init((float*) static_data.data(),static_data.size()*sizeof(StaticSphericalData), 2, GL_STATIC_DRAW);
            
            std::vector<HistoryElement> present = getSphereHistoryElements(0.);
            ssbo_present.init((float*) present.data(),present.size()*sizeof(HistoryElement), 3, GL_STATIC_DRAW);

            ssbo_debugger.init(NULL, 1000*sizeof(float), 4, GL_DYNAMIC_READ);
        }

        ~MyCharges(){                
            
        }

        void draw(){
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE); 
            program.use();
            vao.bind();
            glDrawArraysInstanced(GL_TRIANGLES, 0, vertices_amount, charges_amount);
            ProgramHandler::stop();
            VAOHandler::unbind();

            //std::vector<float> debugger_data(1000,0.);
            //ssbo_debugger.read(debugger_data.data(),debugger_data.size()*sizeof(float),0);
            //Logger::flog("debugger: ",debugger_data[0], ", ",debugger_data[1], ", ",debugger_data[2],", ",debugger_data[3],",  ",debugger_data[4],", ",debugger_data[5], ", ",debugger_data[5]);
        }

        void update(){
            program.compute((charges_amount+128-1)/128,1,1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        }
};
 
class MyArrows{
    private:
        ProgramHandler program;
        VAOHandler vao;
        int arrows_amount;

        SSBOHandler ssbo;
        
        float arrow_points[18] = {
            0.,0.,0.,    0.,0.1,0.8,   0.,-0.1,0.8, 
            0.,0.3,0.8,  0.,0.,1.,     0.,-0.3,0.8
        };
        VBOHandler vbo_arrow_points;
        static std::vector<Vec3> getPivotPoints(Vec3 start, Vec3 size, Vec3 step){
            std::vector<Vec3> pivot_points;
            for (float x=start.x; x<=start.x+size.x;x=x+step.x){
                for (float y=start.y; y<=start.y+size.y;y=y+step.y){
                    for (float z=start.z; z<=start.z+size.z;z=z+step.z){
                        pivot_points.push_back({x,y,z});
                    }
                }
            }
            return pivot_points;
        }


    public:
        u_int grid_size_x;
        u_int grid_size_y;
        u_int grid_size_z;
        MyArrows(){

        }
        void init(u_int grid_size_x_, u_int grid_size_y_, u_int grid_size_z_, std::string path){
            program.init(path,{1,0,1},true);
            vbo_arrow_points.init(arrow_points,sizeof(arrow_points),GL_STATIC_DRAW);
            grid_size_x = grid_size_x_;
            grid_size_y = grid_size_y_;
            grid_size_z = grid_size_z_;
            arrows_amount = grid_size_x * grid_size_y * grid_size_z_;
            

            vbo_arrow_points.configure(0,3,0,0,0);
            vao.bind();
            vbo_arrow_points.bind_configuration();
            VAOHandler::unbind();
            ssbo.init(NULL,arrows_amount*12*4,1,GL_DYNAMIC_COPY);
        }
        MyArrows (u_int grid_size_x, u_int grid_size_y, u_int grid_size_z, std::string path)
        {
            init(grid_size_x, grid_size_y, grid_size_z, path);
        }
        ~MyArrows (){

        }
        void draw(int blendMode){
            glEnable(GL_BLEND);
            if (blendMode==1){
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            }
            if (blendMode==0){
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
            glDepthMask(GL_FALSE);
            program.compute((grid_size_x+8-1)/8,(grid_size_y+8-1)/8,(grid_size_z+8-1)/8);
            vao.bind();
            program.use();
            glDrawArraysInstanced(GL_TRIANGLES, 0, 6, arrows_amount*3);
            //glDrawArrays(GL_POINTS, 0, points_amount);
            VAOHandler::unbind();
            ProgramHandler::stop();
        }

};

struct StageDefiner{
    std::function<void(std::vector<MyCharge>&, State&)> defineStage;
    std::string name;
};

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


int initEverything(sf::RenderWindow& window, uint x, uint y){
    sf::ContextSettings settings;
    settings.depthBits = 24;
    window.create(sf::VideoMode({x,y}), 
                sf::String("VOVASOFT_TECH"), 
                sf::Style::Default, 
                sf::State::Windowed,
                settings);
    Logger::begin("Initializing everything");
    Logger::log("Depth bits: ", settings.depthBits);
    Logger::log("size of StaticSphericalData: ", sizeof(StaticSphericalData));
    Logger::log("size of HistoryElement: ",sizeof(HistoryElement));
    Logger::log("size of Uniforms: ",sizeof(Uniforms));
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Помилка ініціалізації GLEW!" << std::endl;
        return -1;
    }
    if (!ImGui::SFML::Init(window, true)) {
        std::cerr << "Помилка ініціалізації ImGui!"<<std::endl;
        return -1;
    }
    //window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();



    const ImWchar* range = io.Fonts->GetGlyphRangesDefault();
    ImFont* myFont = io.Fonts->AddFontFromFileTTF("DejaVuSans.ttf", 32.0f,nullptr, range);
    io.FontDefault = myFont;
    if (myFont == nullptr) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            std::cerr << "Поточна директорія: " << cwd << std::endl;
        }
        std::cerr << "Помилка: Не вдалося завантажити DejaVuSans.ttf!" << std::endl;
    }

    Logger::end("Everything is initialized");
    return 0;
}

void drawOpenGL(sf::RenderWindow& window, MyCharges& charges, MyArrows& arrows, State& state, UBOHandler& ubo){

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ubo.update();
    if (!state.paused){
        charges.update();
    }
    if (state.show_spheres){
        charges.draw();
    }

    if (state.show_E || state.show_B || state.show_P){
        arrows.draw(state.transparency_rendering_style);
    }
    
    if (!state.paused){
        state.time += state.time_per_frame;
        state.last_buffer_offset = state.buffer_offset;
        state.buffer_offset = (int)floor(state.time);
    }
    
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        Logger::log("OpenGL Error: ",err);
    }
}

void drawImGui(sf::RenderWindow& window, sf::Time& elapsed, int& fps, State& state){


    ImGui::SFML::Update(window, elapsed);
    ImGui::Begin("Control Panel 1234567890");
    if (ImGui::BeginCombo(" ", state.stages[state.currect_stage].name.c_str()))
    {
        for (int i = 0; i < state.stages.size(); i++)
        {
            const bool is_selected = (state.currect_stage == i);
            if (ImGui::Selectable(state.stages[i].name.c_str(), is_selected))
                state.intended_to_be_selected_stage = i;
                
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::Text((std::string("FPS: ")+std::to_string(fps)).c_str());
    if (ImGui::Button(state.paused? "Run": "Stop")) {state.paused=!state.paused;}
    ImGui::DragFloat("Angle (1)", &state.angle_of_vertical,0.3f, -90.f, 90.f);
    ImGui::DragFloat("Angle (2)", &state.angle_of_horizontal,0.3f, 0.f, 360.f);
    ImGui::SliderFloat("Arrows' size", &state.arrow_size,0.01f, 0.3f);
    ImGui::SliderFloat("Arrows' transparency", &state.arrow_transparency_factor,0.05f, 1.f);
    ImGui::Checkbox("Electric Field", &state.show_E);
    ImGui::SliderFloat("Electric Field Factor", &state.log_factor_E,-2.0f, 2.f);
    ImGui::Checkbox("Magnetic Field", &state.show_B);
    ImGui::SliderFloat("Magnetic Field Factor", &state.log_factor_B,-2.0f, 2.f);
    ImGui::Checkbox("Energy Flow", &state.show_P);
    ImGui::SliderFloat("Energy Flow Factor", &state.log_factor_P,-2.0f, 2.f);
    ImGui::SliderFloat("Common Factor", &state.log_factor_common,-2.f, 2.f);
    ImGui::SliderFloat("Amplifying Factor", &state.amplifying_factor,0.8f, 5.f);
    ImGui::SliderInt("Transparency rendering", &state.transparency_rendering_style,0,1);
    ImGui::End();
}

void applyStateToUbo(State& state, UBOHandler& ubo){
    replaceBit(ubo.uniforms.display_mode,0,state.show_E);
    replaceBit(ubo.uniforms.display_mode,1,state.show_B);
    replaceBit(ubo.uniforms.display_mode,2,state.show_P);
    replaceBit(ubo.uniforms.display_mode,3,state.show_spheres);

    ubo.uniforms.arrow_size = state.arrow_size;
    ubo.uniforms.arrow_transparency_factor = state.arrow_transparency_factor;

    ubo.uniforms.view = Matrix::getViewMatrix({0.,0.,0.}, 2., state.angle_of_vertical/180.*PI, state.angle_of_horizontal/180.*PI);
    ubo.uniforms.projection = Matrix::getProjectionMatrix(45.f/180.*PI, (float)state.display_ratio, 0.1f, 5.f);
    
    ubo.uniforms.cameraPos = Matrix::getCameraPos({0.,0.,0.}, 2., state.angle_of_vertical/180.*PI, state.angle_of_horizontal/180.*PI);

    ubo.uniforms.buffer_offset = state.buffer_offset;
    
    ubo.uniforms.factor_E = std::pow(10,state.log_factor_E+state.const_log_factor_E);
    ubo.uniforms.factor_B = std::pow(10,state.log_factor_B+state.const_log_factor_B);
    ubo.uniforms.factor_P = std::pow(10,state.log_factor_P+state.const_log_factor_P);
    ubo.uniforms.factor_common = std::pow(10,state.log_factor_common+state.const_log_factor_common);

    ubo.uniforms.c = state.c;
    ubo.uniforms.k = state.k;
    ubo.uniforms.grid_size_x = state.grid_size_x;
    ubo.uniforms.grid_size_y = state.grid_size_y;
    ubo.uniforms.grid_size_z = state.grid_size_z;
    ubo.uniforms.history_size = state.history_size;

    ubo.uniforms.amount_of_spheres = state.amount_of_charges;
    ubo.uniforms.history_element_size = state.amount_of_charges * sizeof(HistoryElement);

    ubo.uniforms.time = state.time;
    ubo.uniforms.time_per_frame = state.time_per_frame;
}


class Stage {
    private:

    public:
        std::vector<MyCharge> charges_vector;
        std::function<void(std::vector<MyCharge>&,State&)> defineStage;
        MyCharges charges;
        MyArrows arrows;
        Stage(std::function<void(std::vector<MyCharge>&, State&)> defineStage)
        :
        defineStage(defineStage)
        {
        }
        void activate(sf::RenderWindow& window, State& state){
            window.pushGLStates();
            Logger::begin("Activating the stage... ");
            Logger::log("defining stage");
            defineStage(charges_vector,state);
            state.time=0;
            state.buffer_offset=-1;
            state.amount_of_charges = charges_vector.size();
            Logger::begin("initializing charges");
            charges.init(state.charge_detailing,state.history_size,"shaders/spheres_shader",charges_vector);
            Logger::end("Charges is initialized");
            Logger::begin("initializing arrows");
            arrows.init(state.grid_size_x, state.grid_size_y, state.grid_size_z, "shaders/arrows_shader");
            Logger::end("arrows are ready");
            Logger::end("Stage is activated");
            window.popGLStates();
        }
        void draw(sf::RenderWindow& window, sf::Time& elapsed, int fps, UBOHandler& ubo, State& state){
            Logger::off();
            Logger::begin("trying to draw");
            if (state.show_context_menu){
                Logger::log("drawing ImGui");
                drawImGui(window, elapsed, fps, state);
            }

            window.clear();
            Logger::log("clearing GL_DEPTH_BUFFER_BIT");
            Logger::log("pushing GLStates");
            window.pushGLStates();
            applyStateToUbo(state,ubo);
            Logger::begin("drawing OpenGL");
            drawOpenGL(window,charges,arrows,state,ubo);
            Logger::end("opengl drawwed");
            Logger::log("popping GLStates");
            window.popGLStates();
            if (state.show_context_menu){
                Logger::log("rendering SFML");
                ImGui::SFML::Render(window);
            }

            Logger::end("drawwed");
        }
        ~Stage(){
            Logger::log("Stage is being destructed");
        }
};

std::unique_ptr<Stage> createStage(std::function<void(std::vector<MyCharge>&, State&)> defineStage) {
    return std::make_unique<Stage>(defineStage);
}

void defineStage1(std::vector<MyCharge>& charges_vector, State& state){
    
    state.grid_size_x= 70;
    state.grid_size_y= 70;
    state.grid_size_z= 70;
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

    float lambda = .5;
    float A = 0.025;

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

    //addAntenna(charges_vector,  1, {0.,0.,0.}, {2.,0.,0.},lambda,2.*A,state.c,.0000000005,0.02,true);
    for(int i=0; i<2; i++){
        charges_vector.emplace_back(Vec3{(i-1)*0.05,0.,0.},Vec3{0.,0.,0.},0.02,2.,300.,Vec3{1.,0.,0.});
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

    state.log_factor_E=-0.367;
    state.log_factor_B=-0.276;
    state.log_factor_P=-0.;
    state.log_factor_common=-0.387;
    state.arrow_size=0.127;
    state.transparency_rendering_style=0;
    state.arrow_transparency_factor=1.;
    state.show_E=false;
    state.show_B=false;
    state.show_P=true;



    state.const_log_factor_E=std::log10(1/(2.*state.k*36*A/lambda/lambda));
    state.const_log_factor_B=std::log10(1/(2.*state.k*36*A/lambda/lambda));
    state.const_log_factor_P=2*std::log10(1/(2.*state.k*36*A/lambda/lambda))+std::log10(12.*state.k/state.c/state.c)+2;
    state.const_log_factor_common=-3.;

    MyCharge charge(Vec3{0,0.,0.}, Vec3{0.,0.,0.}, 0.1,1.,-1.,Vec3{1.,0.,0.},3,Vec4{-10.,0.,0.,0.},Vec4{0.0085,1.,0.,0.},Vec4{0.,0.,0.,0.});
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
    state.const_log_factor_P=6.;
    state.const_log_factor_common=-3.;


    int N=3;
    float radius = 0.05;
    float w = -0.002/radius;
    for (int n=0; n<N; n+=1){
        charges_vector.emplace_back(Vec3{0.,0.,0.}, Vec3{0.,0.,0.}, 0.02,1.,-1.,Vec3{1.,0.,0.},2,Vec4{0.,0.,0.,0.},Vec4{0.,0.,-1.,0.},Vec4{w,2*PI*n/N,radius,0.});
        charges_vector.emplace_back(Vec3{0.,0.,0.}, Vec3{0.,0.,0.}, 0.02,-1.,-1.,Vec3{0.,0.,1.},2,Vec4{0.,0.,0.,0.},Vec4{0.,0.,-1.,0.},Vec4{-w,2*PI*n/N,radius,0.});
    }

    //charges_vector.emplace_back(Vec3{0.,0.,0.}, Vec3{0.,0.,0.}, 0.02,1.,1.,Vec3{1.,0.,0.},2,Vec4{0.,0.,0.,0.},Vec4{0.,0.,-1.,0.},Vec4{w,0.,radius,0.});


}

void defineStage6(std::vector<MyCharge>& charges_vector, State& state){
    
    state.grid_size_x = 30;
    state.grid_size_y = 30;
    state.grid_size_z = 30;
    state.c = 0.01;
    state.k = 0.0002;

    float lambda = .5/3.;
    float A = 0.025/3.;

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


    int N=8;
    int K=4;
    float alpha = 2*PI/K;
    for (int n=0; n<N; n+=1){
        addHerzt(charges_vector, Vec3{n*lambda/K-1.5,0.,0.}, lambda, Vec3{0.,cos(n*alpha)*A,sin(n*alpha)*A}, n*alpha, state.c, 1., 0.02);
    }
}

void defineStage7(std::vector<MyCharge>& charges_vector, State& state){
    
    state.grid_size_x = 30;
    state.grid_size_y = 30;
    state.grid_size_z = 30;
    state.c = 0.01;
    state.k = 0.0002;

    float lambda = .5;
    float A = 0.025;

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



    state.const_log_factor_E=2.+std::log10(1/(2.*state.k*36*A/lambda/lambda));
    state.const_log_factor_B=std::log10(1/(2.*state.k*36*A/lambda/lambda));
    state.const_log_factor_P=2*std::log10(1/(2.*state.k*36*A/lambda/lambda))+std::log10(12.*state.k/state.c/state.c);
    state.const_log_factor_common=-3.;
    
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

    float lambda = .5;
    float A = 0.025;

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



    state.const_log_factor_E=2.+std::log10(1/(2.*state.k*36*A/lambda/lambda));
    state.const_log_factor_B=std::log10(1/(2.*state.k*36*A/lambda/lambda));
    state.const_log_factor_P=2*std::log10(1/(2.*state.k*36*A/lambda/lambda))+std::log10(12.*state.k/state.c/state.c);
    state.const_log_factor_common=-3.;
    
    float v = 0.002;
    float e = 1.;
    float m = 1000.;
    float r = 1000.;
    float R = state.k * e * e / v / v / m * r/(r+1.);
    //addAntenna(charges_vector,  1, {0.,0.,0.}, {2.,0.,0.},lambda,2.*A,state.c,.0000000005,0.02,true);

    charges_vector.emplace_back(Vec3{-R/(r+1),0.,0.},Vec3{0.,-v/r,0.},0.04,e,r*m,Vec3{1.,0.,0.});
    charges_vector.emplace_back(Vec3{R*r/(r+1),0.,0.},Vec3{0.,v,0.},0.02,-e,m,Vec3{0.,0.,1.});
}

int main() {
    sf::RenderWindow window;
    sf::Clock clock;
    sf::Time elapsed;
    int fps;
    State state;
    if (initEverything(window,state.display_x,state.display_y)){return -1;}//if (initEverything(window,1280,720)){return -1;}
    Uniforms uniforms_init;
    UBOHandler ubo(uniforms_init);


    state.stages={
        {defineStage1, "Single Antena"},
        {defineStage2, "Interfearing Antenas"},
        {defineStage3, "Simpel Herz"},
        {defineStage4, "Moving Charge"},
        {defineStage5, "Cyclotron"},
        {defineStage6, "circular polarization"},
        {defineStage7, "electron - positron"},
        {defineStage8, "atom"}
    };
    std::unique_ptr<Stage> stage = createStage(state.stages[state.currect_stage].defineStage);
    stage->activate(window,state);
    while (window.isOpen()) {
        // Рахуємо FPS
        elapsed = clock.restart();
        fps = static_cast<int>(1.0f / elapsed.asSeconds());

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::KeyPressed>() && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P)){
                state.paused = !state.paused;
            }
            if (event->is<sf::Event::KeyPressed>() && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::K)){
                state.show_context_menu = !state.show_context_menu;
            }
            ImGui::SFML::ProcessEvent(window, *event);
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        stage->draw(window,elapsed,fps,ubo,state);
        window.display();

        if(state.intended_to_be_selected_stage>-1){
            state.currect_stage=state.intended_to_be_selected_stage;
            state.intended_to_be_selected_stage=-1;
            stage = createStage(state.stages[state.currect_stage].defineStage);
            stage->activate(window,state);
        }
    }
    return 0;
} 
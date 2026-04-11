#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <string>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include "imgui.h"
#include "imgui-SFML.h"
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
    float length(){return std::sqrt(x*x+y*y+z*z);}
    static Vec3 crossProduct(const Vec3& a, const Vec3& b) {
    return {
        a.y * b.z - a.z * b.y,  // x = ay*bz - az*by
        a.z * b.x - a.x * b.z,  // y = az*bx - ax*bz
        a.x * b.y - a.y * b.z   // z = ax*by - ay*bx
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

struct Uniforms {
    int buffer_offset;
    int history_element_size;
    int amount_of_spheres;
    int history_size;

    float c;
    float k;
    float arrow_size;
    float arrow_transparency_factor=1.;

    float amplifying_factor = 1.;
    float display_ratio;
    float angle_of_rotation=1.5;
    float angle_of_rotation_2=.11;

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
    float log_time_per_frame = 0.;
  
    
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
    float display_ratio = 9./16.;
    int transparency_rendering_style = 1;

    int amount_of_charges=0;

    std::vector<StageDefiner> stages;
    int currect_stage=0;
    int intended_to_be_selected_stage=-1;
    bool show_context_menu = false;
};



using MovementPattern = std::function<HistoryElement(float time)>;

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
                for (int i=0;i<Logger::depth;i++){
                    std::cout<<" - ";
                }
                (std::cout << ... << args)<<std::endl;
            }
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
        MovementPattern pattern;
        MyCharge(Vec3 pos_, float r_, float q_, float m_, Vec3 color_)
            :r(r_),
            q(q_),
            m(m_),
            pos(pos_),
            color(color_)
        {

        }
        MyCharge(Vec3 pos_, float r_, float q_, float m_, Vec3 color_, MovementPattern pattern)
            :r(r_),
            q(q_),
            m(m_),
            pos(pos_),
            color(color_),
            pattern(pattern)
        {

        }
        void update(float time){
            if (pattern){
                HistoryElement new_state = pattern(time);
                pos=new_state.pos;
                vel=new_state.vel;
                acc=new_state.acc;
            }
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
        
        static std::vector<Vec4> getSphereColors(std::vector<MyCharge> charges_){
            std::vector<Vec4> colors;
            for (int i=0; i<charges_.size();i++){
                colors.push_back({charges_[i].color.x,charges_[i].color.y,charges_[i].color.z,charges_[i].r});
            }
            return colors;
        }

        static std::vector<Vec3> getSpherePositions(std::vector<MyCharge> charges_){
            std::vector<Vec3> positions;
            for (int i=0; i<charges_.size();i++){
                positions.push_back(charges_[i].pos);
            }
            return positions;
        }


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
                update_static_charges(i+t);
                elements = getSphereHistoryElements(i+t);
                history.insert(history.end(),elements.begin(),elements.end());
            }
            return history;
        }

        VBOHandler vbo_sphere_color;
        VBOHandler vbo_sphere_position;
        VBOHandler vbo_sphere_vertices;
        SSBOHandler ssbo;
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
            std::vector<Vec4> sphereColors = MyCharges::getSphereColors(charges_);
            std::vector<Vec3> spherePositions = MyCharges::getSpherePositions(charges_);
            vbo_sphere_position.init((float*) spherePositions.data(), spherePositions.size()*3*sizeof(float), GL_DYNAMIC_DRAW);
            vbo_sphere_vertices.init((float*) sphereVertices.data(), sphereVertices.size()*3*sizeof(float),GL_STATIC_DRAW);
            vbo_sphere_color.init((float*) sphereColors.data(), sphereColors.size()*4*sizeof(float), GL_STATIC_DRAW);
            program.init(path_,{1,0,1},true);
            vertices_amount=sphereVertices.size();
            charges_amount=charges.size();
            history_length=history_length_;


            vbo_sphere_vertices.configure(0,3,0,0,0);
            vbo_sphere_position.configure(1,3,0,0,1);
            vbo_sphere_color.configure(2,4,0,0,1);
            
            vao.bind();
            vbo_sphere_vertices.bind_configuration();
            vbo_sphere_position.bind_configuration();
            vbo_sphere_color.bind_configuration();
            VAOHandler::unbind();

            std::vector<HistoryElement> history = getSphereHistoryInit(0,history_length);
            ssbo.init((float*) history.data(),history.size()*sizeof(HistoryElement),0, GL_DYNAMIC_DRAW);

        }

        ~MyCharges(){                
            
        }

        void update_static_charges(float offset){
            for (int i=0; i<charges.size();i++){
                charges[i].update(offset);
            }
        }


        void update(int last_buffer_offset, int buffer_offset, float time){
            std::vector<HistoryElement> new_elements;
            std::vector<HistoryElement> temp_new_elements;
            Logger::log("buffer offset ", buffer_offset);
            Logger::log("last_buffer_offset ", last_buffer_offset);
            for (int offset = last_buffer_offset+1; offset<=buffer_offset; offset++){
                update_static_charges((float) offset);
                temp_new_elements = MyCharges::getSphereHistoryElements((float) offset);
                new_elements.insert(new_elements.end(),temp_new_elements.begin(),temp_new_elements.end());
            }
            if (last_buffer_offset == buffer_offset){

                //std::cout<<"skipping updating buffer"<<std::endl;
                // в теорії тут можна оновити остайній елемент історії але я не бачу з цього багато сенсу
            } else {
                if (((last_buffer_offset+1)%history_length)>(buffer_offset%history_length)) {
                    int amount_staying_in_the_end = history_length - 1 - (last_buffer_offset%history_length);
                    int total_amount = buffer_offset - last_buffer_offset;
                    ssbo.update(
                        (float*) &new_elements[0],
                        sizeof(HistoryElement)*charges.size()*amount_staying_in_the_end,
                        sizeof(HistoryElement)*charges.size()*((last_buffer_offset+1)%history_length)
                    );
                    ssbo.update(
                        (float*) &new_elements[amount_staying_in_the_end*charges.size()],
                        sizeof(HistoryElement)*charges.size()*(total_amount-amount_staying_in_the_end),
                        0
                    );
                } else {
                    //std::cout<<"inserting "<<sizeof(HistoryElement)*new_elements.size()<<" bytes in "<<sizeof(HistoryElement)*charges.size()*((last_buffer_offset+1)%history_length)<<std::endl;
                    ssbo.update(
                        (float*) &new_elements[0],
                        sizeof(HistoryElement)*new_elements.size(),
                        sizeof(HistoryElement)*charges.size()*((last_buffer_offset+1)%history_length)
                    );
                    //std::cout<<"writing in ssbo: "<<sizeof(HistoryElement)*new_elements.size()<<" bytes"<<"  in  "<< sizeof(HistoryElement)*charges.size()*((last_buffer_offset+1)%history_length) <<std::endl;
                }
            }

            for (int i=0; i<charges.size();i++){
                charges[i].update(time);
            }
            std::vector<Vec3> new_pos = MyCharges::getSpherePositions(charges);
            vbo_sphere_position.update((float*) new_pos.data(), sizeof(Vec3)*new_pos.size(),0);
        }
        void draw(){
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE); 
            program.use();
            vao.bind();
            glDrawArraysInstanced(GL_TRIANGLES, 0, vertices_amount, charges_amount),
            ProgramHandler::stop();
            VAOHandler::unbind();
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

MovementPattern circularMotion(Vec3 center, float radius, float vel, float phase){
    float w=vel/radius;
    MovementPattern motion = [=](float time)->HistoryElement{
        return {
            {center.x+radius*((float)sin(phase+w*time)),center.y+radius*((float)cos(phase+w*time)), center.z}, time,
            {radius*((float)cos(phase+w*time))*w, -radius*((float)sin(phase+w*time))*w, 0.}, 0.,
            {-radius*((float)sin(phase+w*time))*w*w,-radius*((float)cos(phase+w*time))*w*w,0.},0.
        };
    };
    return motion;
}

MovementPattern occilation(Vec3 center, Vec3 direction, float vel,float phase){
    float w=vel/std::sqrt(direction.x*direction.x+direction.y*direction.y+direction.z*direction.z);
    MovementPattern motion = [=](float time)->HistoryElement{
        return {
            {center.x+direction.x*((float)sin(w*time+phase)),center.y+direction.y*((float)sin(w*time+phase)), center.z+direction.z*((float)sin(w*time+phase))}, time,
            {direction.x*w*((float)cos(w*time+phase)), direction.y*w*((float)cos(w*time+phase)), direction.z*w*((float)cos(w*time+phase))}, 0.,
            {-direction.x*w*w*((float)sin(w*time+phase)), -direction.y*w*w*((float)sin(w*time+phase)), -direction.z*w*w*((float)sin(w*time+phase))},0.
        };
    };
    return motion;
}

MovementPattern lineMotion(Vec3 start, Vec3 direction, float vel){
    float w=vel/std::sqrt(direction.x*direction.x+direction.y*direction.y+direction.z*direction.z);
    MovementPattern motion = [=](float time)->HistoryElement{
        return {
            {start.x+direction.x*vel*time,start.y+direction.y*w*time, start.z+direction.z*w*time}, time,
            {direction.x*w, direction.y*w, direction.z*w}, 0.,
            {0.,0.,0.},0.
        };
    };
    return motion;
}

void addHerzt(std::vector<MyCharge>& charges, Vec3 position, float lambda, Vec3 amplitude, float phase, float c, float charge, float radius){
    float v = amplitude.length()*2*PI*c/lambda;
    MyCharge charge1(position,radius,charge,-1.,{1.,0.,0.},occilation(position,amplitude,v,phase));
    MyCharge charge2(position,radius,-charge,-1.,{0.,0.,1.},occilation(position,amplitude*(-1),v,phase));
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


int initEverything(sf::RenderWindow& window, int x, int y){
    sf::ContextSettings settings;
    settings.depthBits = 24;
    window.create(sf::VideoMode(x, y), "VOVASOFT_TECH", sf::Style::Default, settings);
    Logger::begin("Initializing everything");
    Logger::log("Depth bits: ", settings.depthBits);
    Logger::log("size of HistoryElement: ",sizeof(HistoryElement));
    Logger::log("size of Uniforms: ",sizeof(Uniforms));
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Помилка ініціалізації GLEW!" << std::endl;
        return -1;
    }
    if (!ImGui::SFML::Init(window)) {
        std::cerr << "Помилка ініціалізації ImGui!"<<std::endl;
        return -1;
    }
    window.setVerticalSyncEnabled(true);
    //window.setFramerateLimit(60);

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear(); // Очищаємо старі шрифти



    const ImWchar* range = io.Fonts->GetGlyphRangesCyrillic();
    ImFont* myFont = io.Fonts->AddFontFromFileTTF("DejaVuSans.ttf", 20.0f, nullptr, range);
    if (myFont == nullptr) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            std::cerr << "Поточна директорія: " << cwd << std::endl;
        }
        std::cerr << "Помилка: Не вдалося завантажити DejaVuSans.ttf!" << std::endl;
    }
    if (!ImGui::SFML::UpdateFontTexture()) {
        std::cerr << "Помилка: Не вдалося оновити текстуру шрифту!" << std::endl;
    }
    Logger::end("Everything is initialized");
    return 0;
}

void drawOpenGL(sf::RenderWindow& window, MyCharges& charges, MyArrows& arrows, State& state, UBOHandler& ubo){

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (!state.paused){
        state.last_buffer_offset = state.buffer_offset;
        state.buffer_offset = (int)floor(state.time);
        charges.update(state.last_buffer_offset,state.buffer_offset,state.time);
    }
    if (state.show_spheres){
        charges.draw();
    }


    ubo.update();
    if (state.show_E || state.show_B || state.show_P){
        arrows.draw(state.transparency_rendering_style);
    }
    
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        Logger::log("OpenGL Error: ",err);
    }
    if (!state.paused){
        state.time += state.time_per_frame;
    }
}

void drawImGui(sf::RenderWindow& window, sf::Time& elapsed, int& fps, State& state){


    ImGui::SFML::Update(window, elapsed);
    ImGui::Begin("Control Panel");
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
    ImGui::SliderFloat("Angle (1)", &state.angle_of_vertical, -90.f, 90.f);
    ImGui::SliderFloat("Angle (2)", &state.angle_of_horizontal, 0.f, 360.f);
    ImGui::SliderFloat("Arrows' size", &state.arrow_size,0.05f, 0.3f);
    ImGui::SliderFloat("Arrows' transparency", &state.arrow_transparency_factor,0.05f, 1.f);
    ImGui::Checkbox("Electric Field", &state.show_E);
    ImGui::SliderFloat("Electric Field Factor", &state.log_factor_E,-2.0f, 2.f);
    ImGui::Checkbox("Magnetic Field", &state.show_B);
    ImGui::SliderFloat("Magnetic Field Factor", &state.log_factor_B,-2.0f, 2.f);
    ImGui::Checkbox("Energy Flow", &state.show_P);
    ImGui::SliderFloat("Energy Flow Factor", &state.log_factor_P,-2.0f, 2.f);
    ImGui::SliderFloat("Common Factor", &state.log_factor_common,-2.f, 2.f);
    ImGui::SliderFloat("Amplifying Factor", &state.amplifying_factor,0.8f, 5.f);
    ImGui::SliderFloat("Time Control Factor", &state.log_time_per_frame,-2.f, 2.f);
    ImGui::SliderInt("Transparency rendering", &state.transparency_rendering_style,0,1);
    ImGui::End();
    state.time_per_frame = std::pow(10,state.log_time_per_frame);
}

void applyStateToUbo(State& state, UBOHandler& ubo){
    replaceBit(ubo.uniforms.display_mode,0,state.show_E);
    replaceBit(ubo.uniforms.display_mode,1,state.show_B);
    replaceBit(ubo.uniforms.display_mode,2,state.show_P);
    replaceBit(ubo.uniforms.display_mode,3,state.show_spheres);

    ubo.uniforms.arrow_size = state.arrow_size;
    ubo.uniforms.arrow_transparency_factor = state.arrow_transparency_factor;
    ubo.uniforms.angle_of_rotation_2 = state.angle_of_vertical/180.*PI;
    ubo.uniforms.angle_of_rotation = state.angle_of_horizontal/180.*PI;
    ubo.uniforms.amplifying_factor = state.amplifying_factor;

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
    ubo.uniforms.display_ratio = state.display_ratio;

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
            //Logger::off();
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
    state.const_log_factor_P=state.const_log_factor_B+state.const_log_factor_E+std::log10(12.*state.k/state.c/state.c);
    state.const_log_factor_common=-3.;

    addAntenna(charges_vector,  3, {0.,0.,0.}, {2.,0.,0.},lambda,2.*A,state.c,.000000005,0.1/5.,true);
}

void defineStage3(std::vector<MyCharge>& charges_vector, State& state){
    
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

void defineStage2(std::vector<MyCharge>& charges_vector, State& state){
    
    state.grid_size_x = 30;
    state.grid_size_y = 30;
    state.grid_size_z = 30;
    state.c = 0.01;
    state.k = 0.0002;

    float lambda = .5;
    float A = 0.025;

    state.log_factor_E=0.167;
    state.log_factor_B=-0.276;
    state.log_factor_P=-1.536;
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

    addAntenna(charges_vector,  1, {0.,0.,0.}, {2.,0.,0.},lambda,2.*A,state.c,.000000005,0.02,true);
    MyCharge charge({0.5,0.,0.},0.02,1.,.01,{1.,0.,0.});
    charges_vector.push_back(charge);
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

    MyCharge charge({0,0.,0.},0.1,1.,-1.,{1.,0.,0.},lineMotion({-10,0.,0.},{1.,0.,0.},0.0085));
    charges_vector.push_back(charge);
}

void defineStage5(std::vector<MyCharge>& charges_vector, State& state){
    state.grid_size_x = 30;
    state.grid_size_y = 30;
    state.grid_size_z = 30;
    state.c = 0.01;
    state.k = 0.0002;

    float lambda = .5*4;
    float A = 0.025*4;

    state.log_factor_E=-0.367;
    state.log_factor_B=-0.276;
    state.log_factor_P=-1.536;
    state.log_factor_common=-0.387;
    state.arrow_size=0.127;
    state.transparency_rendering_style=0;
    state.arrow_transparency_factor=1.;
    state.show_E=true;
    state.show_B=false;
    state.show_P=false;


    state.const_log_factor_E=std::log10(1/(2.*state.k*36*A/lambda/lambda));
    state.const_log_factor_B=state.const_log_factor_E;
    state.const_log_factor_P=state.const_log_factor_B+state.const_log_factor_E+std::log10(12.*state.k/state.c/state.c);
    state.const_log_factor_common=-3.;


    int N=3;
    for (int n=0; n<N; n+=1){
        charges_vector.emplace_back(Vec3{0.,0.,0.},0.02,1,-1.,Vec3{1.,0.,0.},circularMotion({0,0,0},0.05,0.002,2*PI*n/N));
        charges_vector.emplace_back(Vec3{0.,0.,0.},0.02,-1,-1.,Vec3{0.,0.,1.},circularMotion({0,0,0},0.05,-0.002,2*PI*n/N));
    }
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


int main() {
    sf::RenderWindow window;
    sf::Clock clock;
    sf::Time elapsed;
    sf::Event event;
    int fps;
    State state;
    if (initEverything(window,state.display_x,state.display_y)){return -1;}//if (initEverything(window,1280,720)){return -1;}
    Uniforms uniforms_init;
    UBOHandler ubo(uniforms_init);


    state.stages={{defineStage1, "Single Antena"},
    {defineStage3, "Interfearing Antenas"},
    {defineStage2, "Simpel Herz"},
    {defineStage4, "Moving Charge"},
    {defineStage5, "Cyclotron"},
    {defineStage6, "circular polarization"}
    };
    std::unique_ptr<Stage> stage = createStage(state.stages[state.currect_stage].defineStage);
    stage->activate(window,state);
    while (window.isOpen()) {
        // Рахуємо FPS
        elapsed = clock.restart();
        fps = static_cast<int>(1.0f / elapsed.asSeconds());

        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::K) {
                    state.show_context_menu = !state.show_context_menu;
                }
                if (event.key.code == sf::Keyboard::P) {
                    state.paused = !state.paused;
                }
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
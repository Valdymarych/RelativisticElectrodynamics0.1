#include "greatOpenGLHandlers.h"
#include <GL/glew.h>
#include <string>
#include <iostream>
#include <string>
#include <vector>
#include "greatShadersExtractor.h"
#include "greatLogger.h"
#include "greatCommon.h"
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

            const std::string source = extractShader(path);
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

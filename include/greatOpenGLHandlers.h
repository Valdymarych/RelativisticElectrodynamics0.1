#pragma once
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

        void checkErrorsShaders(unsigned int shader, std::vector<std::string> paths) {
            int success;
            char infoLog[1024];
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cerr << "ERROR: Помилка компіляції шейдера: ";
                for (int i=0;i<paths.size();i++){
                    std::cout << paths[i] << " ";
                }
                std::cout << "\n" << infoLog << std::endl;
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

        unsigned int get_shader(const std::vector<std::string>& paths, GLenum type){
            std::string version_line = "#version 430 core\n";

            std::vector<std::string> sources_string;
            std::vector<const char*> sources_char;
            sources_string.reserve(paths.size()+1);
            sources_char.reserve(paths.size()+1);
            sources_string.push_back(version_line);
            sources_char.push_back(sources_string[0].c_str());
            for (int i=0; i<paths.size();i++){
                sources_string.push_back(extractShader(paths[i]));
                sources_char.push_back(sources_string[i+1].c_str());
            }
            unsigned int shader = glCreateShader(type);
            glShaderSource(shader, sources_char.size(), sources_char.data(), NULL);
            glCompileShader(shader);
            checkErrorsShaders(shader, paths);
            return shader;
        }

    public:
        ProgramHandler(){

        }
        ProgramHandler(std::vector<std::vector<std::string>>& paths){   //  [ "vertex.glsl" , "geometry.glsl" , "fragment.glsl" , "compute.glsl" ]
            init(paths);
        }
        void init(const std::vector<std::vector<std::string>>& paths){
            unsigned int vertShader;
            unsigned int geomShader;
            unsigned int fragShader;
            ID = glCreateProgram();
            if (paths[0].size()>0){vertShader = get_shader(paths[0],GL_VERTEX_SHADER);glAttachShader(ID, vertShader);}
            if (paths[1].size()>0){geomShader = get_shader(paths[1],GL_GEOMETRY_SHADER);glAttachShader(ID, geomShader);}
            if (paths[2].size()>0){fragShader = get_shader(paths[2],GL_FRAGMENT_SHADER);glAttachShader(ID, fragShader);}
            glLinkProgram(ID);
            checkErrorsProgram(ID, "render");
            if (paths[0].size()>0){glDeleteShader(vertShader);}
            if (paths[1].size()>0){glDeleteShader(geomShader);}
            if (paths[2].size()>0){glDeleteShader(fragShader);}
            
            if(paths[3].size()>0){
                hasComputeShader=true;
                unsigned int compShader = get_shader(paths[3],GL_COMPUTE_SHADER);
                compute_ID = glCreateProgram();
                glAttachShader(compute_ID, compShader);
                glLinkProgram(compute_ID);
                checkErrorsProgram(compute_ID,"compute");
                glDeleteShader(compShader);
            } else {hasComputeShader=false;}
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
            // data_size:   in bytes
            // usage:  GL_STATIC_READ,  GL_DYNAMIC_DRAW... and so on
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
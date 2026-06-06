#pragma once
#include "greatCommon.h"
#include "greatOpenGLHandlers.h"
#include <GL/glew.h>
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

            program.init(
                {
                    {"shaders/common.glsl","shaders/uniform.glsl",path_+"/vertex.glsl"},
                    {},
                    {path_+"/fragment.glsl"},
                    {"shaders/common.glsl","shaders/uniform.glsl","shaders/get_field.glsl",path_+"/compute.glsl"}
                }
            );
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

            std::vector<float> debugger_data(1000,0.);
            ssbo_debugger.read(debugger_data.data(),debugger_data.size()*sizeof(float),0);
            Logger::flog("debugger: ",debugger_data[0], ", ",debugger_data[1], ", ",debugger_data[2],", ",debugger_data[3],",  ",debugger_data[4],", ",debugger_data[5], ", ",debugger_data[6]);
        }

        void update(){
            program.compute((charges_amount+128-1)/128,1,1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        }
};
 
class MyArrows{
    private:
        ProgramHandler program;   // має бути програма
        VBOHandler vbo_arrow_points;  // має бути VBO для вершин стрілки
        VAOHandler vao;           // має бути VAO для glDrawArraysInstanced 
        int pivot_points_amount;        // має бути для ssbo і glDrawArraysInstanced
        SSBOHandler ssbo;         // має бути для FieldDataBuffer в compute shader (index = 1) 
        SSBOHandler ssbo_pivot_points;
        
        const int FieldDataSize_inBytes = 4*12; // 4 байти по 4 на кожний з 3-ох типів поля.
        float arrow_points[18] = {
            0.,0.,0.,    0.,0.1,0.8,   0.,-0.1,0.8, 
            0.,0.3,0.8,  0.,0.,1.,     0.,-0.3,0.8
        };                         // стрілка, по дефолту дивиться в напрямку +Z, так що ребро йде вздовд +Y. З довжиною 1., і шириною 0.6 (від -0.3 до 0.3)
        const int amount_of_arrow_points = 2*3;
        
        static std::vector<Vec4> getPivotPoints(Vec3 start, Vec3 size, uint grid_size_x, uint grid_size_y, uint grid_size_z){
            std::vector<Vec4> pivot_points;
            for (int x=0; x<grid_size_x;x++){
                for (int y=0; y<grid_size_y;y++){
                    for (int z=0; z<grid_size_z;z++){
                        pivot_points.push_back({
                            (float)start.x+(float)size.x*(x+1)/(grid_size_x+1.),
                            (float)start.y+(float)size.y*(y+1)/(grid_size_y+1.),
                            (float)start.z+(float)size.z*(z+1)/(grid_size_z+1.),
                            1
                        });
                    }
                }
            }
            return pivot_points;
        }


    public:
        MyArrows(){}
        void init(u_int grid_size_x_, u_int grid_size_y_, u_int grid_size_z_, std::string path){
            pivot_points_amount = grid_size_x_ * grid_size_y_ * grid_size_z_;
            
            program.init(
                {
                    {"shaders/common.glsl","shaders/uniform.glsl",path+"/vertex.glsl"},
                    {},
                    {path+"/fragment.glsl"},
                    {"shaders/common.glsl","shaders/uniform.glsl","shaders/get_field.glsl",path+"/compute.glsl"}
                }
            );
            vbo_arrow_points.init(arrow_points,sizeof(arrow_points),GL_STATIC_DRAW);
            vbo_arrow_points.configure(0,3,0,0,0);
            vao.bind();
            vbo_arrow_points.bind_configuration();
            VAOHandler::unbind();
            ssbo.init(NULL,pivot_points_amount*FieldDataSize_inBytes,1,GL_DYNAMIC_COPY);
            std::vector<Vec4> pivot_points_data = getPivotPoints({-1,-1,-1}, {2,2,2}, grid_size_x_,grid_size_y_,grid_size_z_);
            std::cout<<"compare  "<<pivot_points_data[511].x<<" "<<pivot_points_data[511].y<<" "<<pivot_points_data[511].z<<std::endl;
            std::cout<<"compare  "<<pivot_points_data[512].x<<" "<<pivot_points_data[512].y<<" "<<pivot_points_data[512].z<<std::endl;
            std::cout<<"compare  "<<pivot_points_data[513].x<<" "<<pivot_points_data[513].y<<" "<<pivot_points_data[513].z<<std::endl;
            ssbo_pivot_points.init((float*)pivot_points_data.data(),pivot_points_data.size()*16,5,GL_STATIC_DRAW);
        }
        void init(float* pivot_points, u_int _points_amount, std::string path){
            pivot_points_amount = _points_amount;
            
            program.init(
                {
                    {path+"/vertex.glsl"},
                    {},
                    {path+"/fragment.glsl"},
                    {path+"/compute.glsl"}
                }
            );
            vbo_arrow_points.init(arrow_points,sizeof(arrow_points),GL_STATIC_DRAW);
            vbo_arrow_points.configure(0,3,0,0,0);
            vao.bind();
            vbo_arrow_points.bind_configuration();
            VAOHandler::unbind();
            ssbo.init(NULL,pivot_points_amount*FieldDataSize_inBytes,1,GL_DYNAMIC_COPY);
            ssbo_pivot_points.init(pivot_points,_points_amount*sizeof(Vec4),5,GL_STATIC_DRAW);
        }
        MyArrows (u_int grid_size_x, u_int grid_size_y, u_int grid_size_z, std::string path)
        {
            init(grid_size_x, grid_size_y, grid_size_z, path);
        }

        MyArrows (float* pivot_points, u_int points_amount, std::string path)
        {
            init(pivot_points, points_amount, path);
        }

        ~MyArrows (){

        }
        void draw(int blendMode){
            glEnable(GL_BLEND);
            if (blendMode==1){glBlendFunc(GL_SRC_ALPHA, GL_ONE);}
            if (blendMode==0){glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);}
            glDepthMask(GL_FALSE);


            program.compute((pivot_points_amount+256-1)/256,1,1);  // розрахунок полів і запис в ssbo
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            vao.bind();
            program.use();


            glDrawArraysInstanced(GL_TRIANGLES, 0, amount_of_arrow_points, pivot_points_amount*3); // малювання точок починаючи з 0, кожна по amount_of_arrow_points точок, і кожна точка це наспрвді 3 поля


            VAOHandler::unbind();
            ProgramHandler::stop();
        }

};

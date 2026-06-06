#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <string>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <iostream>
#include <functional>
#include "greatLogger.h"
#include "greatCommon.h"
#include "greatMyObjectsPool.h"
#include "greatOpenGLHandlers.h"

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
    //window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);

    ImGui::CreateContext();
    if(!ImGui::SFML::Init(window, true)){
        return -1;
    }
    ImGuiIO& io = ImGui::GetIO();
    ImFont* myFont = io.Fonts->AddFontFromFileTTF("DejaVuSans.ttf", 21.0f,nullptr, io.Fonts->GetGlyphRangesDefault());
    io.Fonts->Build();
    
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

void drawImGui(sf::RenderWindow& window, sf::Time& elapsed, State& state){
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SFML::Update(window, elapsed);
    //ImGui::ShowDemoWindow();
    ImGui::Begin("Control Panel     (1234567890.-ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz)",nullptr);
    ImGui::GetStyle().ItemSpacing.y = 10.0f; 
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
    ImGui::Text("FPS: %0.1f", (float)state.fps);
    if (ImGui::Button(state.paused? "Run": "Stop")) {state.paused=!state.paused;}

    ImGui::TextUnformatted("Point of View: ");
    ImGui::TextUnformatted("  ");
    ImGui::SameLine(50.0);
    ImGui::DragFloat("##qa", &state.angle_of_vertical,0.3f, -90.f, 90.f);
    ImGui::TextUnformatted("  ");
    ImGui::SameLine(50.0f);
    ImGui::DragFloat("##bs", &state.angle_of_horizontal,0.3f, 0.f, 360.f);
    ImGui::TextUnformatted("Zoom: ");
    ImGui::SameLine(100.0f);
    ImGui::DragFloat("##bsf", &state.amplifying_factor,0.001f, 0.5f, 3.f);
    ImGui::Dummy(ImVec2(0,10)); 


    ImGui::TextUnformatted("Arrows'");
    ImGui::TextUnformatted("    Size");
    ImGui::SameLine(200.0f);
    ImGui::SliderFloat("##cb_size1", &state.arrow_size,0.01f, 0.3f);
    
    ImGui::TextUnformatted("    Opacity");
    ImGui::SameLine(200.0f);
    ImGui::SliderFloat("##cb_size2", &state.arrow_transparency_factor,0.05f, 1.f);
    
    ImGui::Dummy(ImVec2(0,10));


    ImGui::TextUnformatted("Common Factor");
    ImGui::TextUnformatted("  ");
    ImGui::SameLine(50.0f);
    ImGui::SliderFloat("##cbjhub_0", &state.log_factor_common,-2.f, 2.f);


    ImGui::TextUnformatted("Electric Field");
    ImGui::SameLine(0,20.0f);
    ImGui::Checkbox("##cb_2", &state.show_E);
    ImGui::TextUnformatted("  ");
    ImGui::SameLine(50.0f);
    ImGui::SliderFloat("##cb_0", &state.log_factor_E,-2.0f, 2.f);


    ImGui::TextUnformatted("Magnetic Field");
    ImGui::SameLine(0,20.0f);
    ImGui::Checkbox("##cb_3", &state.show_B);
    ImGui::TextUnformatted("  ");
    ImGui::SameLine(50.0f);
    ImGui::SliderFloat("##cb_1", &state.log_factor_B,-2.0f, 2.f);

    ImGui::TextUnformatted("Energy Flow");
    ImGui::SameLine(0,20.0f);
    ImGui::Checkbox("##cb_4", &state.show_P);
    ImGui::TextUnformatted("  ");
    ImGui::SameLine(50.0f);
    ImGui::SliderFloat("##cb_6", &state.log_factor_P,-2.0f, 2.f);

    ImGui::Dummy(ImVec2(0,10)); 
    //ImGui::SliderFloat("Amplifying Factor", &state.amplifying_factor,0.8f, 5.f);
    ImGui::TextUnformatted("Transparency rendering");
    ImGui::TextUnformatted("  ");
    ImGui::SameLine(50.0f);
    ImGui::SliderInt("##casdbjhub_0", &state.transparency_rendering_style,0,1);
    ImGui::End();
}

void applyStateToUbo(State& state, UBOHandler& ubo){
    replaceBit(ubo.uniforms.display_mode,0,state.show_E);
    replaceBit(ubo.uniforms.display_mode,1,state.show_B);
    replaceBit(ubo.uniforms.display_mode,2,state.show_P);
    replaceBit(ubo.uniforms.display_mode,3,state.show_spheres);

    ubo.uniforms.arrow_size = state.arrow_size;
    ubo.uniforms.arrow_transparency_factor = state.arrow_transparency_factor;

    ubo.uniforms.view = Matrix::getViewMatrix({0.,0.,0.}, 2./state.amplifying_factor, state.angle_of_vertical/180.*PI, state.angle_of_horizontal/180.*PI);
    ubo.uniforms.projection = Matrix::getProjectionMatrix(45.f/180.*PI, (float)state.display_ratio, 0.1f, 5.f);
    
    ubo.uniforms.cameraPos = Matrix::getCameraPos({0.,0.,0.}, 2./state.amplifying_factor, state.angle_of_vertical/180.*PI, state.angle_of_horizontal/180.*PI);

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
    ubo.uniforms.magnetic_permeability_inv = state.magnetic_permeability_inv;
    ubo.uniforms.history_size_log = state.history_size_log;
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
            //window.pushGLStates();
                        Logger::begin("Activating the stage... ");
                        Logger::log("defining stage");
            defineStage(charges_vector,state);
            state.time=0;
            state.buffer_offset=-1;
            state.amount_of_charges = charges_vector.size();
            state.history_size_log = int(log2(state.history_size));
            state.magnetic_permeability_inv = (state.c*state.c)/(12.56*state.k);
                        Logger::begin("initializing charges");
            charges.init(state.charge_detailing,state.history_size,"shaders/spheres_shader",charges_vector);
                        Logger::end("Charges is initialized");
                        Logger::begin("initializing arrows");
            arrows.init(state.grid_size_x, state.grid_size_y, state.grid_size_z, "shaders/arrows_shader");
                        Logger::end("arrows are ready");
                        Logger::end("Stage is activated");
            //window.popGLStates();
        }
        void draw(sf::RenderWindow& window, sf::Time& elapsed, UBOHandler& ubo, State& state){

                        Logger::off();
                        Logger::begin("trying to draw");
            if (state.show_context_menu){
                Logger::log("drawing ImGui");
                drawImGui(window, elapsed, state);
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
            window.display();
        }
        void handleEvents(State& state, sf::RenderWindow& window){
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
        }
        ~Stage(){
                        Logger::log("Stage is being destructed");
        }
};

std::unique_ptr<Stage> createStage(std::function<void(std::vector<MyCharge>&, State&)> defineStage) {
    return std::make_unique<Stage>(defineStage);
}

void handleReplacingOfStage(State&state, std::unique_ptr<Stage>& stage, sf::RenderWindow& window){
    if(state.intended_to_be_selected_stage>-1){
        state.currect_stage=state.intended_to_be_selected_stage;
        state.intended_to_be_selected_stage=-1;
        stage = createStage(state.stages[state.currect_stage].defineStage);
        stage->activate(window,state);
    }
}
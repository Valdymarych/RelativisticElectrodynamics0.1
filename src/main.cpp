#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <string>
#include "greatShadersExtractor.h"
#include "greatLogger.h"
#include "greatOpenGLHandlers.h"
#include "greatCommon.h"
#include "greatMyObjectsPool.h"
#include "greatStageClass.h"
#include "greatStagesPool.h"

int main() {
    sf::RenderWindow window;
    sf::Clock clock;
    sf::Time elapsed;
    State state;
    if (initEverything(window,state.display_x,state.display_y)){return -1;}
    Uniforms uniforms_init;
    UBOHandler ubo(uniforms_init);


    state.stages={
        {defineStage1, "single antena"},
        {defineStage2, "interfearing antenas"},
        {defineStage3, "dynamic charges"},
        {defineStage4, "moving charge"},
        {defineStage5, "cyclotron"},
        //{defineStage6, "circular polarization"},
        {defineStage7, "electron-positron"},
        {defineStage8, "atom"},
        {defineStage9, "dipole"}
    };
    std::unique_ptr<Stage> stage = createStage(state.stages[state.currect_stage].defineStage);
    stage->activate(window,state);
    while (window.isOpen()) {
        elapsed = clock.restart();
        state.fps = static_cast<int>(1.0f / elapsed.asSeconds());

        stage->handleEvents(state,window);
        stage->draw(window,elapsed,ubo,state);
        handleReplacingOfStage(state,stage,window);
    }
    return 0;
} 
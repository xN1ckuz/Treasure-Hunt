#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "shader_m.h"
#include "camera.h"
#include "model.h"

#include "DrawableObj.h"
#include "Terrain.h"
#include "ShadowBox.h"
#include "DrawableObjIstanced.h"
#include "effects.h"
#include "Coperchi.h"
#include "renderText.h"
#include "SFML/Audio.hpp"

#include <iostream>

bool areModelsLoaded = false;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, Coperchi* coperchiCasse, SmokeHendler* smokeHendler, float currentFrame);
unsigned int loadTexture(const char* path);
void renderScene(DrawableObjIstanced alberi1, DrawableObjIstanced alberi2, DrawableObj terreno, DrawableObj erba, DrawableObjIstanced basiCasse, Coperchi coperchiCasse, DrawableObj cubo, SmokeHendler* smokeHendler, float currentFrame, bool ombra);
void renderQuad();
void calculateFPS();
unsigned int loadCubemap(vector<std::string> faces);
void renderLoading(Shader* shader, DrawableObj cubo, Camera cam, GLFWwindow* window);

float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

// settings
//const unsigned int SCR_WIDTH = 1366;
//const unsigned int SCR_HEIGHT = 768;
const unsigned int SCR_WIDTH = 640;
const unsigned int SCR_HEIGHT = 360;
//const unsigned int SCR_WIDTH = 2560;
//const unsigned int SCR_HEIGHT = 1440;

// camera
Camera camera;
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;
float NEAR_PLANE = 0.1;
float FAR_PLANE = 100;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//FPS
unsigned int frameCount = 0;
double previousTime = 0;
double timeInterval = 0;
unsigned int fps = 0;

// lighting info
// -------------
glm::vec3 lightPos(-10.0f, 80.0f, -30.0f);

// Shadow
// -------------
float nearDist = 0.001f;
float farDist = 80.0f;
ShadowBox shadowBox = ShadowBox(nearDist, farDist, SCR_WIDTH, SCR_HEIGHT, &camera, lightPos);

// Tempo di gioco
// -------------
float tempoMassimo = 5 * (60);

int main()
{   
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 16);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    string TITOLO_APP = "Treasure-Hunt";
    //GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    //const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, TITOLO_APP.data(), NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Sound Settings
    sf::Music backgroundAudio;

    backgroundAudio.openFromFile("resources/audio/background.wav");

    //Background Audio Settings
    sf::Listener::setPosition(0.0f, 0.0f, 0.0f);
    backgroundAudio.setPosition(0.0f, 0.0f, 0.0f);
    backgroundAudio.setPitch(1.0f);
    backgroundAudio.setVolume(4.0f);
    backgroundAudio.setMinDistance(5.0);
    backgroundAudio.setLoop(true);
    

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader loadingScreen("loading_screen.vs", "loading_screen.fs");
    Shader simpleDepthShader("shadow_mapping_depth.vs", "shadow_mapping_depth.fs");
    Shader simpleDepthShaderInstanced("shadow_mapping_depth_instanced.vs", "shadow_mapping_depth.fs");
    Shader simpleDepthShaderSmoke("shadow_mapping_depth.vs", "shadow_mapping_depth_smoke.fs");
    Shader debugDepthQuad("debug_quad.vs", "debug_quad_depth.fs");
    Shader shaderWithoutAlpha("model_loading.vs", "model_loading.fs");
    Shader shaderWithAlpha("model_loading.vs", "model_loading_alpha.fs");
    Shader shaderWithAlphaInstanced("model_loading_instanced.vs", "model_loading_alpha.fs");
    Shader shaderWithoutAlphaInstanced("model_loading_instanced.vs", "model_loading.fs");
    Shader skyboxShader("skybox.vs", "skybox.fs");
    Shader smokeShader("smokeShader.vs", "smokeShader.fs");

    //load drowables
    //----------------
    Terrain* terreno;
    DrawableObj erba;
    Coperchi coperchiCasse = Coperchi("chests.txt", "resources/models/chest1.obj");
    DrawableObjIstanced basiCasse = DrawableObjIstanced("chests.txt", "resources/models/chest2.obj");
    DrawableObjIstanced alberi1 = DrawableObjIstanced("redwood_01.txt", "resources/models/redwood_01.obj");
    DrawableObjIstanced alberi2 = DrawableObjIstanced("redwood_02.txt", "resources/models/redwood_02.obj");
    SmokeHendler smokeHendler = SmokeHendler("resources/cloud/cloud.obj", 100, 0.5, 10);
    DrawableObj cubo = DrawableObj("resources/loading/loading_screen.obj");
    glm::vec3 posVecchia;

    initRenderText(SCR_WIDTH, SCR_HEIGHT);

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // load textures
    // -------------
    vector<std::string> faces
    {
        "resources/skybox/right.jpg",
        "resources/skybox/left.jpg",
        "resources/skybox/top.jpg",
        "resources/skybox/bottom.jpg",
        "resources/skybox/front.jpg",
        "resources/skybox/back.jpg"
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    // configure depth map FBO
    // -----------------------
    //const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    //const unsigned int SHADOW_WIDTH = 8192, SHADOW_HEIGHT = 8192;
    const unsigned int SHADOW_WIDTH = 16384, SHADOW_HEIGHT = 16384;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // shader configuration
    // --------------------
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    shaderWithoutAlpha.use();
    shaderWithoutAlpha.setInt("shadowMap", 4);

    shaderWithoutAlphaInstanced.use();
    shaderWithoutAlphaInstanced.setInt("shadowMap", 4);

    shaderWithAlpha.use();
    shaderWithAlpha.setInt("shadowMap", 4);

    shaderWithAlphaInstanced.use();
    shaderWithAlphaInstanced.setInt("shadowMap", 4);

    smokeShader.use();
    smokeShader.setInt("shadowMap", 4);

    debugDepthQuad.use();
    debugDepthQuad.setInt("depthMap", 4);

    // render loop
    // -----------
    int frame = 0;
    float tempoInizioGioco = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        if (frame == 0) {
            renderLoading(&loadingScreen, cubo, camera, window);     
        } else {
            if (!areModelsLoaded) {
                terreno = new Terrain("resources/models/world.obj", "resources/models/textures/terrain.jpg");
                erba = DrawableObj("resources/models/grass.obj");
                camera = Camera(terreno->updateCameraPositionOnMap(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 2, true));
                posVecchia = camera.Position;
                backgroundAudio.play();
                areModelsLoaded = true;
            }
            //render as always
            // per-frame time logic
            // --------------------
            float currentFrame = glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            // input
            // -----
            posVecchia = camera.Position;
            processInput(window, &coperchiCasse, &smokeHendler, currentFrame);
            camera.Position = terreno->updateCameraPositionOnMap(camera.Position, posVecchia, 2, false);
            basiCasse.aggiornaPosPerCollisione(&camera.Position, posVecchia, 3.0);
            alberi1.aggiornaPosPerCollisione(&camera.Position, posVecchia, 3.0);
            alberi2.aggiornaPosPerCollisione(&camera.Position, posVecchia, 2.5);
            int cassaNear = coperchiCasse.getCoperchioToOpen(camera.Position, 5);
            //cout << "Camera: "<< "Pos x: " << camera.Position.x << "Pos y: " << camera.Position.y << "Pos z: " << camera.Position.z << endl;
            //cout << "Luce: "<< "Pos x: " << lightPos.x << "Pos y: " << lightPos.y << "Pos z: " << lightPos.z << endl;

            // render
            // ------
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // 1. render scene from light's point of view
            // --------------------------------------------------------------
            glm::mat4 lightSpaceMatrix = shadowBox.getlightSpaceMatrix();

            simpleDepthShader.use();
            simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

            simpleDepthShaderInstanced.use();
            simpleDepthShaderInstanced.setMat4("lightSpaceMatrix", lightSpaceMatrix);

            simpleDepthShaderSmoke.use();
            simpleDepthShaderSmoke.setMat4("lightSpaceMatrix", lightSpaceMatrix);

            terreno->setShaders(&simpleDepthShader);
            erba.setShaders(&simpleDepthShader);
            alberi1.setShaders(&simpleDepthShaderInstanced);
            alberi2.setShaders(&simpleDepthShaderInstanced);
            basiCasse.setShaders(&simpleDepthShaderInstanced);
            coperchiCasse.setShaders(&simpleDepthShaderInstanced);
            smokeHendler.setShaders(&simpleDepthShaderSmoke);

            cubo.setShaders(&simpleDepthShader);

            glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
            glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
            glClear(GL_DEPTH_BUFFER_BIT);

            renderScene(alberi1, alberi2, *terreno, erba, basiCasse, coperchiCasse, cubo, &smokeHendler, currentFrame, true);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // reset viewport
            glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // 2a. draw first skybox with depth write disabled
            // --------------------------------------------------------------
            skyboxShader.use();
            glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 15.0f);
            skyboxShader.setMat4("view", view);
            skyboxShader.setMat4("projection", projection);
            // skybox cube
            glBindVertexArray(skyboxVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
            glDepthMask(GL_FALSE);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glDepthMask(GL_TRUE);
            glBindVertexArray(0);

            // 2b. render scene as normal using the generated depth/shadow map  
            // --------------------------------------------------------------
            projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            view = camera.GetViewMatrix();

            shaderWithoutAlpha.use();
            shaderWithoutAlpha.setMat4("view", view);
            shaderWithoutAlpha.setMat4("projection", projection);
            shaderWithoutAlpha.setVec3("viewPos", camera.Position);
            shaderWithoutAlpha.setVec3("lightPos", lightPos);
            shaderWithoutAlpha.setMat4("lightSpaceMatrix", lightSpaceMatrix);

            shaderWithoutAlphaInstanced.use();
            shaderWithoutAlphaInstanced.setMat4("view", view);
            shaderWithoutAlphaInstanced.setMat4("projection", projection);
            shaderWithoutAlphaInstanced.setVec3("viewPos", camera.Position);
            shaderWithoutAlphaInstanced.setVec3("lightPos", lightPos);
            shaderWithoutAlphaInstanced.setMat4("lightSpaceMatrix", lightSpaceMatrix);

            shaderWithAlpha.use();
            shaderWithAlpha.setMat4("view", view);
            shaderWithAlpha.setMat4("projection", projection);
            shaderWithAlpha.setVec3("viewPos", camera.Position);
            shaderWithAlpha.setVec3("lightPos", lightPos);
            shaderWithAlpha.setMat4("lightSpaceMatrix", lightSpaceMatrix);

            shaderWithAlphaInstanced.use();
            shaderWithAlphaInstanced.setMat4("view", view);
            shaderWithAlphaInstanced.setMat4("projection", projection);
            shaderWithAlphaInstanced.setVec3("viewPos", camera.Position);
            shaderWithAlphaInstanced.setVec3("lightPos", lightPos);
            shaderWithAlphaInstanced.setMat4("lightSpaceMatrix", lightSpaceMatrix);

            smokeShader.use();
            smokeShader.setMat4("view", view);
            smokeShader.setMat4("projection", projection);
            smokeShader.setVec3("viewPos", camera.Position);
            smokeShader.setVec3("lightPos", lightPos);
            smokeShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
            smokeShader.setInt("texture_diffuse1", 0);

            terreno->setShaders(&shaderWithoutAlpha);
            erba.setShaders(&shaderWithAlpha);
            alberi1.setShaders(&shaderWithAlphaInstanced);
            alberi2.setShaders(&shaderWithAlphaInstanced);
            basiCasse.setShaders(&shaderWithoutAlphaInstanced);
            coperchiCasse.setShaders(&shaderWithoutAlphaInstanced);
            smokeHendler.setShaders(&smokeShader);

            cubo.setShaders(&shaderWithoutAlpha);

            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, depthMap);
            renderScene(alberi1, alberi2, *terreno, erba, basiCasse, coperchiCasse, cubo, &smokeHendler, currentFrame, true);
            RenderText(("FPS: " + std::to_string(fps)).c_str(), 10, SCR_HEIGHT - 20, 0.3f, glm::vec3(1.0f, 1.0f, 1.0f));
            float tempoRimasto = tempoMassimo - (currentFrame - tempoInizioGioco);
            string tempo;
            if (int(tempoRimasto) % 60 < 10) {
                tempo = "Tempo: " + std::to_string(int(tempoRimasto) / 60) + "min e " + "0" + std::to_string(int(tempoRimasto) % 60) + "sec";
            }
            else {
                tempo = "Tempo: " + std::to_string(int(tempoRimasto) / 60) + "min e " + std::to_string(int(tempoRimasto) % 60) + "sec";
            }
            RenderText((tempo).c_str(), (SCR_WIDTH - getWidthOfString(tempo)) / 2, SCR_HEIGHT - getHeightOfString(tempo), 0.6f, glm::vec3(1.0f, 0.0f, 0.0f));
            RenderText(("Casse Rimanenti: " + std::to_string(coperchiCasse.contaCasse())).c_str(), 10, 10, 0.6f, glm::vec3(1.0f, 1.0f, 1.0f));
            if (cassaNear != -1) {
                RenderText("SPACE per aprire", (SCR_WIDTH - getWidthOfString("SPACE per aprire")) / 2, (SCR_HEIGHT - getHeightOfString("SPACE per aprire")) / 2, 0.6f, glm::vec3(1.0f, 1.0f, 1.0f));
            }

            // render Depth map to quad for visual debugging
            // ---------------------------------------------
            //debugDepthQuad.use();
            //debugDepthQuad.setFloat("near_plane", NEAR_PLANE);
            //debugDepthQuad.setFloat("far_plane", FAR_PLANE);
            //glActiveTexture(GL_TEXTURE4);
            //glBindTexture(GL_TEXTURE_2D, depthMap);
            //renderQuad();

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
        }
        frame = 1;
        glfwSwapBuffers(window);
        glfwPollEvents();
        calculateFPS();
        std::string titolo_str =  TITOLO_APP + " FPS: " + std::to_string(fps);
        glfwSetWindowTitle(window, titolo_str.c_str());
        glfwSwapInterval(0);
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVAO);
    glfwTerminate();
    return 0;
}

// renders the 3D scene
// --------------------
void renderScene(DrawableObjIstanced alberi1, DrawableObjIstanced alberi2, DrawableObj terreno, DrawableObj erba, DrawableObjIstanced basiCasse, Coperchi coperchiCasse, DrawableObj cubo, SmokeHendler* smokeHendler, float currentFrame, bool ombra)
{   
    //floor
    terreno.traslate(glm::vec3(0.0f, -0.2f, 0.0f));
    terreno.scale(glm::vec3(1.0f, 1.0f, 1.0f));
    terreno.Draw();

    //Casse
    if (ombra) {
        basiCasse.getShader()->use();
        basiCasse.getShader()->setFloat("soglia", 0.5);
    }
    basiCasse.Draw();

    if (ombra) {
        coperchiCasse.getShader()->use();
        coperchiCasse.getShader()->setFloat("soglia", 0.5);
    }
    coperchiCasse.Draw();

    //Per gli oggetti trasparenti
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //Alberi
    if (ombra) {
        alberi1.getShader()->use();
        alberi1.getShader()->setFloat("soglia", 0.5);
    }
    alberi1.Draw();

    if (ombra) {
        alberi2.getShader()->use();
        alberi2.getShader()->setFloat("soglia", 0.5);
    }
    alberi2.Draw();

    //erba
    if (ombra) {
        erba.getShader()->use();
        erba.getShader()->setFloat("soglia", 0.0001);
    }
    erba.traslate(glm::vec3(0.0f, -0.2f, 0.0f));
    erba.scale(glm::vec3(1.0f, 1.0f, 1.0f));
    erba.Draw();

    //fumo
    if (ombra) {
        smokeHendler->getShader()->use();
        smokeHendler->getShader()->setFloat("soglia", 0.1);
    }
    smokeHendler->Draw(currentFrame);
    glDisable(GL_BLEND);
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window, Coperchi* coperchiCasse, SmokeHendler* smokeHendler, float currentFrame)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        int cassaDaAprire = coperchiCasse->getCoperchioToOpen(camera.Position, 5);
        coperchiCasse->apriCassa(cassaDaAprire, 70, smokeHendler, currentFrame);
    }
       
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void calculateFPS() {
    //  Incrementiamo il contatore
    frameCount++;
    //  Determiniamo il numero di millisecondi trascorsi dalla glutInit
    double currentTime = glfwGetTime();
    //  Calcoliamo il tempo trascorso
    timeInterval = currentTime - previousTime;

    // Se è passato un secondo aggiorna la variabile fps
    if (timeInterval > 1.0f) {
        //  frameCount mantiene il numero di frame generati in un secondo
        fps = frameCount;

        //  Salviamo il tempo trascorso per riutilizzarlo la prossima volta
        previousTime = currentTime;

        //  Azzeriamo il contatore dei tempi
        frameCount = 0;
    }
}

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void renderLoading(Shader* shader, DrawableObj cubo, Camera cam, GLFWwindow* window) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

    glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 15.0f);
    shader->use();
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);

    cubo.setShaders(shader);
    cubo.Draw();

    glfwSwapBuffers(window);
    glfwPollEvents();
}
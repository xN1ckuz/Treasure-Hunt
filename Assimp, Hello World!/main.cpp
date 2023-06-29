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

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
void renderScene(DrawableObjIstanced alberi1, DrawableObjIstanced alberi2, DrawableObj terreno, DrawableObj erba, bool ombra);
void renderQuad();
void calculateFPS();
unsigned int loadCubemap(vector<std::string> faces);

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
//const unsigned int SCR_WIDTH = 1920;
//const unsigned int SCR_HEIGHT = 1080;
const unsigned int SCR_WIDTH = 1366;
const unsigned int SCR_HEIGHT = 768;
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
glm::vec3 lightPos(-4.0f, 150.0f, 70.0f);

// Shadow
// -------------
float nearDist = 0.001f;
float farDist = 80.0f;
ShadowBox shadowBox = ShadowBox(nearDist, farDist, SCR_WIDTH, SCR_HEIGHT, &camera, lightPos);

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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // build and compile shaders
    // -------------------------
    Shader simpleDepthShader("shadow_mapping_depth.vs", "shadow_mapping_depth.fs");
    Shader simpleDepthShaderInstanced("shadow_mapping_depth_instanced.vs", "shadow_mapping_depth.fs");
    Shader debugDepthQuad("debug_quad.vs", "debug_quad_depth.fs");
    Shader shaderWithoutAlpha("model_loading.vs", "model_loading.fs");
    Shader shaderWithAlpha("model_loading.vs", "model_loading_alpha.fs");
    Shader shaderWithAlphaInstanced("model_loading_instanced.vs", "model_loading_alpha.fs");
    Shader skyboxShader("skybox.vs", "skybox.fs");


    //load drowables
    //----------------
    DrawableObj erba = DrawableObj("resources/models/grass.obj");
    Terrain terreno = Terrain("resources/models/world.obj","resources/models/textures/terrain.jpg");
    //DrawableObj albero = DrawableObj("resources/models/redwood_01.obj");
    DrawableObjIstanced alberi1 = DrawableObjIstanced("redwood_01.txt", "resources/models/redwood_01.obj");
    DrawableObjIstanced alberi2 = DrawableObjIstanced("redwood_02.txt", "resources/models/redwood_02.obj");

    //Camera con walk
    camera = Camera(&terreno,terreno.updateCameraPositionOnMap(glm::vec3(0.0f, 0.0f, 0.0f),2,true));
    //camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));

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

    shaderWithAlpha.use();
    shaderWithAlpha.setInt("shadowMap", 4);

    shaderWithAlphaInstanced.use();
    shaderWithAlphaInstanced.setInt("shadowMap", 4);

    debugDepthQuad.use();
    debugDepthQuad.setInt("depthMap", 4);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

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

        terreno.setShaders(&simpleDepthShader);
        erba.setShaders(&simpleDepthShader);
        alberi1.setShaders(&simpleDepthShaderInstanced);
        alberi2.setShaders(&simpleDepthShaderInstanced);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        renderScene(alberi1,alberi2, terreno, erba, true);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // reset viewport
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 2a. draw first skybox with depth write disabled
        // --------------------------------------------------------------
        skyboxShader.use();
        glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        //glm::mat4 view = glm::mat4(camera.GetViewMatrix()); // remove translation from the view matrix
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
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

        terreno.setShaders(&shaderWithoutAlpha);
        erba.setShaders(&shaderWithAlpha);
        alberi1.setShaders(&shaderWithAlphaInstanced);
        alberi2.setShaders(&shaderWithAlphaInstanced);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        renderScene(alberi1, alberi2, terreno, erba, true);

        
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
void renderScene(DrawableObjIstanced alberi1, DrawableObjIstanced alberi2, DrawableObj terreno, DrawableObj erba, bool ombra)
{   

    //floor
    terreno.traslate(glm::vec3(0.0f, -0.2f, 0.0f));
    terreno.scale(glm::vec3(1.0f, 1.0f, 1.0f));
    terreno.Draw();

    //erba
    if (ombra) {
        erba.getShader()->use();
        erba.getShader()->setFloat("soglia", 0.0001);
    }
    erba.traslate(glm::vec3(0.0f, -0.2f, 0.0f));
    erba.scale(glm::vec3(1.0f, 1.0f, 1.0f));
    erba.Draw();

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
void processInput(GLFWwindow* window)
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
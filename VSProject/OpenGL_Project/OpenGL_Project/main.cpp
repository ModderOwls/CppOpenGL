#include <iostream>
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "model.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


void processInput(GLFWwindow* window);
int init(GLFWwindow* &window);

void createGeometry(GLuint &vao, unsigned int& ebo, int &size, int& sizeIndices);
void createShaders(); 
void createProgram(GLuint& program, const char* vertex, const char* fragment);

void loadFile(const char* filename, char*& output);
GLuint loadTexture(const char* path, int comp = 0);

void renderSkyBox();
void renderTerrain();
void renderWater();
void renderModel(Model* model, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale);

unsigned int GeneratePlane(const char* heightmap, unsigned char* &data, GLenum format, int comp, float hScale, float xzScale, unsigned int& indicesSize, unsigned int& heightmapID);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

bool keys[1024];

GLuint simpleProgram, skyProgram, terrainProgram, waterProgram, modelProgram;

//Screen data.
const int screenWidth = 1280, screenHeight = 720;

//World data.
glm::vec3 lightDirection = glm::normalize(glm::vec3(-0.5f, -0.5f, -0.5f));
glm::vec3 cameraPosition = glm::vec3(100.0f, 125.5f, 100.0f);

GLuint VAO, EBO;
int boxSize, boxIndicesSize;

glm::mat4 view, projection;

float lastX, lastY;
bool firstMouse = true;
float camYaw, camPitch;
glm::quat camQuat = glm::quat(glm::vec3(glm::radians(camPitch), glm::radians(camYaw), 0));

//Terrain data.
GLuint terrainVAO, terrainIndicesSize, heightmapID, heightNormalID;
unsigned char* heightmapTexture;

GLuint dirt, sand, grass, rock, snow;

GLuint waterVAO, waterIndicesSize, waterHeightmapID, waterNormal, waterFoam;
unsigned char* waterHeightmapTexture;

float timePassed;

Model* backpack;
Model* teapot;
Model* cabinet;
Model* tree;


int main()
{
    GLFWwindow* window;
    int resultInit = init(window);
    if (resultInit != 0) return resultInit;

    //Flip UVs vertically for correct model importing.
    stbi_set_flip_vertically_on_load(true);

    createGeometry(VAO, EBO, boxSize, boxIndicesSize);
    createShaders();

    terrainVAO = GeneratePlane("sprites/heightmap.png", heightmapTexture, GL_RGBA, 4, 80.0f, 2.0f, terrainIndicesSize, heightmapID);
    heightNormalID = loadTexture("sprites/heightmapNormal.png");
    waterVAO = GeneratePlane("sprites/waterPlane.png", waterHeightmapTexture, GL_RGBA, 4, 10.0f, 20.11f, waterIndicesSize, waterHeightmapID);
    waterNormal = loadTexture("sprites/water.png", 4);
    waterFoam = loadTexture("sprites/terrainFoam.jpg");

    //Load and apply textures.
    GLuint texture1 = loadTexture("sprites/container.jpg");
    GLuint texture1Normal = loadTexture("sprites/containerNormal.png");

    dirt = loadTexture("sprites/dirt.jpg");
    sand = loadTexture("sprites/sand.jpg");
    grass = loadTexture("sprites/grass.png", 4);
    rock = loadTexture("sprites/rock.jpg");
    snow = loadTexture("sprites/snow.jpg");

    backpack = new Model("models/backpack/backpack.obj");
    teapot = new Model("models/utahteapot/teapot.obj");
    cabinet = new Model("models/cabinet/cabinet.obj");
    tree = new Model("models/tree/tree.obj");

    //Useful for debugging.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    //Create viewport.
    glViewport(0, 0, screenWidth, screenHeight);


    //Matrices.
    view = glm::lookAt(cameraPosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    projection = glm::perspective(glm::radians(45.0f), screenWidth / (float)screenHeight, 0.1f, 5000.0f);


    //Render loop.
    while (!glfwWindowShouldClose(window))
    {
        timePassed = glfwGetTime();

        //Input
        processInput(window);

        //Rendering
        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderSkyBox();
        renderTerrain();
        renderWater();
        renderModel(teapot, glm::vec3(500, 100 + glm::sin(timePassed * 2) * 10, 500), glm::vec3(0, timePassed * 2.5f, 0), glm::vec3(0.5f, 0.5f, 0.5f));
        renderModel(cabinet, glm::vec3(500, 40, 720), glm::vec3(glm::radians(-4.0f), glm::radians(190.0f), glm::radians(-1.0f)), glm::vec3(80, 80, 80));
        renderModel(tree, glm::vec3(300, 40, 350), glm::vec3(glm::radians(-8.0f), glm::radians(150.0f), glm::radians(-3.0f)), glm::vec3(70, 70, 70));
        renderModel(tree, glm::vec3(700, 20, 750), glm::vec3(glm::radians(6.0f), glm::radians(199.0f), glm::radians(-3.0f)), glm::vec3(20, 20, 20));
        renderModel(tree, glm::vec3(300, 20, 800), glm::vec3(glm::radians(1.0f), glm::radians(165.0f), glm::radians(12.0f)), glm::vec3(25, 25, 25));

        //Polling
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //Close window.
    glfwTerminate();

	return 0;
}

void renderSkyBox()
{
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_DEPTH);

    glUseProgram(skyProgram);

    glm::mat4 world = glm::mat4(1.0f);
    world = glm::translate(world, cameraPosition);
    world = glm::scale(world, glm::vec3(10, 10, 10));

    glUniformMatrix4fv(glGetUniformLocation(skyProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
    glUniformMatrix4fv(glGetUniformLocation(skyProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(skyProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glUniform3fv(glGetUniformLocation(skyProgram, "lightDirection"), 1, glm::value_ptr(lightDirection));
    glUniform3fv(glGetUniformLocation(skyProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

    //Rendering.
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, boxIndicesSize, GL_UNSIGNED_INT, 0);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH);
}

void renderTerrain() 
{
    glEnable(GL_DEPTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glUseProgram(terrainProgram);

    glm::mat4 world = glm::mat4(1.0f);

    glUniformMatrix4fv(glGetUniformLocation(terrainProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
    glUniformMatrix4fv(glGetUniformLocation(terrainProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(terrainProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    lightDirection = glm::normalize(glm::vec3(glm::sin(timePassed), -0.5f, glm::cos(timePassed)));
    glUniform3fv(glGetUniformLocation(terrainProgram, "lightDirection"), 1, glm::value_ptr(lightDirection));
    glUniform3fv(glGetUniformLocation(terrainProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, heightmapID);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, heightNormalID);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, dirt);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, sand);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, grass);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, rock);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, snow);

    //Rendering.
    glBindVertexArray(terrainVAO);
    glDrawElements(GL_TRIANGLES, terrainIndicesSize, GL_UNSIGNED_INT, 0);
}

void renderWater()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glUseProgram(waterProgram);

    glm::mat4 world = glm::mat4(1.0f);
    world = glm::translate(world, glm::vec3(-750, 0, -750));

    glUniformMatrix4fv(glGetUniformLocation(waterProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
    glUniformMatrix4fv(glGetUniformLocation(waterProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(waterProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glUniform1f(glGetUniformLocation(waterProgram, "time"), timePassed);
    glUniform3fv(glGetUniformLocation(waterProgram, "lightDirection"), 1, glm::value_ptr(lightDirection));
    glUniform3fv(glGetUniformLocation(waterProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, waterNormal);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, waterFoam);

    //Rendering.
    glBindVertexArray(waterVAO);
    glDrawElements(GL_TRIANGLES, waterIndicesSize, GL_UNSIGNED_INT, 0);
}

void renderModel(Model* model, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale)
{
    glDisable(GL_BLEND);
    //Alpha blend.
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //Additive blend.
    //glBlendFunc(GL_ONE, GL_ONE);
    //Soft additive blend.
    //glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);
    //Multiply blend.
    //glBlendFunc(GL_DST_COLOR, GL_ZERO);
    //Double multiply blend.
    //glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);

    glEnable(GL_DEPTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glUseProgram(modelProgram);

    glm::mat4 world = glm::mat4(1.0f);
    world = glm::translate(world, pos);
    world = world * glm::toMat4(glm::quat(rot));
    world = glm::scale(world, scale);

    glUniformMatrix4fv(glGetUniformLocation(modelProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
    glUniformMatrix4fv(glGetUniformLocation(modelProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(modelProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glUniform3fv(glGetUniformLocation(modelProgram, "lightDirection"), 1, glm::value_ptr(lightDirection));
    glUniform3fv(glGetUniformLocation(modelProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

    model->Draw(modelProgram);

    glDisable(GL_BLEND);
}

unsigned int GeneratePlane(const char* heightmap, unsigned char* &data, GLenum format, int comp, float hScale, float xzScale, unsigned int& indicesSize, unsigned int& heightmapID) 
{
    int width, height, channels;
    data = nullptr;
    if (heightmap != nullptr) 
    {
        data = stbi_load(heightmap, &width, &height, &channels, comp);
        if (data) {
            glGenTextures(1, &heightmapID);
            glBindTexture(GL_TEXTURE_2D, heightmapID);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else
        {
            std::cout << "ERROR - Loading texture data into generatePlane.\n" << std::endl;

            return -1;
        }
    }

    int stride = 8;
    float* vertices = new float[(width * height) * stride];
    unsigned int* indices = new unsigned int[(width - 1) * (height - 1) * 6];

    int index = 0;
    for (int i = 0; i < (width * height); i++) 
    {
        // TODO: calculate x/z values
        int x = i % width;
        int z = i / width;

        float texHeight = (float)data[i * comp];

        // TODO: set position
        vertices[index++] = x * xzScale;
        vertices[index++] = (texHeight / 255.0f) * hScale;
        vertices[index++] = z * xzScale;

        // TODO: set normal
        vertices[index++] = 0;
        vertices[index++] = 1;
        vertices[index++] = 0;

        // TODO: set uv
        vertices[index++] = x / (float)width;
        vertices[index++] = z / (float)height;
    }

    // OPTIONAL TODO: Calculate normal
    // TODO: Set normal

    index = 0;
    for (int i = 0; i < (width - 1) * (height - 1); i++) 
    {
        // TODO: calculate x/z values
        int x = i % (width - 1);
        int z = i / (width - 1);

        int vertex = z * width + x;

        indices[index++] = vertex;
        indices[index++] = vertex + width;
        indices[index++] = vertex + width + 1;

        indices[index++] = vertex;
        indices[index++] = vertex + width + 1;
        indices[index++] = vertex + 1;
    }

    unsigned int vertSize = (width * height) * stride * sizeof(float);
    indicesSize = ((width - 1) * (height - 1) * 6);

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertSize, vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    // vertex information!
    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * stride, 0);
    glEnableVertexAttribArray(0);
    // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * stride, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);
    // uv
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * stride, (void*)(sizeof(float) * 6));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    delete[] vertices;
    delete[] indices;

    return VAO;
}

int init(GLFWwindow*& window)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //Create window and make active.
    window = glfwCreateWindow(screenWidth, screenHeight, "OpenGL_Proj", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    //Register callbacks.
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);

    //Load GLAD.
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    bool camChanged = false;

    if (keys[GLFW_KEY_W])
    {
        cameraPosition += camQuat * glm::vec3(0, 0, 1);
        camChanged = true;
    }
    if (keys[GLFW_KEY_S])
    {
        cameraPosition += camQuat * glm::vec3(0, 0, -1);
        camChanged = true;
    }
    if (keys[GLFW_KEY_A])
    {
        cameraPosition += camQuat * glm::vec3(1, 0, 0);
        camChanged = true;
    }
    if (keys[GLFW_KEY_D])
    {
        cameraPosition += camQuat * glm::vec3(-1, 0, 0);
        camChanged = true;
    }

    if (camChanged)
    {
        camQuat = glm::quat(glm::vec3(glm::radians(camPitch), glm::radians(camYaw), 0));

        glm::vec3 camForward = camQuat * glm::vec3(0, 0, 1);
        glm::vec3 camUp = camQuat * glm::vec3(0, 1, 0);
        view = glm::lookAt(cameraPosition, cameraPosition + camForward, camUp);
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    float x = (float)xpos;
    float y = (float)ypos;

    if (firstMouse)
    {
        lastX = x;
        lastY = y;

        firstMouse = false;
    }

    float dx = x - lastX;
    float dy = y - lastY;

    lastX = x;
    lastY = y;

    camYaw -= dx;
    camPitch = glm::clamp(camPitch + dy, -89.0f, 89.0f);

    while (camYaw > 180.0f) camYaw -= 360.0f;
    while (camYaw < -180.0f) camYaw += 360.0f;

    //std::cout << camYaw << " " << camPitch << std::endl;

    camQuat = glm::quat(glm::vec3(glm::radians(camPitch), glm::radians(camYaw), 0));

    glm::vec3 camForward = camQuat * glm::vec3(0, 0, 1);
    glm::vec3 camUp = camQuat * glm::vec3(0, 1, 0);
    view = glm::lookAt(cameraPosition, cameraPosition + camForward, camUp);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        keys[key] = true;
    }
    else if (action == GLFW_RELEASE)
    {
        keys[key] = false;
    }
}

void createGeometry(GLuint& vao, unsigned int& ebo, int& size, int& sizeIndices)
{
    // need 24 vertices for normal/uv-mapped Cube
    float vertices[] = 
    {
        // positions            //colors            // tex coords   // normals          //tangents      //bitangents
        0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
        0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
        -0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
        -0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,

        0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,
        0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   1.f, 0.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,

        0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,
        -0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,

        -0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   0.f, 0.f,      -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,
        -0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   0.f, 1.f,      -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,

        -0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   0.f, 1.f,      0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,
        0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,      0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,

        -0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
        -0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f,

        0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,
        -0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,

        -0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   1.f, 0.f,       -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,
        -0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   1.f, 1.f,       -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,

        -0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,
        0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,

        0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,
        0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   0.f, 0.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,

        0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
        0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f
    };

    unsigned int indices[] = 
    {  // note that we start from 0!
        // DOWN
        0, 1, 2,   // first triangle
        0, 2, 3,    // second triangle
        // BACK
        14, 6, 7,   // first triangle
        14, 7, 15,    // second triangle
        // RIGHT
        20, 4, 5,   // first triangle
        20, 5, 21,    // second triangle
        // LEFT
        16, 8, 9,   // first triangle
        16, 9, 17,    // second triangle
        // FRONT
        18, 10, 11,   // first triangle
        18, 11, 19,    // second triangle
        // UP
        22, 12, 13,   // first triangle
        22, 13, 23,    // second triangle
    };

    int stride = (3 + 3 + 2 + 3 + 3 + 3) * sizeof(float);

    size = sizeof(vertices) / stride;
    sizeIndices = sizeof(indices) / sizeof(int);

    //Create square.
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    //Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //Texture coordinates attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    //Normals attribute
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_TRUE, stride, (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    //Tangents attribute
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_TRUE, stride, (void*)(11 * sizeof(float)));
    glEnableVertexAttribArray(4);

    //Bitangents attribute
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_TRUE, stride, (void*)(14 * sizeof(float)));
    glEnableVertexAttribArray(5);
}

void createShaders()
{
    createProgram(simpleProgram, "shaders/SimpleVertex.shader", "shaders/SimpleFragment.shader");

    glUseProgram(simpleProgram);

    glUniform1i(glGetUniformLocation(simpleProgram, "texture1"), 0);
    glUniform1i(glGetUniformLocation(simpleProgram, "texture1Normal"), 1);


    createProgram(skyProgram, "shaders/SkyVertex.shader", "shaders/SkyFragment.shader");
    createProgram(terrainProgram, "shaders/TerrainVertex.shader", "shaders/TerrainFragment.shader");

    glUseProgram(terrainProgram);

    glUniform1i(glGetUniformLocation(terrainProgram, "texture1"), 0);
    glUniform1i(glGetUniformLocation(terrainProgram, "texture1Normal"), 1);

    glUniform1i(glGetUniformLocation(terrainProgram, "dirt"), 2);
    glUniform1i(glGetUniformLocation(terrainProgram, "sand"), 3);
    glUniform1i(glGetUniformLocation(terrainProgram, "grass"), 4);
    glUniform1i(glGetUniformLocation(terrainProgram, "rock"), 5);
    glUniform1i(glGetUniformLocation(terrainProgram, "snow"), 6);

    createProgram(modelProgram, "shaders/model.vs", "shaders/model.fs");

    glUseProgram(modelProgram);

    glUniform1i(glGetUniformLocation(modelProgram, "texture_diffuse1"), 0);
    glUniform1i(glGetUniformLocation(modelProgram, "texture_specular1"), 1);
    glUniform1i(glGetUniformLocation(modelProgram, "texture_normal1"), 2);
    glUniform1i(glGetUniformLocation(modelProgram, "texture_roughness1"), 3);
    glUniform1i(glGetUniformLocation(modelProgram, "texture_ao1"), 4);

    createProgram(waterProgram, "shaders/WaterVertex.shader", "shaders/WaterFragment.shader");

    glUseProgram(waterProgram);

    glUniform1i(glGetUniformLocation(waterProgram, "textureNormal"), 0);
    glUniform1i(glGetUniformLocation(waterProgram, "textureTerrain"), 1);
}

void createProgram(GLuint& programID, const char* vertex, const char* fragment)
{
    char* vertexSrc;
    char* fragmentSrc;

    loadFile(vertex, vertexSrc);
    loadFile(fragment, fragmentSrc);

    GLuint vertexShaderID, fragmentShaderID;

    //Create vertex shader.
    vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderID, 1, &vertexSrc, nullptr);
    glCompileShader(vertexShaderID);

    int success;
    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(vertexShaderID, 512, nullptr, infoLog);
        std::cout << "ERROR - Compiling vertex shader.\n" << infoLog << std::endl;
    }

    //Create fragment shader.
    fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderID, 1, &fragmentSrc, nullptr);
    glCompileShader(fragmentShaderID);

    glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShaderID, 512, nullptr, infoLog);
        std::cout << "ERROR - Compiling fragment shader.\n" << infoLog << std::endl;
    }

    programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);

    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(programID, 512, nullptr, infoLog);
        std::cout << "ERROR - Linking program.\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    delete vertexSrc;
    delete fragmentSrc;
}

void loadFile(const char* filename, char*& output)
{
    //Open file.
    std::ifstream file(filename, std::ios::binary);

    if (file.is_open())
    {
        //Get file length.
        file.seekg(0, file.end);
        int length = file.tellg();
        file.seekg(0, file.beg);

        //Allocate memory.
        output = new char[length + 1];

        //Read file as block.
        file.read(output, length);

        //Add null terminator to end of char pointer (?).
        output[length] = '\0';

        //Close file.
        file.close();
    }
    else
    {
        //If it failed just set it to null.
        output = NULL;
    }
}

GLuint loadTexture(const char* path, int comp)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, sizeChannels;
    unsigned char* data = stbi_load(path, &width, &height, &sizeChannels, comp);
    if (data)
    {
        if (comp != 0) sizeChannels = comp;

        if (sizeChannels == 3)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else if (sizeChannels == 4)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }

        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "ERROR - Loading texture: " << path << std::endl;
    }

    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}
#include <KHR\khrplatform.h>
#include <glad\glad.h>
#include <glfw3.h>
#include <glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/perpendicular.hpp>

#include <iostream>
#include <vector>
#include <algorithm>
#include <random>

#include "..\shaders\Shader.h"
#include "Camera2.h"
#include "..\shaders\ComputeShader.h"



void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
void processInput(GLFWwindow* window);

const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;

const float TAU = 6.28318531;
const float PI = 3.141592654;
const float E = 2.7182818285;
const float G = 1.0;
float DT = 0.0001666;
const float C = 1.0;
const int numCubeVertices = 108;

float yaw = 0.0;
float pitch = 0.0;
bool firstMouse = true;
float lastX = SCR_WIDTH / 2;
float lastY = SCR_HEIGHT / 2;
// Camera camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f), 0.005f);


struct Vertex 
{
    glm::vec3 pos;
    glm::vec3 color;
};

struct Mesh
{
    GLuint VAO;
    GLuint VBO;
    GLsizei vertexCount;
};

struct instancedMesh
{
    GLuint VAO;
    GLuint VBO;
    GLuint instanceVBO;

    GLsizei vertexCount;
};

struct Mesh2
{
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;

    GLsizei vertexCount;
    GLsizei indexCount;
};

class FPSHandler
{
    public:
        float fps;
        float startTime;
        float endTime;
        int frames;

        FPSHandler();

        void fpsCheck()
        {
            if (endTime - startTime > 1.0)
            {
                fps = frames / (endTime - startTime);
                startTime = glfwGetTime();
                frames = 0;

                std::cout << "FPS: " << fps << "\r" << std::flush;
            }
        }
};

FPSHandler::FPSHandler()
{
    float fps = 0.0;
    float startTime = glfwGetTime();
    float endTime = 0.0;
    int frames = 0;
}

std::vector<Vertex> getCircleVert(float radius, glm::vec3 pos, glm::vec3 color, float da)
{
    std::vector<Vertex> vertices;

    vertices.push_back({
        pos,
        color
    });

    for (float i = 0.0; i < TAU; i += da)
    {
        float x = radius * cos(i);
        float y = radius * sin(i);

        vertices.push_back(
            {glm::vec3(x + pos.x, y + pos.y, 0.0f), color}
        );

        if (i + da > TAU)
        {
            vertices.push_back(
                {glm::vec3(radius + pos.x, 0.0f, 0.0f), color}
            );
        }
    }
    return vertices;
}

GLFWwindow* initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    return window;
}

Mesh2 bufferTriangle2(std::vector<Vertex> vertices, std::vector<unsigned int> indices, int buffer)
{
    Mesh2 mesh;
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &mesh.EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    mesh.vertexCount = vertices.size();
    mesh.indexCount = indices.size();
    return mesh;
}

instancedMesh bufferInstancedParticles2(std::vector<Vertex> circleVert, std::vector<Particle2> particles)
{
    std::vector<glm::vec3> translations;
    for (auto& a : particles)
        translations.push_back(a.position);


    instancedMesh mesh;
    glGenBuffers(1, &mesh.instanceVBO);

    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, circleVert.size() * sizeof(Vertex), circleVert.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, translations.size() * sizeof(glm::vec3), translations.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0); // instanced stuff
    glVertexAttribDivisor(2, 1);

    mesh.vertexCount = circleVert.size();
    return mesh;
}

void updateUniforms(
    Camera2D& camera, GLFWwindow* window, GLuint& cameraZoomLoc, 
    GLuint& cameraPosLoc, glm::vec2& windowSize, GLuint& windowSizeLoc,
    GLuint& dtLoc
)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glUniform2f(windowSizeLoc, width, height);

    glUniform1f(cameraZoomLoc, camera.zoom);
    glUniform2f(cameraPosLoc, camera.pos.x, camera.pos.y);

    glUniform1f(dtLoc, DT);
}

// https://gamedev.stackexchange.com/questions/179426/c-generate-random-float-values-between-a-range
class RandomNumberGenerator
{
    std::random_device m_randomDevice{};
    std::mt19937 m_engine{m_randomDevice()};

    public:
        // Generates a random float in the range [low, high)
        float Generate(float low, float high)
        {
            return std::uniform_real_distribution<float>{low, high}(m_engine);
        }
};

std::pair<float, float> getParticleForces(int& type)
{
    float forceStrength, forceDist = 0.0;

    if (type == 0)
    {
        forceStrength = 5.0;
        forceDist = 2.0;
    }
    if (type == 1)
    {
        forceStrength = 0.5;
        forceDist = 1.0;
    }
    return std::pair<float, float>(forceStrength, forceDist);
}

float lerp(float v0, float v1, float t) 
{
  return v0 + t * (v1 - v0);
}

void helpParticleShader(ComputeShader& shader, std::vector<Particle2>& particles, GLuint& particlesLoc)
{
    // in the shader, local_size_x = 128

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shader.SSBOList[1]); // next particles
    // std::cout << "worked\n";
    glGetBufferSubData(
        GL_SHADER_STORAGE_BUFFER,
        0,
        particles.size() * sizeof(Particle2),
        particles.data()
    );
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shader.SSBOList[0]); // current particles
    std::swap(shader.SSBOList[0], shader.SSBOList[1]);
}

std::pair<GLuint, GLuint> makeSSBOs(std::vector<Particle2>& particles, ComputeShader& shader)
{
    glUseProgram(shader.computeProgram);

    GLuint SSBOcurrent;
    glGenBuffers(1, &SSBOcurrent);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOcurrent);
    glBufferData(GL_SHADER_STORAGE_BUFFER, particles.size() * sizeof(Particle2), particles.data(), GL_DYNAMIC_DRAW); // static size for now
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBOcurrent);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    GLuint SSBOnext;
    glGenBuffers(1, &SSBOnext);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOnext);
    glBufferData(GL_SHADER_STORAGE_BUFFER, particles.size() * sizeof(Particle2), particles.data(), GL_DYNAMIC_DRAW); // static size for now
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, SSBOnext);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    return std::pair(SSBOcurrent, SSBOnext);
}

GLuint setupParticles(std::vector<Particle2>& particles, ComputeShader& shader)
{
    glUseProgram(shader.computeProgram);
    GLuint particlesLoc = glGetUniformLocation(shader.computeProgram, "count");
    glUniform1ui(particlesLoc, particles.size());

    std::pair<GLuint, GLuint> s = makeSSBOs(particles, shader);
    shader.SSBOList.push_back(s.first);
    shader.SSBOList.push_back(s.second);
    return particlesLoc;
}

int main(int argc, char** argv)
{
    GLFWwindow* window = initWindow();
    Shader mainShader("shaders/vshader.glsl", "shaders/fshader.glsl");
    GLuint windowSizeLoc = glGetUniformLocation(mainShader.ID,"windowSize");

    GLuint cameraZoomLoc = glGetUniformLocation(mainShader.ID,"cameraZoom");
    GLuint cameraPosLoc = glGetUniformLocation(mainShader.ID,"cameraPos");

    glm::vec2 windowSize = glm::vec2(SCR_WIDTH, SCR_HEIGHT);
    RandomNumberGenerator rng = RandomNumberGenerator();
 
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // glfwSetCursorPosCallback(window, mouse_callback);

    glEnable(GL_DEPTH_TEST);

    // initialize particle states
    float step = 0.02;
    float radius = 0.005;
    std::vector<Particle2> particles2;
    for (float x = -0.8; x < 0.8; x += step)
    {
        for (float y = -0.8; y < 0.8; y += step)
        {
            int type = 0;
            float rando = rng.Generate(0, 1);
            if (rando > 0.5) type = 1;
            std::pair<float, float> pair = getParticleForces(type);
            particles2.push_back({glm::vec4(rng.Generate(-0.8, 0.8), rng.Generate(-0.8, 0.8), 0.0, 0.0), glm::vec4(1.0, 0.0, 0.0, 0.0), radius, pair.first, pair.second, type});
        }
    }
    std::cout << "particle count: " << particles2.size() << std::endl;

    std::vector circleVert = getCircleVert(radius, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.8, 0.8, 0.1), 2);
    instancedMesh particlesMesh2 = bufferInstancedParticles2(circleVert, particles2);

    // setup camera movement
    Camera2D camera = Camera2D(glm::vec2(0.0, 0.0), 1.0, 1.0);

    // setup FPS tracking
    FPSHandler fpsCounter = FPSHandler();

    // setup compute shader
    ComputeShader particleShader(
        "shaders/particleShader.glsl", 
        glm::ivec3(128, 1, 1), 
        glm::uvec3(particles2.size(), 1, 1)
    );
    GLuint particlesLoc = setupParticles(particles2, particleShader);
    GLuint dtLoc = glGetUniformLocation(particleShader.computeProgram, "dt");
    glUniform1f(dtLoc, DT);

    while (!glfwWindowShouldClose(window))
    {
        fpsCounter.frames += 1;
        fpsCounter.fpsCheck();
        if (fpsCounter.startTime > 1.0f)
            DT = lerp(DT, 1.0 / fpsCounter.fps, 0.0001);

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mainShader.use();

        std::vector<glm::vec3> positions(particles2.size());
        for (int i = 0; i < positions.size(); i++)
        {
            positions[i] = glm::vec3(particles2[i].position.x, particles2[i].position.y, particles2[i].position.z);
        }

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * positions.size(), positions.data());
        glBindVertexArray(particlesMesh2.VAO);
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, particlesMesh2.vertexCount, positions.size());

        updateUniforms(
            camera, window, cameraZoomLoc, 
            cameraPosLoc, windowSize, windowSizeLoc,
            dtLoc
        );

        camera.doCameraMovement(window);
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, 1);

        glfwSwapBuffers(window);
        glfwPollEvents();

        fpsCounter.endTime = glfwGetTime();
        
        glUniform1ui(particlesLoc, particles2.size());
        particleShader.use();
        helpParticleShader(particleShader, particles2, particlesLoc);
    }

    return 0;
}
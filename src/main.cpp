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


struct vertex 
{
    glm::vec3 pos;
    glm::vec3 color;
};

struct Vertex2
{
    glm::vec2 pos;
    glm::vec4 color;
    float luminance;
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

struct Mesh3
{
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;

    glm::mat4 modelMatrix = glm::rotate(modelMatrix, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    GLsizei vertexCount;
    GLsizei indexCount;
};

struct RenderGroup
{
    GLuint VAO;
    GLuint VBO;
    glm::mat4 model;
    std::vector<vertex> vertices;
};

struct Ray2 // as in 2D ray not as in version 2
{
    glm::vec2 pos;
    glm::vec4 color;
    glm::vec2 polar; // polar
    glm::vec2 vel;
    std::vector<Vertex2> head;
    std::vector<Vertex2> trail;

    float dr;
    float dθ;


    Ray2(glm::vec2 position, glm::vec2 velocity, glm::vec4 colors, glm::vec2 polarCoord, std::vector<Vertex2> headVert,std::vector<Vertex2> trailVert)
        : pos(position), vel(velocity), color(colors), polar(polarCoord), head(headVert), trail(trailVert) {}

    Ray2(glm::vec2 position, glm::vec2 velocity, glm::vec4 colors)
        : pos(position), vel(velocity), color(colors), polar(glm::vec2(glm::length(position), atan2(position.y, position.x))), 
        dr(0.001), dθ(0.001),
        head({
            Vertex2({glm::vec2(pos.x, pos.y + 0.02), color, 1.0}), 
            Vertex2({glm::vec2(pos.x + 0.02, pos.y), color, 1.0}), 
            Vertex2({glm::vec2(pos.x, pos.y - 0.02), color, 1.0})
        }), 
        trail({}) {}
};

struct Particle
{
    glm::vec3 velocity;
    float radius;
    int type;
};

// struct Particle2
// {
//     glm::vec3 position;
//     glm::vec3 velocity;
//     float radius;
//     float forceStrength;
//     float forceDist;
//     int type;
// };

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

std::vector<vertex> getCircleVert(float radius, glm::vec3 pos, glm::vec3 color, float da)
{
    std::vector<vertex> vertices;

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

Mesh bufferTriangle(std::vector<vertex> vertices, int buffer)
{
    Mesh mesh;
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), vertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, color));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);



    mesh.vertexCount = vertices.size();
    return mesh;
}

Mesh2 bufferTriangle2(std::vector<vertex> vertices, std::vector<unsigned int> indices, int buffer)
{
    Mesh2 mesh;
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), vertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, color));
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

Mesh bufferPoints(std::vector<vertex> points, int buffer)
{
    Mesh mesh;
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(vertex), points.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, color));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    mesh.vertexCount = points.size();
    return mesh;
}

instancedMesh bufferInstanced(std::vector<vertex> circleVert, std::vector<glm::vec3> translations)
{
    instancedMesh mesh;
    glGenBuffers(1, &mesh.instanceVBO);

    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, circleVert.size() * sizeof(vertex), circleVert.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, color));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, translations.size() * sizeof(glm::vec3), translations.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0); // instanced stuff
    glVertexAttribDivisor(2, 1);

    mesh.vertexCount = circleVert.size();
    return mesh;
}

instancedMesh bufferInstancedParticles2(std::vector<vertex> circleVert, std::vector<Particle2> particles)
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
    glBufferData(GL_ARRAY_BUFFER, circleVert.size() * sizeof(vertex), circleVert.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, color));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, translations.size() * sizeof(glm::vec3), translations.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0); // instanced stuff
    glVertexAttribDivisor(2, 1);

    mesh.vertexCount = circleVert.size();
    return mesh;
}

void updateUniformMatrices(Shader& ourShader, 
    glm::mat4& model, glm::mat4& view, glm::mat4& proj,
    GLuint& modelLoc, GLuint& viewLoc, GLuint& projLoc
)
{
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
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

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    // if (firstMouse)
    // {
    //     lastX = xpos;
    //     lastY = ypos;
    //     firstMouse = false;
    // }

    // float xoffset = xpos - lastX;
    // float yoffset = lastY - ypos; 
    // lastX = xpos;
    // lastY = ypos;

    // float sensitivity = 0.1f;
    // xoffset *= sensitivity;
    // yoffset *= sensitivity;

    // yaw += xoffset;
    // pitch += yoffset;

    // if(pitch > 89.0f)
    //     pitch = 89.0f;
    // if(pitch < -89.0f)
    //     pitch = -89.0f;

    // glm::vec3 direction;
    // direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    // direction.y = sin(glm::radians(pitch));
    // direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    // camera.setCameraFront(glm::normalize(direction));
}

std::vector<vertex> vertToVectors(std::vector<float> vertices, glm::vec3 color)
{
    std::vector<vertex> vectors;
    for (int i = 0; i < numCubeVertices; i+=3)
    {
        vectors.push_back({
            glm::vec3(vertices[i], vertices[i + 1], vertices[i + 2]),
            color
        });
    }

    return vectors;
}

std::vector<float> getCubeVert()
{
    std::vector<float> vec = {
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
    return vec;
}

std::pair<std::vector<vertex>, std::vector<unsigned int>> getSphereVert(float radius, int sides, int strips, glm::vec3 color)
{
    std::vector<vertex> vertices;
    std::vector<unsigned int> indices;

    for (int i = 0; i <= strips; i++)
    {
        float V = ((float) i) / ((float) strips);
        float phi = V * PI;

        for (int u = 0; u <= sides; u++)
        {
            float U = ((float) u) / ((float) sides);
            float theta = U * (PI * 2.0);

            float x = (radius * cos(theta) * sin(phi)) - 3;
            float y = (radius * cos(phi)) - 3;
            float z = (radius * sin(theta) * sin(phi)) - 3;

            vertices.push_back({glm::vec3(x, y, z), color});
        }
    }

    for (int i = 0; i < strips; i++)
    {
        for (int u = 0; u < sides; u++)
        {
            int v0 = i * (sides + 1) + u;
            int v1 = v0 + 1;
            int v2 = (i + 1) * (sides + 1) + u;
            int v3 = v2 + 1;

            indices.push_back(v0);
            indices.push_back(v1);
            indices.push_back(v2);

            indices.push_back(v1);
            indices.push_back(v2);
            indices.push_back(v3);
        }
    }

    return std::make_pair(vertices, indices);
}

Mesh bufferRay(Ray2& ray, int buffer)
{
    Mesh mesh;
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, ray.head.size() * sizeof(Vertex2), ray.head.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex2), (void*)offsetof(Vertex2, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex2), (void*)offsetof(Vertex2, color));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    mesh.vertexCount = ray.head.size();
    return mesh;
}

Mesh bufferRays(std::vector<Vertex2>& heads, int buffer)
{
    Mesh mesh;
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, heads.size() * sizeof(Vertex2), heads.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex2), (void*)offsetof(Vertex2, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex2), (void*)offsetof(Vertex2, color));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    mesh.vertexCount = heads.size();
    return mesh;
}

float getSchwartzschildRadius(float& bMass){
    return (G * bMass * 2.0) / (C * C);
}

glm::vec2 applySchwartzschildMetric(Ray2& ray, float& schwartzschildRadius)
{
    // φθ
    // float arg = atan2(ray.vel.y, ray.vel.x);
    // float dθ = arg * DT;
    // float dr = C * DT;

    float dθ = ray.dθ;
    float dr = ray.dr;

    // dθ = -2.0 * dr * dθ / ray.polar.x;
    // dr += ray.polar.x * (dθ * dθ) - ((C * C) * schwartzschildRadius) / (2.0 * (ray.polar.x * ray.polar.x));



    // keep in mind that G = 1.0 and c = 1.0
    
    float d2θ = (-2.0 / ray.polar.x) * dθ * dr;
    float d2r = (((-C * C) * schwartzschildRadius) / (2.0 * (ray.polar.x * ray.polar.x))) + (ray.polar.x * (dθ * dθ));
    ray.dr += d2r * DT;
    ray.dθ += d2θ * DT;

    // std::cout << "d2theta: " << d2θ << " -- d2r: " << d2r << std::endl;
    return glm::vec2(d2θ, d2r); // polar acceleration vector
}

void updateRay(Ray2& ray, int maxIter, float& schwartzschildRadius)
{
    glm::vec2 vNorm = glm::normalize(ray.vel);
    glm::vec2 vPerp = glm::vec2(-vNorm.y, vNorm.x);

    ray.head.at(0).pos = ray.pos + (vNorm * 0.02f);
    ray.head.at(1).pos = ray.pos - (vNorm * 0.02f) + (vPerp * 0.02f);
    ray.head.at(2).pos = ray.pos - (vNorm * 0.02f) - (vPerp * 0.02f);

    // integrate & apply motion
    // temporary euler method
    
    ray.pos += ray.vel * DT;
    glm::vec2 polarAccel = applySchwartzschildMetric(ray, schwartzschildRadius);
    glm::vec2 cartAccel = glm::vec2(-polarAccel.x * cos(polarAccel.y), -polarAccel.x * sin(polarAccel.y));
    ray.vel += cartAccel * DT;

    // glm::vec2 polarAccel;
    // glm::vec2 cartAccel;
    // std::vector<glm::vec2> positions;
    // glm::vec2 posSum = glm::vec2(0.0, 0.0);
    // std::vector<glm::vec2> velocities;
    // glm::vec2 velSum = glm::vec2(0.0, 0.0);
    // for (int i = 0; i < maxIter; i++)
    // {
    //     positions.push_back(ray.pos + (ray.vel * DT));
    //     posSum += positions[i];
    //     polarAccel = applySchwartzschildMetric(ray, schwartzschildRadius);
    //     cartAccel = glm::vec2(-polarAccel.x * cos(polarAccel.y), -polarAccel.x * sin(polarAccel.y));
    //     velocities.push_back(ray.vel + (cartAccel * DT));
    //     velSum += velocities[i];
    // }
    // ray.pos = posSum / (float) maxIter;
    // ray.vel = velSum / (float) maxIter;

    // std::cout << ray.vel.y << std::endl;
    ray.polar = glm::vec2(glm::length(ray.pos), atan2(ray.pos.y, ray.pos.x));
    // ray.trail.push_back(Vertex2({ray.pos, ray.color, 1.0})); // no mesh yet
}

float gFunc(float x)
{
    return 1.0 / ((x * x) + 0.1);
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

/*
void updateParticles2(std::vector<Particle2>& particles)
{
    for (int i = 0; i < particles.size(); i++)
    {
        float mag = glm::length(particles[i].position);
        glm::vec3 norm = glm::normalize(particles[i].position);
        // glm::vec3 g = norm * -(1.0f / ((mag * mag) + 0.5f));
        glm::vec3 g = glm::vec3(0.0);
        for (int j = 0; j < particles.size(); j++)
        {
            if (&particles[j] != &particles[i])
            {
                glm::vec3 dif = particles[j].position - particles[i].position;
                float dist2 = glm::dot(dif, dif);
                if (dist2 < 0.01) continue;
                float result = gFunc(dist2) - (particles[i].forceDist * gFunc(particles[i].forceStrength * dist2));
                g += glm::normalize(dif) * (G * result);
            }
        }


        particles[i].velocity += g * DT;
        particles[i].position += particles[i].velocity * DT;

        float distx = abs(particles[i].position.x);
        float disty = abs(particles[i].position.y);
        if (distx + 0.01 > 0.95 || distx - 0.01 < -0.95)
        {
            particles[i].position.x = glm::clamp(particles[i].position.x, -1.0f, 1.0f);
            particles[i].velocity.x *= -1.0;
        }
        if (disty + 0.01 > 0.95 || disty - 0.01 < -0.95)
        {
            particles[i].position.y = glm::clamp(particles[i].position.y, -1.0f, 1.0f);
            particles[i].velocity.y *= -1.0;
        }
    }
}

bool comparePos(const Particle2& p1, const Particle2& p2)
{
    return p1.position.x > p2.position.x;
}

void handleCollisions2(std::vector<Particle2>& particles)
{
    // std::stable_sort(particles2.begin(), particles2.end(), comparePos);
    // float friction = 1.0;
    float friction = 0.92;
    
    float radius = particles[0].radius;
    for (int z = 0; z < 10; z++)
    {
        for (int i = 0; i < particles.size(); i++)
        {
            for (int q = i + 1; q < particles.size(); q++)
            {
                if (particles[q].position.x - particles[i].position.x > 2 * radius) continue;
                float dist = glm::distance(particles[i].position, particles[q].position);
                if ((dist < particles[i].radius + particles[q].radius))
                {
                    glm::vec3 norm;
                    float penetration = (particles[i].radius + particles[q].radius) - glm::distance(particles[i].position, particles[q].position);
                    if (dist > 0.0001) norm = glm::normalize(particles[q].position - particles[i].position);
                    else norm = glm::vec3(1, 0, 0);
                    particles[i].position -= norm * (penetration * 0.5f);
                    particles[q].position += norm * (penetration * 0.5f);

                    if (glm::dot(particles[q].velocity - particles[i].velocity, norm) < 0.0f)
                    {
                        float v1 = glm::dot(particles[i].velocity, norm);
                        float v2 = glm::dot(particles[q].velocity, norm);

                        particles[i].velocity = (particles[i].velocity + ((v2 - v1) * norm)) * friction;
                        particles[q].velocity = (particles[q].velocity + ((v1 - v2) * norm)) * friction;
                    }
                    // std::cout << "collision" << std::endl;
                }
            }
        }
    }
}
*/

void addParticles(std::vector<Particle2>& particles, int& count, int& type, glm::vec2& bounds)
{
    float xstep = 0.2;
    float ystep = 0.2;
    float radius = 0.01;
    for (float x = 0.0; x < bounds.x; x += xstep)
    {
        for (float y = 0.0; y < bounds.y; y += ystep)
        {
            particles.push_back({glm::vec4(x, y, 0.0, 0.0), glm::vec4(0.0, 0.0, 0.0, 0.0), radius, 5.0, 2.0, type});
        }
    }
}

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
            // glm::vec2 norm = glm::normalize(glm::vec2(x, y));
            particles2.push_back({glm::vec4(rng.Generate(-0.8, 0.8), rng.Generate(-0.8, 0.8), 0.0, 0.0), glm::vec4(1.0, 0.0, 0.0, 0.0), radius, pair.first, pair.second, type});
        }
    }
    std::cout << "particle count: " << particles2.size() << std::endl;

    std::vector circleVert = getCircleVert(radius, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.8, 0.8, 0.1), 2);
    instancedMesh particlesMesh2 = bufferInstancedParticles2(circleVert, particles2);

    Camera2D camera = Camera2D(glm::vec2(0.0, 0.0), 1.0, 1.0);

    FPSHandler fpsCounter = FPSHandler();






    
    ComputeShader particleShader(
        "shaders/particleShader.glsl", 
        glm::ivec3(128, 1, 1), 
        glm::uvec3(particles2.size(), 1, 1)
    );
    GLuint particlesLoc = setupParticles(particles2, particleShader);
    // GLuint particlesLoc = particleShader.setupParticles(particles2, 2);
    GLuint dtLoc = glGetUniformLocation(particleShader.computeProgram, "dt");
    glUniform1f(dtLoc, DT);


    while (!glfwWindowShouldClose(window))
    {
        fpsCounter.frames += 1;
        fpsCounter.fpsCheck();
        if (fpsCounter.startTime > 1.0f)
            DT = lerp(DT, 1.0 / fpsCounter.fps, 0.0001);

        // updateParticles2(particles2);
        // handleCollisions2(particles2);

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
        // particleShader.use(particles2, particlesLoc);
    }

    return 0;
}
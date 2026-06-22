#include <KHR\khrplatform.h>
#include <glad/glad.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <glfw3.h>

#include <iostream>

#include "..\shaders\Shader.h"

#pragma once

const float deltaTime = 0.005;

class Camera2D
{
    public:
        glm::vec2 pos;
        float zoom;
        float speed;
        float zoomSpeed;
        Camera2D(glm::vec2 cameraPos, float cameraZoom, float cameraSpeed)
        {
            this->pos = cameraPos;
            this->zoom = cameraZoom;
            this->speed = cameraSpeed * deltaTime;
            this->zoomSpeed = cameraSpeed * 3;
        }
        
        void doCameraMovement(GLFWwindow *window)
        {

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            {
                pos.y += speed;
            }
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            {
                pos.y -= speed;
            }
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            {
                pos.x += speed;
            }
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            {
                pos.x -= speed;
            }

            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            {
                if (zoom < 3.0)
                {
                    zoom *= std::pow(zoomSpeed, deltaTime);
                    speed *= std::pow(zoomSpeed * 0.5, deltaTime);
                }
            }
            if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            {
                if (zoom > 0.0)
                {
                    zoom /= std::pow(zoomSpeed, deltaTime);
                    speed /= std::pow(zoomSpeed * 0.5, deltaTime);
                }
            }

            zoom = glm::clamp(zoom, 0.0f, 20.0f);
        }
};
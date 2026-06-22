#include <KHR\khrplatform.h>
#include <glad/glad.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <glfw3.h>

#include <iostream>

#include "..\shaders\Shader.h"

class Camera 
{
    public:
        glm::vec3 cameraPos;
        float speed;
        glm::vec3 cameraFront;
        glm::vec3 cameraUp;

        Camera(glm::vec3 camPos, float camSpeed)
        {
            this->cameraPos = camPos;
            this->speed = camSpeed;

            glm::vec3 cameraTarget = glm::vec3(0.0f);
            glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f); 
            glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
            this->cameraUp = glm::cross(cameraDirection, cameraRight);
            this->cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
        }
        glm::mat4 updateView(glm::mat4& viewMatrix)
        {
            viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
            return viewMatrix;
        }
        void doCameraMovement(GLFWwindow *window)
        {
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                glfwSetWindowShouldClose(window, 1);
            if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                cameraPos += speed * cameraFront;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                cameraPos -= speed * cameraFront;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
        }
        void setCameraFront(glm::vec3 vec)
        {
            cameraFront = vec;
        }
};
#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm.hpp>

struct Particle2
{
    glm::vec4 position;
    glm::vec4 velocity;
    float radius;
    float forceStrength;
    float forceDist;
    int type;
};

class ComputeShader
{
public:
    unsigned int computeProgram;
    std::vector<GLuint> SSBOList;
    glm::ivec3 computeSize;
    glm::uvec3 groupSize;

    ComputeShader(const char* filepath, glm::ivec3& computeSize, glm::uvec3& dataSize)
    {
        std::string shaderCode;
        std::ifstream shaderFile;

        try
        {
            shaderFile.open(filepath);
            std::stringstream shaderStream;
            shaderStream << shaderFile.rdbuf();
            shaderFile.close();
            shaderCode = shaderStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        }

        const char* computeShaderCode = shaderCode.c_str();
        unsigned int computeShader = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(computeShader, 1, &computeShaderCode, NULL);
        glCompileShader(computeShader);
        checkCompileErrors(computeShader, "COMPUTE");

        computeProgram = glCreateProgram();
        glAttachShader(computeProgram, computeShader);
        glLinkProgram(computeProgram);

        checkCompileErrors(computeProgram, "PROGRAM");

        glDeleteShader(computeShader);

        SSBOList.clear();
        if (computeSize.x <= 0 || computeSize.y <= 0 || computeSize.z <= 0)
            throw std::runtime_error("Invalid compute size in compute shader");
        this->computeSize = computeSize;
        this->groupSize = glm::uvec3(
            (dataSize.x + computeSize.x - 1) / computeSize.x, 
            (dataSize.y + computeSize.y - 1) / computeSize.y,
            (dataSize.z + computeSize.z - 1) / computeSize.z
        );
    }

    void use()
    {
        glUseProgram(computeProgram);

        for (int i = 0; i < SSBOList.size(); i++)
        {
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i, SSBOList[i]);
        }
        glDispatchCompute(groupSize.x, groupSize.y, groupSize.z);

        
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
    }

private:
    void checkCompileErrors(unsigned int shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};
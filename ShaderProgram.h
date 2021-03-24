#ifndef _SHADER_PROGRAM_INCLUDE
#define _SHADER_PROGRAM_INCLUDE

#include <string>
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include "Shader.h"

// Using the Shader class ShaderProgram can link a vertex and a fragment shader
// together, bind input attributes to their corresponding vertex shader names,
// and bind the fragment output to a name from the fragment shader

class ShaderProgram
{

public:
    ShaderProgram();

    void init();
    void addShader(const Shader &shader);
    void bindFragmentOutput(const std::string &outputName);
    GLint bindVertexAttribute(const std::string &attribName, GLint size, GLsizei stride, GLvoid *firstPointer);
    void link();
    void free();

    void use();

    // Pass uniforms to the associated shaders
    void setUniform1i(const std::string &uniformName, int v);
    void setUniform2f(const std::string &uniformName, float v0, float v1);
    void setUniform3f(const std::string &uniformName, float v0, float v1, float v2);
    void setUniform4f(const std::string &uniformName, float v0, float v1, float v2, float v3);

    void setUniformMatrix3f(const std::string &uniformName, const glm::mat3 &mat);
    void setUniformMatrix4f(const std::string &uniformName, const glm::mat4 &mat);

    bool isLinked();
    const std::string &log() const;

private:
    GLuint programId;
    bool linked;
    std::string errorLog;
};

#endif // _SHADER_PROGRAM_INCLUDE

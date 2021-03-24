#ifndef _SHADER_INCLUDE
#define _SHADER_INCLUDE

#include <string>
#include <GL/glew.h>
#include <GL/gl.h>

enum ShaderType
{
    VERTEX_SHADER,
    FRAGMENT_SHADER
};

// This class is able to load to OpenGL a vertex or fragment shader and compile it.
// It can do so from a file or from a string so that shader code can be
// procedurally modified if needed.

class Shader
{

public:
    Shader();

    // These methods should be called with an active OpenGL context
    void initFromSource(const ShaderType type, const std::string &source);
    bool initFromFile(const ShaderType type, const std::string &filename);
    void free();

    GLuint getId() const;
    bool isCompiled() const;
    const std::string &log() const;

private:
    bool loadShaderSource(const std::string &filename, std::string &shaderSource);

private:
    GLuint shaderId;
    bool compiled;
    std::string errorLog;
};

#endif // _SHADER_INCLUDE

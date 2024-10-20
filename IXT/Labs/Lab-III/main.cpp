//
//  main.cpp
//  OpenGL_Shader_Example_step1
//
//  Created by CGIS on 30/11/15.
//  Copyright © 2015 CGIS. All rights reserved.
//

#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

int glWindowWidth = 640;
int glWindowHeight = 480;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

GLuint shaderProgram;

GLfloat vrtxs1[] = {
    .0, .5, .0,
    .5, -.5, .0,
    -.5, -.5, .0
};

GLuint vrtxVBO;
GLuint trigVAO;

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
    fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
    //TODO
}

void initObjects()
{
    glGenBuffers( 1, &vrtxVBO );

    glBindBuffer( GL_ARRAY_BUFFER, vrtxVBO );
    glBufferData( GL_ARRAY_BUFFER, sizeof( vrtxs1 ), vrtxs1, GL_STATIC_DRAW );

    glGenVertexArrays( 1, &trigVAO );
    glBindVertexArray( trigVAO );

    //glBindBuffer( GL_ARRAY_BUFFER, vrtxVBO );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

    glEnableVertexAttribArray( 0 );
    glBindVertexArray( 0 );
}

bool initOpenGLWindow()
{
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //window scaling for HiDPI displays
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    // for multisampling/antialising
    glfwWindowHint(GLFW_SAMPLES, 4);

    glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
    if (!glWindow) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return false;
    }
    
    glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
    glfwMakeContextCurrent(glWindow);

#if not defined (__APPLE__)
    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    printf( "glewInit()\n" );
    if( glewInit() != GLEW_OK )
        printf( "glewInit() fault.\n" );
#endif

    // get version info
    const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
    const GLubyte* version = glGetString(GL_VERSION); // version as a string
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    //for RETINA display
    glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

    return true;
}

void renderScene()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glClearColor( .1, .1, .16, 1.0 );
    glViewport( 0, 0, retina_width, retina_height );

    glUseProgram( shaderProgram );

    glBindVertexArray( trigVAO );
    glDrawArrays( GL_TRIANGLES, 0, 3 );
}

std::string readShaderFile(std::string fileName)
{
    std::ifstream shaderFile;
    std::string shaderString;

    //open shader file
    shaderFile.open(fileName);

    std::stringstream shaderStringStream;

    //read shader content into stream
    shaderStringStream << shaderFile.rdbuf();

    //close shader file
    shaderFile.close();

    //convert stream into GLchar array
    shaderString = shaderStringStream.str();
    return shaderString;
}

void shaderCompileLog(GLuint shaderId)
{
    GLint success;
    GLchar infoLog[512];

    //check compilation info
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shaderId, 512, NULL, infoLog);
        std::cout << "Shader compilation error\n" << infoLog << std::endl;
    }
}

void shaderLinkLog(GLuint shaderProgramId)
{
    GLint success;
    GLchar infoLog[512];

    //check linking info
    glGetProgramiv(shaderProgramId, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "Shader linking error\n" << infoLog << std::endl;
    }
}

GLuint initBasicShader(std::string vertexShaderFileName, std::string fragmentShaderFileName)
{
    GLuint shaderProgram;

    //read, parse and compile the vertex shader
    std::string v = readShaderFile(vertexShaderFileName);
    const GLchar* vertexShaderString = v.c_str();
    GLuint vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderString, NULL);
    glCompileShader(vertexShader);
    //check compilation status
    shaderCompileLog(vertexShader);

    //read, parse and compile the fragment shader
    std::string f = readShaderFile(fragmentShaderFileName);
    const GLchar* fragmentShaderString = f.c_str();
    GLuint fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderString, NULL);
    glCompileShader(fragmentShader);
    //check compilation status
    shaderCompileLog(fragmentShader);

    //attach and link the shader programs
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    //check linking info
    shaderLinkLog(shaderProgram);

    return shaderProgram;
}

void cleanup() {
    glfwDestroyWindow(glWindow);
    //close GL context and any other GLFW resources
    glfwTerminate();
}

int main(int argc, const char * argv[]) {

    if (!initOpenGLWindow()) {
        glfwTerminate();
        return 1;
    }

    initObjects();

    shaderProgram = initBasicShader(SOURCE_DIR"/shaders/shader.vert", SOURCE_DIR"/shaders/shader.frag");

    while (!glfwWindowShouldClose(glWindow)) {
        renderScene();
        glfwPollEvents();
        glfwSwapBuffers(glWindow);
    }

    cleanup();

    return 0;
}

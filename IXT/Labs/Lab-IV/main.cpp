//
//  main.cpp
//  OpenGL_Shader_Example_step1
//
//  Created by CGIS on 30/11/15.
//  Copyright © 2015 CGIS. All rights reserved.
//

#include <IXT/ring-0.hpp>
using namespace IXT;

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
Surface* surf = nullptr;

GLuint shaderProgram;

GLfloat vrtx_data[] = {
    .0, .7, .0, .0, .0, .0,
    -.5, .5, .0, 1.0, 1.0, 1.0,
    -.5, -.5, .0, 1.0, 0.0, 1.0,
    .5, -.5, .0, 1.0, 1.0, 0.0,
    .5, .5, .0, 0.0, 1.0, 1.0
};
GLuint vrtx_idx[] = {
    0, 1, 4,
    1, 2, 3,
    1, 3, 4
};
GLuint VBO, VAO, VEO;

void initObjects() {
    glGenVertexArrays( 1, &VAO );
    glBindVertexArray( VAO );

    glGenBuffers( 1, &VBO );
    glBindBuffer( GL_ARRAY_BUFFER, VBO );
    glBufferData( GL_ARRAY_BUFFER, sizeof( vrtx_data ), vrtx_data, GL_STATIC_DRAW );

    glGenBuffers( 1, &VEO );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, VEO );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( vrtx_idx ), vrtx_idx, GL_STATIC_DRAW );

    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof( GLfloat ), ( GLvoid* )0 );
    glEnableVertexAttribArray( 0 );

    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof( GLfloat ), ( GLvoid* )( 3 * sizeof( GLfloat ) ) );
    glEnableVertexAttribArray( 1 );

    glBindVertexArray( 0 );
}

bool initOpenGLWindow() {
    surf->uplink( SURFACE_THREAD_ACROSS );
    
    surf->uplink_context_on_this_thread();

    // get version info
    const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
    const GLubyte* version = glGetString(GL_VERSION); // version as a string
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    //for RETINA display
    glfwGetFramebufferSize( surf->handle(), &retina_width, &retina_height );

    return true;
}

void renderScene()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glClearColor( .1, .1, .16, 1.0 );
    glViewport( 0, 0, retina_width, retina_height );

    glUseProgram( shaderProgram );

    glBindVertexArray( VAO );
    glDrawElements( GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0 );
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
    surf->downlink();
    IXT::final_downlink( NULL, NULL, 0, NULL, NULL );
}

int main(int argc, char* argv[]) {
    IXT::initial_uplink( argc, argv, 0, NULL, NULL );

    surf = new Surface{ "Lab-IV", Crd2{}, Vec2{ Env::w<2.>() }, SURFACE_STYLE_LIQUID };

    if (!initOpenGLWindow()) {
        IXT::final_downlink( argc, argv, 0, NULL, NULL );
        return 1;
    }

    initObjects();

    shaderProgram = initBasicShader( SOURCE_DIR"/shaders/shader.vert", SOURCE_DIR"/shaders/shader.frag" );

    glUseProgram( shaderProgram );
    GLint shader_vrtx_off = glGetUniformLocation( shaderProgram, "vrtx_off" );

    Vec2 vrtx_off = {};

    while( !surf->down( SurfKey::ESC ) ) {
        static Ticker ticker;
        auto elapsed = ticker.lap() * 60.0;

        if( surf->down( SurfKey::LMB ) ) {
            vrtx_off = surf->ptr_v();
        } else {
            vrtx_off.x += elapsed * ( surf->down( SurfKey::D ) - surf->down( SurfKey::A ) ) * std::atof( argv[ 1 ] );
            vrtx_off.y += elapsed * ( surf->down( SurfKey::W ) - surf->down( SurfKey::S ) ) * std::atof( argv[ 1 ] );
        }

        glPolygonMode( GL_FRONT_AND_BACK, surf->down( SurfKey::SPACE ) ? GL_LINE : GL_FILL );

        glUniform2f( shader_vrtx_off, vrtx_off.x, vrtx_off.y );

        renderScene();
        glfwSwapBuffers( surf->handle() );
    }

    cleanup();

    return 0;
}

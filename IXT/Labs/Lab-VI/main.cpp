//
//  main.cpp
//  OpenGL_Shader_Example_step1
//
//  Created by CGIS on 30/11/15.
//  Copyright © 2015 CGIS. All rights reserved.
//

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) int NvOptimusEnablement = 1;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

#ifdef __cplusplus
}
#endif

#include <IXT/ring-0.hpp>
using namespace IXT;

#include "Camera.hpp"
#include "Shader.hpp"
#include "Model3D.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

int glWindowWidth = 640;
int glWindowHeight = 480;
int retina_width, retina_height;

Surface*   surf = NULL;
Renderer3* rend = NULL;


GLfloat vrtx_data[] = {
	// first triangle
	-5.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	5.0f,0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 8.0f, 0.0f, 0.5f, 1.0f,
	// second triangle
	0.1f, 8.0f, 0.0f, 1.0f, 0.0f,
	5.1f, 0.0f, 0.0f, 0.5f, 1.0f,
	10.1f, 8.0f, 0.0f, 0.0f, 0.0f
};
GLuint vrtx_idx[] = {
	0,1,2,
	3,4,5
};

GLuint VBO, EBO, VAO, VBO2;


void initObjects() {
    glGenVertexArrays( 1, &VAO );
    glBindVertexArray( VAO );

    glGenBuffers( 1, &VBO );
    glBindBuffer( GL_ARRAY_BUFFER, VBO );
    glBufferData( GL_ARRAY_BUFFER, sizeof( vrtx_data ), vrtx_data, GL_STATIC_DRAW );
    
    glGenBuffers( 1, &EBO );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, EBO );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( vrtx_idx ), vrtx_idx, GL_STATIC_DRAW );

    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof( GLfloat ), ( GLvoid* )0 );
    glEnableVertexAttribArray( 0 );

    glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof( GLfloat ), ( GLvoid* )( 3 * sizeof( GLfloat ) ) );
    glEnableVertexAttribArray( 2 );

    glBindVertexArray( 0 );
}

bool initOpenGLWindow() {
    //for RETINA display
    glfwGetFramebufferSize( surf->handle(), &retina_width, &retina_height );

    return true;
}



int main(int argc, char* argv[] ) {
    IXT::initial_uplink( argc, argv, 0, NULL, NULL );

    surf = new Surface{ "Lab-VI", Crd2{}, Vec2{ 8e2, 6e2 }, SURFACE_THREAD_ACROSS, SURFACE_STYLE_LIQUID };
    rend = new Renderer3{ *surf };

    if (!initOpenGLWindow()) {
        surf->downlink();
        IXT::final_downlink( argc, argv, 0, NULL, NULL );
        return 1;
    }

    glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LESS );
	glEnable( GL_CULL_FACE ); 
	glCullFace( GL_BACK );
	glFrontFace( GL_CCW );

    gps::Camera cam(glm::vec3(0.0f, 5.0f, 15.0f), glm::vec3(0.0f, 5.0f, -10.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    initObjects();

    Shader3 shader_vert{ SOURCE_DIR"/shaders/shader.vert", SHADER3_PHASE_VERTEX };
    Shader3 shader_frag{ SOURCE_DIR"/shaders/shader.frag", SHADER3_PHASE_FRAGMENT };

    ShaderPipe3 shader_pipe{ shader_vert, shader_frag };
    shader_pipe.uplink();

    Uniform3< glm::mat4 > model{ shader_pipe, "model", glm::mat4( 1.0f ) };
    Uniform3< glm::mat4 > view{ shader_pipe, "view", cam.getViewMatrix() };
    Uniform3< glm::mat4 > proj{ shader_pipe, "projection", glm::perspective(glm::radians(55.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f) };
    model.uplink(); view.uplink(); proj.uplink();

    gps::Model3D mod3D;

    auto text = mod3D.ReadTextureFromFile( SOURCE_DIR"/textures/hazard2.png" );

    while( !surf->down( SurfKey::ESC ) ) {
        static Ticker ticker;
        auto elapsed = ticker.lap() * 60.0;

        //glPolygonMode( GL_FRONT_AND_BACK, surf->down( SurfKey::SPACE ) ? GL_LINE : GL_FILL );

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glClearColor( .1, .1, .16, 1.0 );
        glViewport( 0, 0, retina_width, retina_height );

        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(shader_pipe, "diffuse_texture"), 0);
        glBindTexture(GL_TEXTURE_2D, text);

        glBindVertexArray( VAO );
        glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );

        glfwSwapBuffers( surf->handle() );
    }

    surf->downlink();
    IXT::final_downlink( argc, argv, 0, NULL, NULL );

    return 0;
}

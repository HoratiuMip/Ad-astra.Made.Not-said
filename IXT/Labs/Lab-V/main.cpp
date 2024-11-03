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
	-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, //0
	0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, //1
	0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, //2
	-0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, //3
	-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, //4
	0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, //5
	0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 1.0f, //6
	-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, //7
	-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, //8   0'
	0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, //9   1'
	0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, //10   2'
	-0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, //11   3'
	-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, //12   4'
	0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 1.0f, //13   5'
	0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, //14   6'
	-0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, //15   7'
	-0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, //16   0''
	0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, //17   1''
	0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 1.0f, //18   2''
	-0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 1.0f, //19   3''
	-0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f, //20   4''
	0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f, //21   5''
	0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 1.0f, //22   6''
	-0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 1.0f, //23   7''
};

GLuint vrtx_idx[] = {
	0, 1, 2, 0, 2, 3, // bottom plane triangles
	8, 9, 5, 8, 5, 4, // back plane triangles
	17, 10, 6, 17, 6, 13, // right plane triangles
	16, 12, 7, 16, 7, 11, // left plane triangles
	20, 21, 14, 20, 14, 15, // top plane triangles
	19, 18, 22, 19, 22, 23 // front plane triangles
};

GLuint VBO, EBO, VAO, VBO2;


void initObjects() {
    glGenVertexArrays( 1, &VAO );
    glBindVertexArray( VAO );

    glGenBuffers( 1, &VBO );
    glBindBuffer( GL_ARRAY_BUFFER, VBO );
    glBufferData( GL_ARRAY_BUFFER, sizeof( vrtx_data ), vrtx_data, GL_STATIC_DRAW );

    glGenBuffers( 1, &VBO2 );
    glBindBuffer( GL_ARRAY_BUFFER, VBO2 );
    glBufferData( GL_ARRAY_BUFFER, sizeof( vrtx_data ), vrtx_data, GL_STATIC_DRAW );

    glGenBuffers( 1, &EBO );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, EBO );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( vrtx_idx ), vrtx_idx, GL_STATIC_DRAW );

    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof( GLfloat ), ( GLvoid* )0 );
    glEnableVertexAttribArray( 0 );

    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof( GLfloat ), ( GLvoid* )( 3 * sizeof( GLfloat ) ) );
    glEnableVertexAttribArray( 1 );

    glBindVertexArray( 0 );
}

bool initOpenGLWindow() {
    //for RETINA display
    glfwGetFramebufferSize( surf->handle(), &retina_width, &retina_height );

    return true;
}



int main(int argc, char* argv[] ) {
    IXT::initial_uplink( argc, argv, 0, NULL, NULL );

    surf = new Surface{ "Lab-IV", Crd2{}, Vec2{ Env::w<2.>() }, SURFACE_THREAD_ACROSS, SURFACE_STYLE_LIQUID };
    rend = new Renderer3{ *surf };

    if (!initOpenGLWindow()) {
        surf->downlink();
        IXT::final_downlink( argc, argv, 0, NULL, NULL );
        return 1;
    }

    initObjects();

    Shader3 shader_vert{ SOURCE_DIR"/shaders/shader.vert", SHADER3_PHASE_VERTEX };
    Shader3 shader_frag{ SOURCE_DIR"/shaders/shader.frag", SHADER3_PHASE_FRAGMENT };

    ShaderPipe3 shader_pipe{ shader_vert, shader_frag };

    shader_pipe.uplink();
    
    Uniform3< glm::mat4 > model{ shader_pipe, "tmx_model", glm::mat4( 1.0f ) };
    model.uplink(); 

    Uniform3< glm::mat4 > view{ 
        shader_pipe, "tmx_view", 
        glm::lookAt( glm::vec3( 0, 0, 5 ), glm::vec3(0, 0, -10 ), glm::vec3( 0, 1, 0 ) ) 
    };
    Uniform3< glm::mat4 > proj{ 
        shader_pipe, "tmx_proj", 
        glm::perspective( 70.0f, ( float )surf->width() / ( float )surf->height(), .1f, 1000.0f )
    };
    view.uplink(); proj.uplink();

    float angles[] = { 0, 0 };
    float dist     = 1.0;

    while( !surf->down( SurfKey::ESC ) ) {
        static Ticker ticker;
        auto elapsed = ticker.lap() * 60.0;

        if( surf->down( SurfKey::LMB ) ) {
            angles[ 0 ] += elapsed * .05;
        }
        if( surf->down( SurfKey::RMB ) ) {
            angles[ 1 ] -= elapsed * .05;
        }
        if( surf->down( SurfKey::LEFT ) ) {
            dist -= elapsed * .05;
        }
        if( surf->down( SurfKey::RIGHT ) ) {
            dist += elapsed * .05;
        }

        glPolygonMode( GL_FRONT_AND_BACK, surf->down( SurfKey::SPACE ) ? GL_LINE : GL_FILL );

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glClearColor( .1, .1, .16, 1.0 );
        glViewport( 0, 0, retina_width, retina_height );

        glBindVertexArray( VAO );

        model.get() = 
            glm::translate( glm::mat4( 1.0f ), glm::vec3( dist, .0, .0 ) )
            *
            glm::rotate( glm::mat4( 1.0f ), angles[ 0 ], glm::vec3( 0, 1, 0 ) );
        model.uplink();

        glDrawElements( GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0 );

        model.get() = 
            glm::translate( glm::mat4( 1.0f ), glm::vec3( -dist, .0, .0 ) )
            *
            glm::rotate( glm::mat4( 1.0f ), angles[ 1 ], glm::vec3( 0, 1, 0 ) );
        model.uplink();
        
        glDrawElements( GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0 );

        glfwSwapBuffers( surf->handle() );
    }

    surf->downlink();
    IXT::final_downlink( argc, argv, 0, NULL, NULL );

    return 0;
}

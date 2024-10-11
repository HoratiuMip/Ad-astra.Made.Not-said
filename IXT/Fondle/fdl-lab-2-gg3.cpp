//
//  main.cpp
//  Check Max Supported OpenGL version (on non-apple devices)
//
//  Created by CGIS on 25/09/2023.
//  Copyright 2023 CGIS. All rights reserved.
//

#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <iostream>
#include <glm/glm.hpp>

const int GL_WINDOW_WIDTH = 640;
const int GL_WINDOW_HEIGHT = 480;

GLFWwindow* glWindow = NULL;

glm::vec4 clearColor{ 1.0f, 1.0f, 1.0f, 1.0f };

bool initOpenGLWindow()
{
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return false;
    }

    glWindow = glfwCreateWindow(GL_WINDOW_WIDTH, GL_WINDOW_HEIGHT, "Hello Window", NULL, NULL);
    if (!glWindow) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(glWindow);

#if !defined (__APPLE__)
    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    glewInit();
#endif

    // get version info
    const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
    const GLubyte* version = glGetString(GL_VERSION); // version as a string
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    return true;
}

void cleanup() {

    glfwDestroyWindow(glWindow);
    //close GL context and any other GLFW resources
    glfwTerminate();
}

void renderScene() {
    //TODO drawing code
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

int main(int argc, const char* argv[]) {

    if (!initOpenGLWindow()) {

        return 1;
    }

    while (!glfwWindowShouldClose(glWindow)) {

        renderScene();

        glfwSwapBuffers(glWindow);
        glfwPollEvents();
    }

    cleanup();
    return 0;
}

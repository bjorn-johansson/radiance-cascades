/*
 * A C++ framework for OpenGL programming in TNM046 for MT1
 *
 * This is a small, limited framework, designed to be easy to use for students in an introductory
 * computer graphics course in the first year of a M Sc curriculum. It uses custom code for some
 * things that are better solved by external libraries like GLM, but the emphasis is on simplicity
 * andreadability, not generality.
 *
 * For the window management, GLFW 3.x is used for convenience.
 * The framework should work in Windows, MacOS X and Linux.
 * GLEW is used for handling platform specific OpenGL extensions.
 * This code depends only on GLFW, GLEW, and OpenGL libraries.
 * OpenGL 3.3 or higher is required.
 *
 * Authors: Stefan Gustavson (stegu@itn.liu.se) 2013-2015
 *          Martin Falk (martin.falk@liu.se) 2021
 *
 * This code is in the public domain.
 */

#if defined(WIN32) && !defined(_USE_MATH_DEFINES)
#define _USE_MATH_DEFINES
#endif

// File and console I/O for logging and error reporting
#include <iostream>
// Math header for trigonometric functions
#include <cmath>

// glew provides easy access to advanced OpenGL functions and extensions
#include <GL/glew.h>

// GLFW 3.x, to handle the OpenGL window
#include <GLFW/glfw3.h>

#include "Utilities.hpp"
#include "TriangleSoup.hpp"
#include <glm.hpp>
#include "Shader.hpp"
#include "Tracy.hpp"



/*
 * main(int argc, char* argv[]) - the standard C++ entry point for the program
 */
int main(int, char*[]) {

    // Initialise GLFW
    glfwInit();
    
    const GLFWvidmode* vidmode;  // GLFW struct to hold information about the display
    // Determine the desktop size
    vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    // Make sure we are getting a GL context of at least version 4.3 for compute shader capability
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // Enable the OpenGL core profile
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Open a square window (aspect 1:1) to fill half the screen height
    GLFWwindow* window =
        glfwCreateWindow(1920, 1080, "GLprimer", nullptr, nullptr);
    if (!window) {
        std::cout << "Unable to open window. Terminating.\n";
        glfwTerminate();  // No window was opened, so we can't continue in any useful way
        return -1;
    }
    
    // Make the newly created window the "current context" for OpenGL
    // (This step is strictly required or things will simply not work)
    glfwMakeContextCurrent(window);

    // Initialize glew
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cerr << "Error: " << glewGetErrorString(err) << "\n";
        glfwTerminate();
        return -1;
    }

    // Show some useful information on the GL context
    std::cout << "GL vendor:       " << glGetString(GL_VENDOR)
              << "\nGL renderer:     " << glGetString(GL_RENDERER)
              << "\nGL version:      " << glGetString(GL_VERSION)
              << "\nDesktop size:    " << vidmode->width << " x " << vidmode->height << "\n";

    TriangleSoup soup;
    soup.createTriangle();

    // Get window size. It may start out different from the requested size and
    // will change if the user resizes the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    // Set viewport. This is the pixel rectangle we want to draw into
    glViewport(0, 0, width, height);  // The entire window

    glfwSwapInterval(0);  // Do not wait for screen refresh between frames



    //----------------------------------cascade 0 setup---------------------------------------------


    //this bitmap is the scene. it contains the wall/emissive data that the rays march against.
    GLuint bitmapTex;
    glGenTextures(1, &bitmapTex);
    glActiveTexture(GL_TEXTURE0); //binding = 0
    glBindTexture(GL_TEXTURE_2D, bitmapTex);
    const int worldWidth = 128;
    const int worldHeight = 128;
    std::vector<uint8_t> bitmapData(worldWidth * worldHeight, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, worldWidth, worldHeight, 0,
        GL_RED_INTEGER, GL_UNSIGNED_BYTE, &bitmapData);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    //material atlas. Each texel has a color and a brightness, thats all the data materials contain for now.
    GLuint materialAtlas;
    glGenTextures(1, &materialAtlas);
    glActiveTexture(GL_TEXTURE1); //binding = 1
    glBindTexture(GL_TEXTURE_2D, materialAtlas);
    std::vector<glm::vec4> atlasData(8*8, glm::vec4(0.0f));
    Shader c0("shaders/cascade0.comp");
    glUseProgram(c0.id());
    
    


    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        ZoneScoped;
        // Set the clear color to a dark gray (RGBA)
        glClearColor(0.3f, 0.3f, 0.3f, 0.0f);
        // Clear the color and depth buffers for drawing
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



        
        /* ---- Rendering code should go here ---- */
        soup.render();





        
        
        // Swap buffers, display the image and prepare for next frame
        glfwSwapBuffers(window);
        // Poll events (read keyboard and mouse input)
        glfwPollEvents();
        // Exit if the ESC key is pressed (and also if the window is closed)
        if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
        FrameMark; // Marks the end of a frame for Tracy
    }

    // Close the OpenGL window and terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
}

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
//openGL maths library
#include <glm/glm.hpp>

// GLFW 3.x, to handle the OpenGL window
#include <GLFW/glfw3.h>

#include "Utilities.hpp"
#include "TriangleSoup.hpp"

#include "Shader.hpp"
#include "Tracy.hpp"

void SetupCascade() {
    
}

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
        glfwCreateWindow(1080, 1080, "GLprimer", nullptr, nullptr);
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












    //quad for writing to screen:
    float quadVertices[] = {
        // positions   // texCoords
        -0.9f, -0.9f,    0.f, 0.f,
         0.9f, -0.9f,    1.f, 0.f,
        -0.9f,  0.9f,    0.f, 1.f,
         0.9f,  0.9f,    1.f, 1.f,
    };

    GLuint quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    // TexCoords attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
    

    //----------------------------------cascade 0 setup---------------------------------------------

    /*
        setup:
        * setup bitmap from tga texture - black = empty, green = light, red = wall
        * setup cascades 0-3, (use 1 file with different preprocessing directives?)
        
        runtime passes:
        1. update bitmap with world information
        2. cascades
        3. combine cascades w/ interpolation
        
        
    */
    std::cout << "Bitmap setup.\n";
    //this bitmap is the scene. it contains the wall/emissive data that the rays march against.
    GLuint bitmapTex;
    glGenTextures(1, &bitmapTex);
    glActiveTexture(GL_TEXTURE0); //binding = 0
    glBindTexture(GL_TEXTURE_2D, bitmapTex);
    
    constexpr int worldWidth = 128;
    constexpr int worldHeight = 128;
    std::vector<uint8_t> bitmapData(worldWidth * worldHeight, 0);
    
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8UI, worldWidth, worldHeight);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0,0, worldWidth, worldHeight,
        GL_RED_INTEGER,GL_UNSIGNED_BYTE, bitmapData.data());
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    




    std::cout << "Material atlas setup.\n";
    //emissive material atlas. Each texel has a color and a brightness, that's all that materials contain for now.
    GLuint materialAtlas;
    glGenTextures(1, &materialAtlas);
    glActiveTexture(GL_TEXTURE1); //binding = 1
    glBindTexture(GL_TEXTURE_2D, materialAtlas);
    
    constexpr int atlasSide = 8;
    std::vector atlasData(atlasSide * atlasSide, glm::vec4(0.0f));
    
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, 8,8);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0,0,atlasSide,atlasSide,
        GL_RGBA, GL_FLOAT, atlasData.data());
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    



    std::cout << "cascade 0 setup.\n";
    //cascade 0
    GLuint c0Image;
    glGenTextures(1, &c0Image);
    glBindTexture(GL_TEXTURE_2D, c0Image);

    //64x64 probes × 2x2 texels = 128x128 output
    const int c0Width = 256;
    const int c0Height = 256;
    
    //allocate storage gor cascade
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, c0Width, c0Height);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //bind as an image
    glBindImageTexture(2, c0Image, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F); // Binding = 2


    //setup c0 comp shader
    Shader c0("shaders/cascade0.comp");
    glUseProgram(c0.id());
    glDispatchCompute(32, 32, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);









    
    
    std::cout << "cascade 1 setup.\n";
    //cascade 1
    GLuint c1Image;
    glGenTextures(1, &c1Image);
    glBindTexture(GL_TEXTURE_2D, c1Image);

    //32x32 probes × 4x4 texels = 64x64 rays
    const int c1Width = 256;
    const int c1Height = 256;
    
    //allocate storage gor cascade
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, c1Width, c1Height);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //bind as an image
    glBindImageTexture(2, c1Image, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F); // Binding = 2

    
    Shader c1("shaders/cascade1.comp");
    glUseProgram(c1.id());
    glDispatchCompute(32, 32, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    
    

    std::cout << "cascade 2 setup.\n";
    //cascade 1
    GLuint c2Image;
    glGenTextures(1, &c2Image);
    glBindTexture(GL_TEXTURE_2D, c2Image);

    //32x32 probes × 4x4 texels = 64x64 rays
    const int c2Width = 256;
    const int c2Height = 256;
    
    //allocate storage gor cascade
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, c2Width, c2Height);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //bind as an image
    glBindImageTexture(2, c2Image, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F); // Binding = 2

    
    Shader c2("shaders/cascade2.comp");
    glUseProgram(c2.id());
    glDispatchCompute(32, 32, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);



    

    std::cout << "cascade 2 setup.\n";
    //cascade 1
    GLuint c3Image;
    glGenTextures(1, &c3Image);
    glBindTexture(GL_TEXTURE_2D, c3Image);

    //32x32 probes × 4x4 texels = 64x64 rays
    const int c3Width = 256;
    const int c3Height = 256;
    
    //allocate storage gor cascade
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, c3Width, c3Height);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //bind as an image
    glBindImageTexture(2, c3Image, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F); // Binding = 2

    
    Shader c3("shaders/cascade3.comp");
    glUseProgram(c3.id());
    glDispatchCompute(32, 32, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);







    
    //setup simple write to screen
    Shader screenWrite("shaders/ScreenWrite.vert", "shaders/ScreenWrite.frag");
    
    glUseProgram(screenWrite.id());
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, c2Image);
    
    GLint loc = glGetUniformLocation(screenWrite.id(), "_Texture");
    glUniform1i(loc, 3);  //tell sampler "_Texture" to use texture unit 3
    
    
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        ZoneScoped;
        // Set the clear color to a dark gray (RGBA)
        glClearColor(0.3f, 0.3f, 0.3f, 0.0f);
        // Clear the color and depth buffers for drawing
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



        
        /* ---- Rendering code should go here ---- */
        //soup.render();

        //glUseProgram(c0.id());
        //glDispatchCompute(64, 64, 1);
        //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        
        glUseProgram(screenWrite.id());
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, c3Image);
        
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
        
        
        util::displayFPS(window);
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

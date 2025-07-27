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
#include "CascadePreprocessor.hpp"

#include "Shader.hpp"
#include "Texture.hpp"
#include "Tracy.hpp"
#include "TracyOpenGL.hpp"
#include "../build/CascadePreprocessor.hpp"

constexpr int CASCADE_COUNT = 6;

int rayLengthMultiplier = 1;
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    rayLengthMultiplier += yoffset;
    if(rayLengthMultiplier < 1) rayLengthMultiplier = 1;
    if (rayLengthMultiplier > 200) rayLengthMultiplier = 200;
    std::cout << "mod: " << rayLengthMultiplier <<"\n";
}

int main(int, char*[]) {

    //This generates N cascade shader files and adds "#Define CASCADE_LEVEL N" when it hits the line #PreprocesCascadeLevel
    //this is to get loop unrolling in my raymarching without having to manage 7+ files where the only difference is 1 value.
    CascadePreprocessor::preprocessCascades(CASCADE_COUNT);
    
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
        glfwCreateWindow(1024, 1024, "GLprimer", nullptr, nullptr);
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



    constexpr float SQUASH = 1.f; //scalar for squashing the quad
    //quad for writing to screen:
    float quadVertices[] = {
        -SQUASH, -SQUASH,    0.f, 0.f,
         SQUASH, -SQUASH,    1.f, 0.f,
        -SQUASH,  SQUASH,    0.f, 1.f,
         SQUASH,  SQUASH,    1.f, 1.f,
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
    

    //----------------------------------Scene & Material Setup--------------------------------------
    //this bitmap is the scene. it contains the wall/emissive data that the rays march against.
    
    std::cout << "Bitmap setup.\n";
    
    GLuint bitmapTex;
    glGenTextures(1, &bitmapTex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bitmapTex);
    
    constexpr int worldWidth = 1024;
    constexpr int worldHeight = 1024;
    
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8UI, worldWidth, worldHeight);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glBindImageTexture(0, bitmapTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8UI); //binding to image unit 0
    GLuint zero = 0;
    glClearTexImage(bitmapTex, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, &zero);

    
    Texture worldTex("Textures/SceneTexture1.tga"); //selects what scene to use
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, worldTex.id());
    
    Shader bitmapGenerator("shaders/GenerateSceneBitmap.comp");
    
    glUseProgram(bitmapGenerator.id());
    glUniform1i(glGetUniformLocation(bitmapGenerator.id(), "sceneTexture"), 10);
    
    glDispatchCompute(128, 128, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


    
    //mats are not implemented yet.
    //emissive material atlas. Each texel has a color and a brightness, that's all that materials contain for now.
    
    std::cout << "Material atlas setup.\n";
    
    GLuint materialAtlas;
    glGenTextures(1, &materialAtlas);
    glActiveTexture(GL_TEXTURE1); //binding to texture unit 1
    glBindTexture(GL_TEXTURE_2D, materialAtlas);
    
    constexpr int atlasSide = 8;
    std::vector atlasData(atlasSide * atlasSide, glm::vec4(0,1,0,1));
    atlasData.at(0) = glm::vec4(0,0,0,0);
    
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, 8,8);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0,0,atlasSide,atlasSide,
        GL_RGBA, GL_FLOAT, atlasData.data());
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    




    //----------------------------------Cascade Setup--------------------------------------
    
    std::cout << "cascade image array setup.\n";
    GLuint cascadeTextures;
    glGenTextures(1, &cascadeTextures);
    glActiveTexture(GL_TEXTURE2); //binding to texture unit 2
    glBindTexture(GL_TEXTURE_2D_ARRAY, cascadeTextures);
    
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA16F, 1024, 1024, 7);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //bind to image unit 2
    glBindImageTexture(2, cascadeTextures, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);

    

    //rebind bitmap to TU 0 for sampling. this shouldn't be necessary as bindings are global,
    //but had issues where the binding got lost and the scenetexture was read instead. I will look to remove this later.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bitmapTex);
    
    Shader cascades[CASCADE_COUNT] = {
        Shader("shaders/generated/Cascade0.comp"),
        Shader("shaders/generated/Cascade1.comp"),
        Shader("shaders/generated/Cascade2.comp"),
        Shader("shaders/generated/Cascade3.comp"),
        Shader("shaders/generated/Cascade4.comp"),
        Shader("shaders/generated/Cascade5.comp"),
        //Shader("shaders/generated/Cascade6.comp"),
    };

    GLint rayLMLoc[CASCADE_COUNT];
    for(int i = 0; i < CASCADE_COUNT; i++) {
        glUseProgram(cascades[i].id());
        glUniform1i(glGetUniformLocation(cascades[i].id(), "_bitmapTexture"), 0);
        rayLMLoc[i] = glGetUniformLocation(cascades[i].id(), "_RayLengthMultiplier");
        glUniform1i(rayLMLoc[i], rayLengthMultiplier);
    }

    
    //-----------------------------------MERGE CASCADES---------------------------------------------
    //RC is typically memory-bound, so we reuse the textures at the cost of having to merge 1 layer at a time.
    //this is prime space for profiling and seeing the difference.
    std::cout << "merge setup.\n";
    Shader merge("shaders/MergeCascades.comp");
    
    glUseProgram(merge.id());
    glUniform1i(glGetUniformLocation(merge.id(), "_cascadeSamplers"), 2);
    GLint mergeLoc = glGetUniformLocation(merge.id(), "_Merge");
    glUniform1i(mergeLoc, 1);

    GLint _sourceLaterIndexLoc = glGetUniformLocation(merge.id(), "_sourceLayerIndex");
    for(int i = 6; i > 0; i--) {
        glUniform1i(_sourceLaterIndexLoc, i);
        glDispatchCompute(64, 64, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }


    //-----------------------------------WRITE TO SCREEN--------------------------------------------  
    

    Shader screenWrite("shaders/ScreenWrite.vert", "shaders/ScreenWrite.frag");
    glUseProgram(screenWrite.id());
    GLint swLayerLoc = glGetUniformLocation(screenWrite.id(), "_Layer");
    glUniform1i(swLayerLoc, 0);
    GLint swInterpLoc = glGetUniformLocation(screenWrite.id(), "_Interpolate");
    glUniform1i(swInterpLoc, 1);
    GLint swUVLoc = glGetUniformLocation(screenWrite.id(), "_ProbeUV");
    glUniform1i(swUVLoc, 0);

    bool applyMerge = true;
    bool interpolate = true;
    bool probeUV = false;
    int highestLayer = 6;
    
    int defaultInputPauseTime = 1000;
    int inputPauseTime = 150;
    int timePaused = 0;

    
    
    //----------------------------------------MAIN LOOP---------------------------------------------
    TracyGpuContext;
    while (!glfwWindowShouldClose(window)) {
        ZoneScoped;
        // Set the clear color to a dark gray (RGBA)
        glClearColor(0.3f, 0.3f, 0.3f, 0.0f);
        // Clear the color and depth buffers for drawing
        glfwSetScrollCallback(window, ScrollCallback);
        
        if (!glfwGetKey(window, GLFW_KEY_SPACE)) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            //-------------------------------GATHER RAYS------------------------------------------------
           
            {
                TracyGpuZone("Cascade Dispatches");

                //glUniform1i(rayLMLoc, rayLengthMultiplier);
                for(int i = 0; i < CASCADE_COUNT; i++) {
                    {
                        TracyGpuZone("Cascade");
                        std::string msg = "c" + std::to_string(i);
                        TracyMessage(msg.c_str(), msg.length());
                        glUseProgram(cascades[i].id());
                        glDispatchCompute(64, 64, 1);
                    }
                }
            }

            {
                TracyGpuZone("Merge Dispatches");
                //RC is typically memory-bound, so i reuse the textures.
                //- Prime space for profiling and seeing the difference.
                glUseProgram(merge.id());
                glUniform1i(glGetUniformLocation(merge.id(), "_cascadeSamplers"), 2);
                for(int i = highestLayer; i > 0; i--) {
                    {
                        TracyGpuZone("Merge Dispatch");
                        std::string msg = "c" + std::to_string(i);
                        TracyMessage(msg.c_str(), msg.length());
                        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                        glUniform1i(glGetUniformLocation(merge.id(), "_sourceLayerIndex"), i);
                        glDispatchCompute(64, 64, 1);
                    }
                }
            }

            {
                TracyGpuZone("ScreenWrite Dispatch");
                //-----------------------------Sample Probes and write to screen----------------------------
        
                glUseProgram(screenWrite.id());
                glBindVertexArray(quadVAO);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                glBindVertexArray(0);
            }
        }
        TracyGpuCollect; //GPU work done
        
        util::displayFPS(window);
        // Swap buffers, display the image and prepare for next frame
        glfwSwapBuffers(window);
        // Poll events (read keyboard and mouse input)
        glfwPollEvents();
        // Exit if the ESC key is pressed (and also if the window is closed)
        if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
            /*
        if(timePaused != 0) {
            if(timePaused >= inputPauseTime) {
                timePaused = 0;
            }else timePaused++;
        }else {
            if (glfwGetKey(window, GLFW_KEY_I)) {
                glUseProgram(screenWrite.id());
                glUniform1i(swInterpLoc, static_cast<int>(interpolate));
                interpolate = !interpolate;
                timePaused++;
            }
            if (glfwGetKey(window, GLFW_KEY_U)) {
                glUseProgram(screenWrite.id());
                glUniform1i(swUVLoc, static_cast<int>(probeUV));
                probeUV = !probeUV;
                timePaused++;
            }
            if (glfwGetKey(window, GLFW_KEY_M)) {
                glUseProgram(merge.id());
                glUniform1i(mergeLoc, static_cast<int>(applyMerge));
                applyMerge = !applyMerge;
                timePaused++;
            }
            if (glfwGetKey(window, GLFW_KEY_0)) {
                glUseProgram(screenWrite.id());
                glUniform1i(swLayerLoc, 0);
                timePaused++;
                inputPauseTime = defaultInputPauseTime;
            }
            if (glfwGetKey(window, GLFW_KEY_1)) {
                glUseProgram(screenWrite.id());
                glUniform1i(swLayerLoc, 1);
                timePaused++;
                inputPauseTime = defaultInputPauseTime;
            }
            if (glfwGetKey(window, GLFW_KEY_2)) {
                glUseProgram(screenWrite.id());
                glUniform1i(swLayerLoc, 2);
                timePaused++;
                inputPauseTime = defaultInputPauseTime;
            }
            if (glfwGetKey(window, GLFW_KEY_3)) {
                glUseProgram(screenWrite.id());
                glUniform1i(swLayerLoc, 3);
                timePaused++;
                inputPauseTime = defaultInputPauseTime;
            }
            if (glfwGetKey(window, GLFW_KEY_4)) {
                glUseProgram(screenWrite.id());
                glUniform1i(swLayerLoc, 4);
                timePaused++;
                inputPauseTime = defaultInputPauseTime;
            }
            if (glfwGetKey(window, GLFW_KEY_5)) {
                glUseProgram(screenWrite.id());
                glUniform1i(swLayerLoc, 5);
                timePaused++;
                inputPauseTime = defaultInputPauseTime;
            }
            if (glfwGetKey(window, GLFW_KEY_6)) {
                glUseProgram(screenWrite.id());
                glUniform1i(swLayerLoc, 6);
                timePaused++;
                inputPauseTime = defaultInputPauseTime;
            }
            if(glfwGetKey(window, GLFW_KEY_F1)) {
                if(highestLayer > 0)
                    highestLayer--;
                timePaused++;
                inputPauseTime = defaultInputPauseTime;
            }
            if(glfwGetKey(window, GLFW_KEY_F2)) {
                if(highestLayer < 6)
                    highestLayer++;
                timePaused++;
                inputPauseTime = defaultInputPauseTime;
            }
        }
        */
                
        FrameMark; // Marks the end of a frame for Tracy
        
    }

    // Close the OpenGL window and terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
}

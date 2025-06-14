/*
 * A class to load and compile GLSL shaders from files.
 *
 * Usage: call createShader() to load and compile a program object
 * or use the constructor with two filenames.
 * Call glUseProgram() with the public member programID as argument.
 *
 * Authors: Stefan Gustavson (stegu@itn.liu.se) 2014
 *          Martin Falk (martin.falk@liu.se) 2021
 *
 * This code is in the public domain.
 */
#pragma once

#include <GLFW/glfw3.h>
#include <string>

class Shader {
public:
    // Argument-less constructor. Creates an invalid shader program.
    Shader();

    // Constructor to create, load and compile a Shader program in one blow.
    Shader(const std::string& vertexshaderfile, const std::string& fragmentshaderfile);
    // Additional constructor just to allow for compute shaders.
    Shader(const std::string& computeshaderfile);

    // Destructor
    ~Shader();

    // createShader() - create, load, compile and link the GLSL shader objects.
    void createShader(const std::string& vertexshaderfile, const std::string& fragmentshaderfile);
    void createComputeShader(const std::string& computeshaderfile);

    GLuint id() const;

private:
    GLuint programID_;
};

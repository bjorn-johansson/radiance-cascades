#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

//this should maybe be made more generic but i just need it for loop unrolling in my cascades.
//if i don't get a perf increase from this i swear.
namespace CascadePreprocessor {

    constexpr const char* CASCADE_SHADER = "shaders/Cascade.generate";
    constexpr const char* OUTPUT_DIR = "shaders/generated/";

    inline void preprocessShaders(int cascadeCount) {

        std::ifstream templateFile(CASCADE_SHADER);
        if (!templateFile.is_open()) {
            std::cerr << "ERROR: Failed to open template shader: " << CASCADE_SHADER << "\n";
            return;
        }

        std::stringstream buffer;
        buffer << templateFile.rdbuf();
        std::string templateContent = buffer.str();
        std::filesystem::create_directories(OUTPUT_DIR);

        for (int c = 0; c < cascadeCount; ++c) {
            std::ostringstream outputFilename;
            outputFilename << OUTPUT_DIR << "Cascade" << c << ".comp";

            std::ofstream outFile(outputFilename.str());
            if (!outFile.is_open()) {
                std::cerr << "ERROR: Couldn't open file: " << outputFilename.str() << "\n";
                continue;
            }
            
            std::string defineDirective = "#define CASCADE_LEVEL " + std::to_string(c) + "\n";
            
            //walk the file to find the marker for where to insert directive.
            std::istringstream stream(templateContent);
            std::ostringstream modified;
            std::string line;
            bool directiveInserted = false;

            while(std::getline(stream, line)) {
                if(!directiveInserted && line.find("#PreprocesCascadeLevel") != std::string::npos){
                    line = defineDirective;
                    directiveInserted = true;
                }
                modified << line << "\n";
            }
            if(!directiveInserted)
                outFile << defineDirective; //if we cant find the marker, just place it at the top. - might interfere with opengls #version?
            outFile << modified.str();
            outFile.close();

            std::cout << "Preprocessor generated cascade file: " << outputFilename.str() << "\n";
        }
    }

}
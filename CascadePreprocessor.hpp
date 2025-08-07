#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>


/*
    This was a bit of an experiment to see if i gain any perf from loop unrolling in my cascades.
    As i raymarch a ton i expected some decent benefits from unrolling these DDA loops.
    Turns out, i only shave off ~0.2ms. Around 5%.
    All in all, less gain than i expected, but a very good learning experience.
    I need to get better at using the Nsight tools to find what actually limits my shader code.
*/
// this should maybe be made more generic but i just need it for loop unrolling in my cascades
// OpenGL doesn't support multiple different compile-time constant values in the same shader so multiple files are needed
namespace CascadePreprocessor {

    inline void preprocessCascades(const int cascadeCount) {

        std::ifstream templateFile("shaders/Cascade.comp");
        if (!templateFile.is_open()) {
            std::cerr << "ERROR: Failed to open template cascade shader!\n";
            return;
        }

        std::stringstream buffer;
        buffer << templateFile.rdbuf();
        std::string templateContent = buffer.str();
        std::filesystem::create_directories("shaders/generated/");

        for (int c = 0; c < cascadeCount; ++c) {
            std::ostringstream outputFilename;
            outputFilename << "shaders/generated/" << "Cascade" << c << ".comp";

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
                if(!directiveInserted && line.find("#PreprocessCascadeLevel") != std::string::npos){
                    directiveInserted = true;
                    
                    line = defineDirective; //add the define
                    modified << line << "\n";
                    
                    std::getline(stream, line); //empty the next line, should contain a dummy #define CASCADE_LEVEL -1
                    line = "\n";
                }
                modified << line << "\n";

            }
            if(!directiveInserted)
            {
                std::cout << "ERROR: Failed to find preprocessing directive location!\nMake sure #PreprocessCascadeLevel exists in Cascades.comp!";
                outFile.close();
            }
            else
            {
                outFile << modified.str();
                outFile.close();
            }

            std::cout << "Preprocessor generated cascade file: " << outputFilename.str() << "\n";
        }
    }

}
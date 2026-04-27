#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

static bool fileExists(const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}

std::string extractShader(std::string path){
    std::string text;
    std::ifstream file;
    std::stringstream stream;
    if (!fileExists(path)){
        std::cerr<<"File "<<path<<" doesnt exist!"<<std::endl;
        return "";
    }
    file.open(path);
    stream << file.rdbuf();
    file.close();
    text = stream.str();
    return text;
}
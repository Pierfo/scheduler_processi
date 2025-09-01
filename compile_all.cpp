#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

//Compila il programma. Il file eseguibile si troverà nella directory "build_main"
int main() {
    std::vector<std::string> v = {"insert_into_buffer", "remove_from_buffer", "monitor_buffer_level", "parasite", "main"};

    for(std::string name : v) {
        std::ofstream makefile {"CMakeLists.txt"};

        //Crea il file CMakeLists.txt
        makefile << "cmake_minimum_required(VERSION 3.6)" << std::endl << std::endl;
        makefile << "project(" << name << ")" << std::endl << std::endl;
        makefile << "set(CMAKE_CXX_FLAGS \"${CMAKE_CXX_FLAGS} -O2\")" << std::endl << std::endl;
        makefile << "set(CMAKE_CXX_FLAGS \"${CMAKE_CXX_FLAGS} -w\")" << std::endl << std::endl;
        makefile << "include_directories(include)" << std::endl << std::endl;
        makefile << "add_executable(" << name << " src/" << name << ".cpp)" << std::endl;
        
        makefile.close();

        std::ofstream script {"script.sh"};

        //Crea lo script per la compilazione
        script << "#!/bin/bash" << std::endl;
        script << "cmake -S . -B build_" << name <<std::endl;
        script << "cd build_" << name << std::endl;
        script << "make -j2" << std::endl;
        script << "cd ..";

        script.close();

        system("chmod +x script.sh");
        std::cout << std::endl << std::endl << "conpilazione di " << name << std::endl << std::endl;
        system("./script.sh");
        system("rm script.sh");
    }

    system("rm CMakeLists.txt");

    return 0;
}
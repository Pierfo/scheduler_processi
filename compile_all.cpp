#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#define FILE_SIZE 100000000

void create_file(const char * name) {
    errno = 0;
    int fd = open(name, O_CREAT | O_EXCL | O_RDWR, 0666);

    if(errno == EEXIST) {
        close(fd);
        return;
    }

    if(fd == -1) {
        perror("");
        close(fd);
        return;
    }

    std::cout << std::endl << std::endl << "Creating " << name << std::endl;

    if(!strcmp(name, "file.txt")) {
        char percentage[100];
        char * content = "AAAAAAAAAA";
        int content_size = strlen(content);

        for(long long i = 0; i < FILE_SIZE / content_size; i++) {
            double value = (i + 1) / ((double)FILE_SIZE / content_size);
            sprintf(percentage, "%f%%\r", (value * 100));
            write(1, percentage, strlen(percentage));
            write(fd, content, content_size);
            
            if(errno) perror("");
        }
    }

    else {
        char header[31] = "TIME,INITIAL PERCENTAGE,,SPEED";
        write(fd, header, strlen(header));

        if(errno) perror("");
    }

    std::cout << std::endl << std::endl;
    close(fd);
}

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
        std::cout << std::endl << std::endl << "Compiling " << name << std::endl << std::endl;
        system("./script.sh");
        system("rm script.sh");
    }

    system("rm CMakeLists.txt");

    create_file("file.txt");
    create_file("realtime.csv");
    create_file("generic.csv");

    return 0;
}
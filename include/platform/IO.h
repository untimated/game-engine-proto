#ifndef IO_H
#define IO_H

#include <string>
#include <vector>


/*
 * Header:  IO.h
 * Impl:    IO_win.cpp, IO_osX ...
 * Purpose: Platform level file input/output handler
 * Author:  Michael Herman
 * */


namespace IO {
    typedef void* LibHandler;
    typedef void(*LibFunction)(void);

    struct FileBuffer {
        char* buffer;
        unsigned long bufferSize;
    };

    FileBuffer OpenAndReadFile(std::string path);
    // char* OpenAndReadFileAsync(std::string path);
    std::vector<std::string> ListDirFiles(std::string pattern);

    bool SaveFile(FileBuffer *file, std::string path);
    
    LibHandler LoadLib(std::string path);
    void FreeLib(LibHandler *libHandler);
    LibFunction GetLibFunction(void* libHandler, std::string functionName);
}


#endif

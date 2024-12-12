#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include<filesystem>
#include <vector>
#include <iomanip>

struct Inodo
{
    char name[64];
    int64_t offset[8];
    size_t size;
    bool free;
    Inodo(const std::string & filename = "", bool _f = true) : free(_f)
    {
        std::strncpy(name, filename.c_str(), sizeof(name));
        for (int i = 0; i < 8; i++)
        {
            offset[i] = -1;
        }
        size = 0;
    }
}__attribute((packed));


class Device
{
    private:
    std::fstream file;
    size_t getBlockCount();
    size_t getBlockSize();
    int getStateBlock(size_t index); // -1 have errors in idex or file, 0 is free, 1 is busy.
    std::vector<char> read(const std::string &filename);
    int64_t searchInodo(const std::string & filename);
    Inodo getInodo(int64_t offset);
    size_t getBlankInodo();
    void setInodo(int64_t offset, Inodo inodo);
    size_t getSizeMapBlocks()
    {
        return ((getBlockCount()*sizeof(bool))%getBlockSize() == 0)? getBlockCount()*sizeof(bool)/getBlockSize():
            getBlockCount()*sizeof(bool)/getBlockSize() +1;
    }
    size_t getTotalInodesPerBlock(){return getBlockSize()/sizeof(Inodo);}
    size_t getTotalInodes(){return getTotalBlocksForInode()*getTotalInodesPerBlock();}
    size_t getTotalBlocksForInode()
    {
        size_t inodos_block = getTotalInodesPerBlock();
        return (256%inodos_block == 0)? 256/inodos_block : 256/inodos_block + 1;
    }

    public:
    //1er Avance (Adaptado y mejorado para el 2do), se me fue ponerlo en ingles
    bool create(const std::string & filename,size_t blocksize, size_t block_count);
    bool open(const std::string & filename);
    bool close();
    bool writeBlock(size_t blockNumber, const std::vector<char> & data);
    std::vector<char> readBlock(size_t blockNumber);
    void info();
    //Blocks start at  = 2*sizeof(size_t);

    //2do Avance (Mismo comentario que el anterior)
    void blockMatrix();
    bool format();
    void list();
    void cat(const std::string & filename);
    void hexdump(const std::string & filenameq);
    bool write(const std::string &filename, const std::string & text);
    bool copy_out(const std::string & f1, const std::string & f2);
    bool copy_in(const std::string & f1, const std::string & f2);
    bool remove(const std::string & filename);
}; 
#include "BlockDevice.hpp"

size_t Device::getBlockCount()
{
    if(file.is_open())
    {
        size_t value;
        file.seekg(0,std::ios::beg);
        file.read(reinterpret_cast<char*>(&value),sizeof(value));
        return value;
    }
    return 0;
}

size_t Device::getBlockSize()
{
    if(!file.is_open()) return 0;

    size_t value;
    file.seekg(sizeof(size_t),std::ios::beg);
    file.read(reinterpret_cast<char*>(&value),sizeof(value));
    return value;
}

int Device::getStateBlock(size_t index)
{
    if(!(index >= 0 && index < getBlockCount() && file.is_open())) return -1;
    bool value;
    file.seekg(2*sizeof(size_t) +index*sizeof(bool),std::ios::beg);
    file.read(reinterpret_cast<char*>(&value),sizeof(value));
    if (value) return 1;
    else return 0;
}

bool Device::create(const std::string &filename, size_t block_size, size_t block_count)
{
    file = std::fstream(filename, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
    if(!file.is_open()) 
    {
        std::cerr << "Oops! The file can't be open for write.\n";
        return false;
    }

    file.write(reinterpret_cast<char *>(&block_count),sizeof(block_count));
    file.write(reinterpret_cast<char *>(&block_size),sizeof(block_size));

    std::vector<char> data(block_size*block_count,0);
    file.write(data.data(), data.size());
    file.close();
    return true;
}

bool Device::open(const std::string & filename)
{
    if(file.is_open())
    {
        if(!close())
        {
            std::cerr << "Oops! The file you were using can't be close.\n";
        return false;
        }
    }
    file = std::fstream(filename, std::ios::binary | std::ios::in | std::ios::out);
    if(!file.is_open())
    {
        std::cerr << "Oops! The file can't be open.\n";
        return false;
    }
    return true;
}

bool Device::close()
{
    file.close();
    if(file.is_open()) 
    {
        std::cerr << "The file is still open."<< '\n';
        return false;
    }
    return true;
}

bool Device::writeBlock(size_t blockNumber, const std::vector<char> &data)
{
    if(!file.is_open())
    {
        std::cerr << "The file isn't open.\n";
        return false;
    }

    std::vector<char> vector(getBlockSize(), 0);
    if(data.size() > vector.size())
    {
        vector.assign(data.begin(),data.begin()+ getBlockSize()-1);        
    }else
    {
        vector.assign(data.begin(),data.end()); 
    }
    file.seekp(2*sizeof(size_t)+getBlockSize()*blockNumber);
    file.write(vector.data(),vector.size());
    file.flush();
    return true;
}

std::vector<char> Device::readBlock(size_t blockNumber)
{
    if(file.is_open())
    {
        size_t size = getBlockSize();
        size_t count = getBlockCount();
        file.seekg(size*blockNumber + 2*sizeof(size_t),std::ios::beg);
        std::vector<char> data(size);
        file.read(data.data(),data.size());
        return data;
    }else
    {
        std::cerr << "You didn't open or create any file.\n";
        return std::vector<char>();
    }
}

void Device::info()
{
    if(file.is_open())
    {
        std::cout<< "*BLOCK DEVICE INFO:*" <<
        "\n- Block Size: " << getBlockSize() << "\n- Block Count: " << getBlockCount();
        std::cout << "\n\n";
    }else
    {
        std::cerr << "The file is not open.\n";
    }
}

void Device::blockMatrix()
{
    if(file.is_open())
    {
        std::cout << "--*BLOCKS MATRIX:*--\n";   
        try
        {
            for (size_t i = 0; i < getBlockCount(); i++)
            {
                int v = getStateBlock(i);
                if(v == -1)
                {
                    std::cerr << "Something goes wrong.";
                    return;
                }
                std::cout << v;
                if(i < getBlockCount()-1)
                {
                    std::cout << ", ";
                    ((i+1) % 32 == 0)? std::cout << '\n': std::cout << "";        
                }
            }
        }catch(std::exception& ex)
        {
            std::cerr << "The file hasn't been formatted.";
        }
        std::cout << "\n";
    }else
    {
        std::cerr << "The file is not open.\n";
    }
}

bool Device::format()
{
    if(!file.is_open()) 
    {
        std::cerr << "Oops! The file can't be open.\n";
        return false;
    }

    size_t block_count;
    size_t block_size;
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(&block_count), sizeof(block_count));
    file.read(reinterpret_cast<char*>(&block_size), sizeof(block_size));

    for (size_t i = 0; i < block_count; i++)
    {
        bool value = (i < getSizeMapBlocks() + getTotalBlocksForInode())? true: false;
        file.seekp(2 * sizeof(size_t)+i*sizeof(bool), std::ios::beg);
        file.write(reinterpret_cast<char*>(&value),sizeof(value));
    }
    
    for (int i = 0; i < getTotalBlocksForInode(); i++)
    {
        for (int j = 0; j < getTotalInodesPerBlock(); j++)
        {
            file.seekp(2*sizeof(size_t)+ block_size*getSizeMapBlocks() +(i)*block_size + j*sizeof(Inodo));
            Inodo inodo;
            file.write(reinterpret_cast<const char *>(&inodo),sizeof(inodo));
        } 
    }
    
    return true;
}

void Device::list()
{
    if(!file.is_open())
    {
        std::cerr << "The file is not open.\n";
        return;
    }
    size_t block_size = getBlockSize();
    for (int i = 0; i < getTotalBlocksForInode(); i++)
    {
        for (int j = 0; j < getTotalInodesPerBlock(); j++)
        {
            Inodo inodo;
            file.seekg(2*sizeof(size_t)+ block_size*getSizeMapBlocks() +(i)*block_size + j*sizeof(Inodo));
            file.read(reinterpret_cast<char*>(&inodo),sizeof(inodo));
            if(!inodo.free)
            {
                std::cout<< "- File: " << inodo.name<< '\n';
            }
        } 
    }
}

void Device::cat(const std::string &filename)
{
    std::vector<char> text = read(filename);
    if(!(text.size() > 0)) 
    {
        std::cout << "There is no text for the file you are searching for.\n";
    }
    else
    {
        for (size_t i = 0; i < text.size(); i++)
        {
            std::cout << text[i];
        }
        std::cout << "\n";
    }    
}

void Device::hexdump(const std::string &filename)
{
    std::vector<char> text = read(filename);
    if(text.size() == 0)
    {
        std::cout << "There is no text for the file you are searching for.\n";
    }else
    {
        for (size_t i = 0; i < text.size(); i++)
        {
            unsigned int num = static_cast<unsigned char>(text[i]);
            std::cout << std::hex << std::setw(2) << std::setfill('0') << num;
        }
        
        std::cout << std::dec << "\n\n";
    }
}

std::vector<char> Device::read(const std::string &filename)
{
    if(!file.is_open()) return std::vector<char>();
    int64_t offset = searchInodo(filename);

    if(offset == -1)
    {
        std::cerr << "There is no inode with these filename.\n";
        return std::vector<char>();
    }

    file.flush();
    std::vector<char> text;
    Inodo inodo = getInodo(offset);
    int64_t size = inodo.size;
    size_t block = getBlockSize();
    int i = 0;
    while(size > 0)
    {
        std::vector<char> line;
        if(size > block)
        {
            line = std::vector<char>(block);
            file.seekg(2*sizeof(size_t) + inodo.offset[i]*block,std::ios::beg);
            file.read(line.data(),block);
            size -= block;
        }else
        {
            line = std::vector<char>(size);
            file.seekg(2*sizeof(size_t) + inodo.offset[i]*block,std::ios::beg);
            file.read(line.data(),size);
            size = 0;
        }

        text.insert(text.end(),line.begin(),line.end());
        i++;
    }
    return text;
}

int64_t Device::searchInodo(const std::string &filename)
{
    size_t block_size = getBlockSize();
    char name[64];
    strncpy(name, filename.c_str(), sizeof(name)); 
    for (int i = 0; i < getTotalBlocksForInode(); i++)
    {
        for (int j = 0; j < getTotalInodesPerBlock(); j++)
        {
            Inodo inodo;
            file.seekg(2*sizeof(size_t)+ block_size*getSizeMapBlocks() +(i)*block_size + j*sizeof(Inodo));
            file.read(reinterpret_cast<char *>(&inodo),sizeof(inodo));
            if(strncmp(inodo.name, name, sizeof(inodo.name)) == 0)
            {
                file.seekg(2*sizeof(size_t)+ block_size*getSizeMapBlocks() +(i)*block_size + j*sizeof(Inodo));
                return file.tellg();
            }
        } 
    }

    return -1;
}

Inodo Device::getInodo(int64_t offset)
{
    try
    {
        file.seekg(offset);
        Inodo inodo;
        file.read(reinterpret_cast<char*>(&inodo),sizeof(Inodo));
        return inodo;        
    }catch(std::exception ex)
    {
        std::cerr << "Bad offset point for get Inode.\n";
        return Inodo();
    }
}

size_t Device::getBlankInodo()
{
    size_t block_size = getBlockSize();
    for (int i = 0; i < getTotalBlocksForInode(); i++)
    {
        for (int j = 0; j < getTotalInodesPerBlock(); j++)
        {
            Inodo inodo;
            file.seekg(2*sizeof(size_t)+ block_size*getSizeMapBlocks() +(i)*block_size + j*sizeof(Inodo));
            file.read(reinterpret_cast<char *>(&inodo),sizeof(inodo));
            if(inodo.free)
            {
                file.seekg(2*sizeof(size_t)+ block_size*getSizeMapBlocks() +(i)*block_size + j*sizeof(Inodo));
                return file.tellg();
            }
        } 
    }
    return -1;
}

void Device::setInodo(int64_t offset, Inodo inodo)
{
    try
    {
        file.seekp(offset);
        file.write(reinterpret_cast<const char*>(&inodo),sizeof(inodo));
        file.flush();
    }catch(std::exception ex)
    {
        std::cerr << "Bad offset point for get Inode.\n";
    }
}

bool Device::write(const std::string &filename, const std::string &text)
{
    if(!file.is_open())
    {
        std::cerr << "The file is not open.\n";
        return false;
    }

    int64_t os = searchInodo(filename);
    if(os == -1)
    {
        os = getBlankInodo();
        if(os == -1) 
        {
            std::cerr << "There is no inode with the filename and no space for any other inode\n";
            return false;
        }
        
    }
    Inodo inodo = getInodo(os);
    size_t block_size = getBlockSize();
    
    int try_to_full = ((inodo.size + text.length())%block_size == 0)? (inodo.size + text.length())/block_size:(inodo.size + text.length())/block_size + 1;
    if(try_to_full >= 8)
    {
        std::cerr << "The inode has no memory. You write a text that surpass the limit of 8 blocks.\n";
        return false;
    }

    std::vector<char> line;
    std::string txt = text;
    if( inodo.size%block_size != 0)
    {
        line = std::vector<char>(block_size - inodo.size%block_size,0);
        if(txt.length() > block_size)
        {
            line.assign(txt.begin(),txt.begin()+block_size);
            txt.erase(0,block_size);
        }
        else
        { 
            line.assign(txt.begin(),txt.end());
        }
        file.seekp(2*sizeof(size_t)+inodo.offset[inodo.size/block_size]*block_size + inodo.size%block_size);
        file.write(line.data(),line.size());
        txt.erase(0,block_size - inodo.size%block_size);
    }
    
    const int needed = (txt.length() == text.length())? try_to_full -inodo.size/block_size : try_to_full-inodo.size/block_size - 1;
    int64_t offsets[needed];
    int count = 0;

    //For to find blocks to write.
    for (size_t i = getSizeMapBlocks() + getTotalBlocksForInode(); i < getBlockCount() && count < needed; i++)
    {
        int val = getStateBlock(i);
        if(val == 0)
        {
            offsets[count] = i;
            count++;
        }
    }
    
    if(count != needed) 
    {
        std::cerr << "The amount of blocksrequired for write hasn't been collected.\n";
        return false;
    }

    for (size_t i = 0; i < needed; i++)
    {
        line = std::vector<char>(block_size,0);
        if(txt.length() > block_size)
        {
            line.assign(txt.begin(),txt.begin()+block_size);
            txt.erase(0,block_size);
        }
        else
        { 
            line.assign(txt.begin(),txt.end());
        }
        bool v = true;
        inodo.offset[i+inodo.size/getBlockSize()] = offsets[i];
        file.seekp(2*sizeof(size_t) + offsets[i]*sizeof(bool));
        file.write(reinterpret_cast<char*>(&v),sizeof(v));
        file.seekp(2*sizeof(size_t)+ offsets[i]*block_size);
        file.write(line.data(),line.size());   
    }

    inodo.size += text.length();
    inodo.free = false;
    std::strncpy(inodo.name, filename.c_str(), sizeof(inodo.name)-1);
    setInodo(os,inodo);
    return true;
}

bool Device::copy_out(const std::string &f1, const std::string &f2)
{
    size_t block = getBlockSize();
    Inodo i1, i2;
    int64_t o1,o2;
    o1 = searchInodo(f1);
    o2 = searchInodo(f2);
    if(o1 == -1 || o2 == -1)
    {
        std::cerr << "One of the two files doesn't exist.\n";
        return false;
    }
    i1 = getInodo(o1);
    i2 = getInodo(o2);
    size_t idx1 = (i1.size%block == 0)? i1.size/block:i1.size/block+1;
    size_t idx2 = (i2.size%block == 0)? i2.size/block:i2.size/block+1;

    if (idx1 > idx2) 
    {
        const size_t needed = idx1 - idx2;
        int64_t offsets[needed];
        int count = 0;

        for (size_t i = getSizeMapBlocks() + getTotalBlocksForInode(); i < getBlockCount() && count < needed; i++) 
        {
            if (getStateBlock(i) == 0) 
            {
                offsets[count++] = i;
            }
        }

        if (count != needed) 
        {
            std::cerr << "The amount of blocks required for writing hasn't been collected.\n";
            return false;
        }

        for (size_t i = idx2; i < idx2 + needed; i++) 
        {
            i2.offset[i] = offsets[i - idx2];
            bool v = true;
            file.seekp(2 * sizeof(size_t) + offsets[i - idx2]);
            file.write(reinterpret_cast<const char*>(&v), sizeof(v));
        }
    } 
    else if (idx2 > idx1) 
    {
        for (size_t i = idx1; i < 8; i++) 
        {
            if (i2.offset[i] == -1) break;
            bool v = false;
            file.seekp(2 * sizeof(size_t) + i2.offset[i]);
            file.write(reinterpret_cast<const char*>(&v), sizeof(v));
            i2.offset[i] = -1;
        }
    }


    for (size_t i = 0; i < idx1; i++)
    {
        if (i2.offset[i] == -1 || i1.offset[i] == -1)
        {
            bool v = false;
            file.seekp(2 * sizeof(size_t) + i1.offset[i]);
            file.write(reinterpret_cast<const char*>(&v), sizeof(v));
            i2.offset[i] = -1;
            break;
        }
        std::vector<char> text = readBlock(i1.offset[i]);
        writeBlock(i2.offset[i],text);
    }
    
    i2.size = i1.size;
    setInodo(o2,i2);
    return true;
}

bool Device::copy_in(const std::string &f1, const std::string &f2)
{
    size_t block = getBlockSize();
    Inodo i1, i2;
    int64_t o1 = searchInodo(f1);
    int64_t o2 = searchInodo(f2);

    if (o1 == -1 || o2 == -1) {
        std::cerr << "One of the two files doesn't exist.\n";
        return false;
    }

    i1 = getInodo(o1);
    i2 = getInodo(o2);

    size_t idx1 = (i1.size % block == 0) ? i1.size / block : i1.size / block + 1;
    size_t idx2 = (i2.size % block == 0) ? i2.size / block : i2.size / block + 1;

    if (idx1 > idx2) 
    {
        for (size_t i = idx2; i < 8; i++)
        {
            if (i1.offset[i] == -1) break;
            bool v = false;
            file.seekp(2 * sizeof(size_t) + i1.offset[i]);
            file.write(reinterpret_cast<const char*>(&v), sizeof(v));
            i1.offset[i] = -1;
        }
    } 
    else if (idx2 > idx1) 
    {
        const size_t needed = idx2 - idx1;
        int64_t offsets[needed];
        int count = 0;
        for (size_t i = getSizeMapBlocks() + getTotalBlocksForInode(); i < getBlockCount() && count < needed; i++)
        {
            if (getStateBlock(i) == 0) 
            {
                offsets[count++] = i;
            }
        }

        if (count != needed) 
        {
            std::cerr << "The amount of blocks required for writing hasn't been collected.\n";
            return false;
        }

        for (size_t i = idx1; i < idx1 + needed; i++) 
        {
            i1.offset[i] = offsets[i - idx1];
            bool v = true;
            file.seekp(2 * sizeof(size_t) + offsets[i - idx1]);
            file.write(reinterpret_cast<const char*>(&v), sizeof(v));
        }
    }

    for (size_t i = 0; i < idx2; i++) {
        if (i2.offset[i] == -1 || i1.offset[i] == -1)
        {
            bool v = false;
            file.seekp(2 * sizeof(size_t) + i1.offset[i]);
            file.write(reinterpret_cast<const char*>(&v), sizeof(v));
            i1.offset[i] = -1;
            break;
        }
        std::vector<char> text = readBlock(i2.offset[i]);
        writeBlock(i1.offset[i], text);
    }

    i1.size = i2.size;
    setInodo(o1, i1);
    return true;
}

bool Device::remove(const std::string &filename)
{
    int64_t os = searchInodo(filename);
    if(os == -1)
    {
        std::cerr << "There is no inode with these filename.\n";
        return false;
    }

    Inodo inodo = getInodo(os);
    for (size_t i = 0; i < 8; i++)
    {
        if (inodo.offset[i] == -1) break;
        bool v = false;
        file.seekp(2 * sizeof(size_t) + inodo.offset[i]);
        file.write(reinterpret_cast<const char*>(&v), sizeof(v));
        inodo.offset[i] = -1;
    }
    
    setInodo(os,Inodo());
    return true;
}

#include "BlockDevice.hpp"
#include <sstream>

void create(Device &device,std::istringstream & line)
{
    std::string name;
    long size,count;
    if(!(line >> name))
    {
        std::cerr<< "Invalid Name.\n";
        return;
    }

    if(!(line >> size && size > 0))
    {
        std::cerr << "Invalid number of size.\n";
        return;
    }

    if(!(line >> count && count > 0))
    {
        std::cerr << "Invalid number of size.\n";
        return;
    }

    if(device.create(name, static_cast<size_t>(size),static_cast<size_t>(count)))
    {
        std::cout<< "File system created succesfully.\n";
    };
};

void writeBlock(Device &device,std::istringstream & line)
{
    size_t index;
    if(!(line >> index))
    {
        std::cerr << "Invalid number for index.\n";
        return;
    }

    std::string text; 
    line >> std::ws;
    if (!(std::getline(line, text) && text.at(0) == '\"' && text.back() == '\"')) 
    {
        std::cerr << "Invalid text.\n";
        return; 
    }
    std::getline(line, text, '\"');
    std::vector<char> data(text.length()-2);
    data.assign(text.begin()+1, text.end()-1);
    if( device.writeBlock(index, data)) std::cout << "Block written successfully.\n";
};

void readBlock(Device &device,std::istringstream & line)
{
    long index;
    int begin, end;
    if(!(line >> index && index >= 0 ))
    {
        std::cerr << "Invalid index.\n";
        return;
    }

    std::vector<char> text = device.readBlock(static_cast<size_t>(index));
    if(text.size() == 0) return;
    if(!(line >> begin && begin >= 0 && line >> end && end < text.size() && begin < end))
    {
        std::cerr << "The beginning, ending, or both that you have insert are bad.\n";
        return;
    }

    std::cout << "*Text*:\n";
    for(int i = begin; i < end;i++)
    {
        unsigned int num = static_cast<unsigned char>(text[i]);
        std::cout << std::hex << std::setw(2) << std::setfill('0') << num << ' ';
    }  
    std::cout << std::dec << "\n\n";
}

void writeFile(Device &device,std::istringstream & line)
{
    std::string filename, text;

    if(!(line>> filename))
    {
        std::cerr<< "You wrote bad the file";
        return;
    }
    line >> std::ws;
    if (!(std::getline(line, text) && text.at(0) == '\"' && text.back() == '\"')) 
    {
        std::cerr << "Invalid text.\n";
        return; 
    }
    std::getline(line, text, '\"');
    text.erase(text.begin());
    text.pop_back();
    device.write(filename,text);        
}

void help()
{
    std::cout << "List of Commands: \n-- create filename block_size block_count(Create a new file in this directory)\n";
    std::cout << "\n-- open filename(Open a file, and if you had one file open the other get close)\n";
    std::cout << "\n-- info (See the configuration of the file)\n";
    std::cout << "\n-- write block \"text\"(write in a certain block of the file)\n";
    std::cout << "\n-- read block begin end(Read a certain block of the file)\n";
    std::cout << "\n-- ls (List the blocks you have available)\n";
    std::cout << "\n-- format (This action reset the file you are using and add the map of blocks and files)\n";
    std::cout << "\n-- wr filename \"text\" (Use this command for write a simulated file in the block simulator you are using)\n";
    std::cout << "\n-- cat filename (Use this command for read normally a simulated file in the block simulator you are using)\n";
    std::cout << "\n-- hexdump filename (Use this command for read in hexadecimal a simulated file in the block simulator you are using)\n";
    std::cout << "\n-- cat filename (Use this command for read normally a simulated file in the block simulator you are using)\n";
    std::cout << "\n-- copy_out filename1 filename2 (Copy simulated filename1 to paste in filename2)\n";
    std::cout << "\n-- copy_in filename1 filename2 (Copy simulated filename2 to paste in filename1)\n";
    std::cout << "\n-- rm filename (Remove the simulated file in the block simulator you are using)\n";
    std::cout << "\n-- blocks (See the map of blocks in the simulated file [Only remember that it will not do it if the file is not 'format'])\n";
    std::cout << "\n-- close (Close the file you are executing)\n";
    std::cout << "\n-- clear (Clear the screen)\n";
    std::cout << "\n-- exit (You close the Block Simulator)\n--------------------------------------------------------------\n\n";
}

int main()
{
    Device device;
    std::string command;
    system("clear");
    std::cout<<"To know all the commands, write the word 'help' as command.\n";
    while(true)
    {
        std::cout << "./Block_Device > "; 
        std::getline(std::cin,command);
        bool accepted = false;
        if(command == "exit"){accepted = true; break;}

        if(command == "close") {accepted = true; device.close();}

        if(command == "info") {accepted = true; device.info();}

        if(command == "blocks") {accepted = true; device.blockMatrix();}

        if(command == "format") {accepted = true; (device.format())? std::cout << "Your file has been resetted succesfully.\n": std::cout << "Something goes wrong.\n";}

        if(command == "ls") {accepted = true; device.list();}

        if(command == "clear") {accepted = true; system("clear");}

        if(command == "help") {accepted = true; help();}
        std::istringstream line(command);
        std::string action;
        line >> action;

        if(action == "create")
        {
            create(device,line);        
        }
        else if(action == "open")
        {
            std::string name;
            if((line >> name)) 
            {
                device.open(name);
                std::cout << "File correctly opened.\n";
            }
            else std::cerr << "Error in the file system.\n";
        }
        else if(action == "write")
        {
            writeBlock(device, line);
        }
        else if(action == "read")
        {
            readBlock(device,line);
        }
        else if(action == "wr")
        {
            writeFile(device,line);
        }else if(action == "cat")
        {
            std::string name;
            line >>name;
            device.cat(name);
            
        }else if(action == "hexdump")
        {
            std::string name;
            line >>name;
            device.hexdump(name);
        }else if(action == "copy_out")
        {
            std::string f1,f2;
            line >> f1;
            line >> f2;
            device.copy_out(f1,f2);
        }
        else if(action == "copy_in")
        {
            std::string f1,f2;
            line >> f1;
            line >> f2;
            device.copy_in(f1,f2);
        }else if(action == "rm")
        {
            std::string name;
            line >> name;
            device.remove(name);
        }
        else if(!accepted)
        {
            std::cout<<"You don't write any command.\n";
            std::cout<<"To know all the commands, write the word 'help' as command.\n";
        }
    }
    return 0;
}
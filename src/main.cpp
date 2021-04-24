/** Includes **/
#include <iostream>
#include <string>
#include <cstring>
#include "mmu.h"
#include "pagetable.h"

/** Prototypes **/
void printStartMessage(int page_size);
void createProcess(int text_size, int data_size, Mmu *mmu, PageTable *page_table);
void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table);
void setVariable(uint32_t pid, std::string var_name, uint32_t offset, void *value, Mmu *mmu, PageTable *page_table, void *memory);
void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table);
void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table);
void splitString(std::string text, char d, std::vector<std::string>& result);
/** Main function **/

int main(int argc, char **argv)
{
    // Ensure user specified page size as a command line parameter
    if (argc < 2)
    {
        fprintf(stderr, "Error: you must specify the page size\n");
        return 1;
    }

    // Print opening instuction message
    int page_size = std::stoi(argv[1]);
    printStartMessage(page_size);

    // Create physical 'memory' of 64 MB (64 * 1024 * 1024)
    uint32_t mem_size = 67108864;
    void *memory = malloc(mem_size);

    // Create MMU and Page Table
    Mmu *mmu = new Mmu(mem_size);
    PageTable *page_table = new PageTable(page_size);

    // Prompt loop
    std::string command;
    std::vector<std::string> command_parameters;
    std::cout << "> ";
    std::getline (std::cin, command);

    // Handle current command
    while (command != "exit") {
        
        // Split the command into space-delimited arguments stored in the command_parameters vector
        splitString(command, ' ', command_parameters);

        // Parse create() arguments
        if(command_parameters[0] == "create") {
            int text_size = std::stoi(command_parameters[1]);
            int data_size = std::stoi(command_parameters[2]);
            createProcess(text_size, data_size, mmu, page_table);

        // Parse allocate() arguments
        } else if(command_parameters[0] == "allocate") {
            /* 
            void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table)
            */
            uint32_t pid = atoi(command_parameters[1].c_str());

            //check if process exists
            if(mmu->findProcess(pid)){
            std::string var_name = command_parameters[2];
            DataType type;
                if(command_parameters[3] == "char"){
                    type = DataType::Char;
                }else if(command_parameters[3] == "int"){
                type = DataType::Int;
                }else if(command_parameters[3] == "short"){
                    type = DataType::Short;
                }else if(command_parameters[3] == "float"){
                    type = DataType::Float;
                }else if(command_parameters[3] == "double"){
                    type = DataType::Double;
                }else if(command_parameters[3] == "long"){
                    type = DataType::Long;
                }
            uint32_t num_elements = atoi(command_parameters[4].c_str());
            allocateVariable(pid, var_name, type, num_elements, mmu, page_table);
            }else{
                std::cout << "error: process not found" << std::endl;
            }


            
            /*
            std::cout << "pid: " << pid << std::endl;
            std::cout << "var_name: " << var_name << std::endl;
            std::cout << "type: " << type << std::endl;
            std::cout << "num_elements: " << num_elements << std::endl;
            */

            

        // Parse set() arguments
        } else if(command_parameters[0] == "set") {
            uint32_t PID = std::stoi(command_parameters[1]);
            std::string var_name = command_parameters[2];
            uint32_t offset = std::stoi(command_parameters[3]);

            // Call setVariable() for all n values passed in, starting from the 4th parameter and ending at the size of the vector
            for(int i = 4; i < command_parameters.size(); i++) {
                setVariable(PID, var_name, offset, &command_parameters[i], mmu, page_table, memory);
            }

        // Parse free() arguments
        } else if(command_parameters[0] == "free") {
            uint32_t PID = std::stoi(command_parameters[1]);
            std::string var_name = command_parameters[2];
            freeVariable(PID, var_name, mmu, page_table);

        // Parse terminate() arguments 
        } else if(command_parameters[0] == "terminate") {
            uint32_t PID = std::stoi(command_parameters[1]);
            terminateProcess(PID, mmu, page_table);

        // Parse print() arguments
        } else if(command_parameters[0] == "print") {
            std::string object = command_parameters[1];
            if(object == "mmu") {
                // Print the MMU memory table
                mmu->print();

            } else if(object == "page") {
                // Print the page table (do not need to print anything for free frames)

            } else if(object == "processes") {
                // Print a list of PIDs for processes that are still running
                mmu->printProcesses();

            } else {
                // If <object> is a "<PID>:<var_name>", print the value of the variable for that process 
                std::vector<std::string> print_process_arguments;
                splitString(object, ':', print_process_arguments);
                try {
                    uint32_t PID = std::stoi(print_process_arguments[0]);
                    std::string var_name = print_process_arguments[1];

                    // Print the value of the variable var_name for process PID


                } catch(const std::invalid_argument& ia) {
                    std::cout << "error: command not recognized" << std::endl;
                }
            }
            

        // Command not recognized
        } else {
            std::cout << "error: command not recognized" << std::endl;
        }

        // Get next command
        std::cout << "> ";
        std::getline (std::cin, command);
    }

    // Clean up
    free(memory);
    delete mmu;
    delete page_table;

    return 0;
}

/** Prints start message and command list **/
void printStartMessage(int page_size)
{
    std::cout << "Welcome to the Memory Allocation Simulator! Using a page size of " << page_size << " bytes." << std:: endl;
    std::cout << "Commands:" << std:: endl;
    std::cout << "  * create <text_size> <data_size> (initializes a new process)" << std:: endl;
    std::cout << "  * allocate <PID> <var_name> <data_type> <number_of_elements> (allocated memory on the heap)" << std:: endl;
    std::cout << "  * set <PID> <var_name> <offset> <value_0> <value_1> <value_2> ... <value_N> (set the value for a variable)" << std:: endl;
    std::cout << "  * free <PID> <var_name> (deallocate memory on the heap that is associated with <var_name>)" << std:: endl;
    std::cout << "  * terminate <PID> (kill the specified process)" << std:: endl;
    std::cout << "  * print <object> (prints data)" << std:: endl;
    std::cout << "    * If <object> is \"mmu\", print the MMU memory table" << std:: endl;
    std::cout << "    * if <object> is \"page\", print the page table" << std:: endl;
    std::cout << "    * if <object> is \"processes\", print a list of PIDs for processes that are still running" << std:: endl;
    std::cout << "    * if <object> is a \"<PID>:<var_name>\", print the value of the variable for that process" << std:: endl;
    std::cout << std::endl;
}

/** [DONE] Initializes a new process and prints its PID **/
void createProcess(int text_size, int data_size, Mmu *mmu, PageTable *page_table)
{
    // Create a new process in the MMU using the MMU's createProcess() method, which returns the current PID
    uint32_t current_pid = mmu->createProcess();
    // Allocate <TEXT> variable for the newly created process using text_size
    allocateVariable(current_pid, "<TEXT>", Char, text_size, mmu, page_table);
    // Allocate <GLOBALS> variable for the newly created process using data_size
    allocateVariable(current_pid, "<GLOBALS>", Char, data_size, mmu, page_table);
    // Allocate <STACK> variable with a defined size of 65536
    allocateVariable(current_pid, "<STACK>", Char, 65536, mmu, page_table);
    // Print the current PID to the console
    std::cout << current_pid << std::endl;
}

/** [INCOMPLETE] Allocates memory on the heap (how much depends on the data type and the number of elements), then prints the virtual memory address **/
void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    //   - find first free space within a page already allocated to this process that is large enough to fit the new variable
    //   - if no hole is large enough, allocate new page(s)
    //   - insert variable into MMU
    //   - print virtual memory address 
    uint32_t theNewVariableSize;
    int elementSize;
    std::vector<Variable*> variables = mmu->getVariables(pid);

    if(type == DataType::Char){
        theNewVariableSize = num_elements * 1;
        elementSize = 1;
    }else if(type == DataType::Short){
        theNewVariableSize = num_elements * 2;
        elementSize = 2;
    }else if(type == DataType::Int){
        theNewVariableSize = num_elements * 4;
        elementSize = 4;
    }else if(type == DataType::Float){
        theNewVariableSize = num_elements * 4;
        elementSize = 4;
    }else if(type == DataType::Double){
        theNewVariableSize = num_elements * 8;
        elementSize = 8;
    }else if(type == DataType::Long){
        theNewVariableSize = num_elements * 8;
        elementSize = 8;
    }

    for(int i=0; i < variables.size(); i++){
        //Check this if statement later, used var.name for mmu line(19)
        if(variables[i]->name == "<FREE_SPACE>"){
            uint32_t AddressOfFreeSpace = variables[i]->virtual_address;
            uint32_t sizeOfFreeSpace = variables[i]->size;
            int pageNumber = page_table->getPageNumber(AddressOfFreeSpace);

            //check if there is enough space for the new elements
            if(sizeOfFreeSpace >= theNewVariableSize){
                uint32_t spaceLeftOnpage = mmu->getFreeSpaceLeftOnPage(pid, pageNumber, page_table->getPageSize(), AddressOfFreeSpace);
                //check if the page fits the new variable
                if(spaceLeftOnpage >= theNewVariableSize){
                    variables[i]->size = sizeOfFreeSpace - theNewVariableSize;
                    variables[i]->virtual_address = AddressOfFreeSpace + theNewVariableSize;
                    mmu->addVariableToProcess(pid, var_name, type, theNewVariableSize, AddressOfFreeSpace);
                    if(var_name != "<TEXT>" && var_name != "<GLOBALS>" && var_name != "<STACK>"){
                            std::cout << AddressOfFreeSpace << std::endl;
                    }
                    break;
                }else if(spaceLeftOnpage >= elementSize){

                    while(spaceLeftOnpage%elementSize != 0){
                        AddressOfFreeSpace++;
                        sizeOfFreeSpace--;
                    }
                    
                    if(sizeOfFreeSpace >= theNewVariableSize){
                        variables[i]->size = sizeOfFreeSpace - theNewVariableSize;
                        variables[i]->virtual_address = AddressOfFreeSpace + theNewVariableSize;
                        mmu->addVariableToProcess(pid, var_name, type, theNewVariableSize, AddressOfFreeSpace);
                        if(var_name != "<TEXT>" && var_name != "<GLOBALS>" && var_name != "<STACK>"){
                            std::cout << AddressOfFreeSpace << std::endl;
                        }
                        
                        break;
                    }else{
                        //the variables size is bigger than the new size of the free space
                        mmu->addVariableToProcess(pid, var_name, type, theNewVariableSize, AddressOfFreeSpace);

                    }
                }else if(spaceLeftOnpage < elementSize){
                    while(page_table->getPageNumber(AddressOfFreeSpace) == pageNumber){
                        AddressOfFreeSpace++;
                        sizeOfFreeSpace--;
                    }
                    if(sizeOfFreeSpace >= theNewVariableSize){
                        variables[i]->size = sizeOfFreeSpace - theNewVariableSize;
                        variables[i]->virtual_address = AddressOfFreeSpace + theNewVariableSize;
                        mmu->addVariableToProcess(pid, var_name, type, theNewVariableSize, AddressOfFreeSpace);

                        if(var_name != "<TEXT>" && var_name != "<GLOBALS>" && var_name != "<STACK>"){
                            std::cout << AddressOfFreeSpace << std::endl;
                        }
                        break;
                    }else{
                        //the variables size is bigger than the new size of the free space
                        mmu->addVariableToProcess(pid, var_name, type, theNewVariableSize, AddressOfFreeSpace);
                    }
                }

            }
            
            page_table->addEntry(pid, pageNumber);
            

        }
        mmu->addVariableToProcess(pid, var_name, type, theNewVariableSize, variables[i]->virtual_address);
    }//end of for loop

    


    





}

/** [INCOMPLETE] Sets the value for a variable starting at an offset **/
void setVariable(uint32_t pid, std::string var_name, uint32_t offset, void *value, Mmu *mmu, PageTable *page_table, void *memory)
{
    // TODO: implement this!
    //   - look up physical address for variable based on its virtual address / offset

    //   - insert `value` into `memory` at physical address
    
    //   * note: this function only handles a single element (i.e. you'll need to call this within a loop when setting
    //           multiple elements of an array) 
}

/** [INCOMPLETE] Deallocates memory on the heap that is associated with a variable **/
void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    //   - remove entry from MMU
    //   - free page if this variable was the only one on a given page

    // Change the variable name and type to represent free space

    // Check if either the variable just before it and/or just after it are also free space - if so merge them into one larger free space
}

/** [INCOMPLETE] Kills the specified process and frees all memory associated with it **/
void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    //   - remove process from MMU

    //   - free all pages associated with given process
}

/** splitString function imported from assignment 2 - splits a string based on a delimiter and stores the result in a vector **/
void splitString(std::string text, char d, std::vector<std::string>& result)
{   
    enum states { NONE, IN_WORD, IN_STRING } state = NONE;
    int i;
    std::string token;
    result.clear();
    for (i = 0; i < text.length(); i++)
    {
        char c = text[i];
        switch (state) {
            case NONE:
                if (c != d)
                {
                    if (c == '\"')
                    {
                        state = IN_STRING;
                        token = "";
                    }
                    else
                    {
                        state = IN_WORD;
                        token = c;
                    }
                }
                break;
            case IN_WORD:
                if (c == d)
                {
                    result.push_back(token);
                    state = NONE;
                }
                else
                {
                    token += c;
                }
                break;
            case IN_STRING:
                if (c == '\"')
                {
                    result.push_back(token);
                    state = NONE;
                }
                else
                {
                    token += c;
                }
                break;
        }
    }
    if (state != NONE)
    {
        result.push_back(token);
    }
}

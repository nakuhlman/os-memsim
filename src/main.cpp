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
int element_size(DataType type);
void printValue(uint32_t pid, std::string var_name, uint32_t offset, void *value, void *memory);

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

        // Parse set() arguments
        } else if(command_parameters[0] == "set") {

            uint32_t PID = std::stoi(command_parameters[1]);
            std::string var_name = command_parameters[2];
            int offset = std::stoi(command_parameters[3]);
            Variable* curVar = mmu->getVariable(PID, var_name);
            int nextElementOffset = 0;

            // Check if the pid exists, if not, print an error and do nothing
            if(!mmu->findProcess(PID)) {
            std::cout << "error: process not found" << std::endl;
            }else{
                // Check if the variable exists, if not, print an error and do nothing
                if(mmu->findVariable(PID, var_name)) {
                    if(curVar->type == DataType::Char){
                        for(int i = 4; i < command_parameters.size(); i++) {
                            char val = command_parameters[i].at(0);
                            setVariable(PID, var_name, (offset + nextElementOffset)* 1, &val, mmu, page_table, memory);
                            nextElementOffset++;
                        }
                    }else if(curVar->type == DataType::Int){
                        for(int i = 4; i < command_parameters.size(); i++) {
                            int val = std::stoi(command_parameters[i]);
                            setVariable(PID, var_name, (offset + nextElementOffset) * 4, &val, mmu, page_table, memory);
                            nextElementOffset++;
                        }
                    }else if(curVar->type == DataType::Short){
                        for(int i = 4; i < command_parameters.size(); i++) {
                            short val = std::stoi(command_parameters[i]);
                            setVariable(PID, var_name, (offset + nextElementOffset) * 2, &val, mmu, page_table, memory);
                            nextElementOffset++;
                        }
                    }else if(curVar->type == DataType::Float){
                        for(int i = 4; i < command_parameters.size(); i++) {
                            float val = std::stof(command_parameters[i]);
                            setVariable(PID, var_name, (offset + nextElementOffset) * 4, &val, mmu, page_table, memory);
                            nextElementOffset++;
                        }
                    }else if(curVar->type == DataType::Double){
                        for(int i = 4; i < command_parameters.size(); i++) {
                            double val = std::stod(command_parameters[i]);
                            setVariable(PID, var_name, (offset + nextElementOffset) * 8, &val, mmu, page_table, memory);
                            nextElementOffset++;
                        }
                    }else if(curVar->type == DataType::Long){
                        for(int i = 4; i < command_parameters.size(); i++) {
                            long val = std::stol(command_parameters[i]);
                            setVariable(PID, var_name, (offset + nextElementOffset) * 8, &val, mmu, page_table, memory);
                            nextElementOffset++;
                        }
                    }
                }else{
                    std::cout << "error: variable not found" << std::endl;
                }

        }
            
            // Call setVariable() for all n values passed in, starting from the 4th parameter and ending at the size of the vector


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
                page_table->print();

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
                    Variable* curVar = mmu->getVariable(PID, var_name);
                    uint32_t elementSize = element_size(curVar->type);
                    int curVarElements = (curVar->size)/elementSize;

                    if(!mmu->findProcess(PID)) {
                        std::cout << "error: process not found" << std::endl;
                    } else if(!mmu->findVariable(PID, var_name)) {
                        std::cout << "error: command not recognized" << std::endl;
                    } else {
                        int counter = 0;
                    switch (curVar->type) {

                        case DataType::Char:
                        for(int i = 0; i < curVar->size; i = i + elementSize) {
                            char curChar;
                            uint32_t physical_address = page_table->getPhysicalAddress(PID, curVar->virtual_address);

                            memcpy(&curChar, (uint8_t*)memory + physical_address + i, elementSize);


                            if(i == 0){
                                std::cout << curChar;
                                counter++;
                            }else{
                                if(curChar != NULL && counter < 4){
                                    std::cout << ", " << curChar;
                                }
                                counter++;
                            }

                            
                            if(counter == 4) {
                                std::cout << ", ... [" << curVarElements << " items]";
                                break;
                            }
                        }
                        std::cout << std::endl;
                        break;

                        case DataType::Int:

                        for(int i = 0; i < curVar->size; i = i + elementSize) {
                            int curInt;
                            uint32_t physical_address = page_table->getPhysicalAddress(PID, curVar->virtual_address);
                            memcpy(&curInt, (uint8_t*)memory + physical_address + i, elementSize);
                            if(i == 0){
                                std::cout << curInt;
                                counter++;
                            }else{
                                if(curInt != NULL && counter < 4){
                                    std::cout << ", " << curInt;
                                }
                                counter++;
                            }

                            
                            if(counter == 4) {
                                std::cout << ", ... [" << curVarElements << " items]";
                                break;
                            }
                        }    
                        std::cout << std::endl;                    
                        break;

                        case DataType::Short:
                        for(int i = 0; i < curVar->size; i = i + elementSize) {
                            short curShort;
                            uint32_t elementSize = element_size(curVar->type);
                            uint32_t physical_address = page_table->getPhysicalAddress(PID, curVar->virtual_address);

                            memcpy(&curShort, (uint8_t*)memory + physical_address + i, elementSize);
                            if(i == 0){
                                std::cout << curShort;
                                counter++;
                            }else{
                                if(curShort != NULL && counter < 4){
                                    std::cout << ", " << curShort;
                                }
                                counter++;
                            }

                            
                            if(counter == 4) {
                                std::cout << ", ... [" << curVarElements << " items]";
                                break;
                            }
                        }        
                        std::cout << std::endl;                
                        break;

                        case DataType::Float:
                        for(int i = 0; i < curVar->size; i = i + elementSize) {
                            float curFloat;
                            uint32_t elementSize = element_size(curVar->type);
                            uint32_t physical_address = page_table->getPhysicalAddress(PID, curVar->virtual_address);

                            memcpy(&curFloat, (uint8_t*)memory + physical_address + i, elementSize);
                            std::cout << curFloat << ", ";
                            if(i == 0){
                                std::cout << curFloat;
                                counter++;
                            }else{
                                if(curFloat != NULL && counter < 4){
                                    std::cout << ", " << curFloat;
                                }
                                counter++;
                            }

                            
                            if(counter == 4) {
                                std::cout << ", ... [" << curVarElements << " items]";
                                break;
                            }
                        }     
                        std::cout << std::endl;                   
                        break;

                        case DataType::Double:
                        for(int i = 0; i < curVar->size; i = i + elementSize) {
                            double curDouble;
                            uint32_t elementSize = element_size(curVar->type);
                            uint32_t physical_address = page_table->getPhysicalAddress(PID, curVar->virtual_address);

                            memcpy(&curDouble, (uint8_t*)memory + physical_address + i, elementSize);
                            if(i == 0){
                                std::cout << curDouble;
                                counter++;
                            }else{
                                if(curDouble != NULL && counter < 4){
                                    std::cout << ", " << curDouble;
                                }
                                counter++;
                            }

                            
                            if(counter == 4) {
                                std::cout << ", ... [" << curVarElements << " items]";
                                break;
                            }
                        }
                        std::cout << std::endl;
                        break;

                        case DataType::Long:
                        for(int i = 0; i < curVar->size; i = i + elementSize) {
                            long curLong;
                            uint32_t elementSize = element_size(curVar->type);
                            uint32_t physical_address = page_table->getPhysicalAddress(PID, curVar->virtual_address);

                            memcpy(&curLong, (uint8_t*)memory + physical_address + i, elementSize);

                            if(i == 0){
                                std::cout << curLong;
                                counter++;
                            }else{
                                if(curLong != NULL && counter < 4){
                                    std::cout << ", " << curLong;
                                }
                                counter++;
                            }

                            
                            if(counter == 4) {
                                std::cout << ", ... [" << curVarElements << " items]";
                                break;
                            }
                        }
                        std::cout << std::endl;
                        break;

                        default:
                        break;
                    }
                    }
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

/** Initializes a new process and prints its PID **/
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

/** Allocates memory on the heap (how much depends on the data type and the number of elements), then prints the virtual memory address **/
void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table)
{
    uint32_t theNewVariableSize;
    int elementSize;

    theNewVariableSize = element_size(type) * num_elements;
    elementSize = element_size(type);

    if(mmu->findVariable(pid, var_name)){
        std::cout << "error: variable already exists" << std::endl;
        return;
    }

    if(mmu->checkTotalSpace(theNewVariableSize) == true){

        std::vector<Variable*> variables = mmu->getVariables(pid);
        for(int i=0; i < variables.size(); i++){
            //Look for free space and check if there is enough space for the new elements
            if(variables[i]->name == "<FREE_SPACE>" && variables[i]->size >= theNewVariableSize){
                uint32_t addressOfFreeSpace = variables[i]->virtual_address;
                uint32_t sizeOfFreeSpace = variables[i]->size;
                int pageNumber = page_table->getPageNumber(addressOfFreeSpace);
                uint32_t spaceLeftOnpage = mmu->getFreeSpaceLeftOnPage(pid, pageNumber, page_table->getPageSize(), addressOfFreeSpace);

                Variable *newVariable = variables[i];
                //check if the page fits the new variable
                if(spaceLeftOnpage >= theNewVariableSize){
                    variables[i]->size = sizeOfFreeSpace - theNewVariableSize;
                    
                    mmu->addVariableToProcess(pid, var_name, type, theNewVariableSize, newVariable->virtual_address);
                    pageNumber = page_table->getPageNumber(newVariable->virtual_address);
                    variables[i]->virtual_address = variables[i]->virtual_address + theNewVariableSize;
                    uint32_t tempAddress = addressOfFreeSpace + theNewVariableSize - 1;
                    int endOfVariablePage = page_table->getPageNumber(tempAddress);

                    int i = pageNumber;
                    while (i <= endOfVariablePage){
                        page_table->addEntry(pid,i);
                        i++;
                    }
                    
                    if(var_name != "<TEXT>" && var_name != "<GLOBALS>" && var_name != "<STACK>"){
                            std::cout << addressOfFreeSpace << std::endl;
                    }
                    break;
                }else if(spaceLeftOnpage >= elementSize){

                    while(spaceLeftOnpage%elementSize != 0){
                        addressOfFreeSpace++;
                    }
                        
                    if(sizeOfFreeSpace >= theNewVariableSize){
                        variables[i]->size = sizeOfFreeSpace - theNewVariableSize;
                        mmu->addVariableToProcess(pid, var_name, type, theNewVariableSize, newVariable->virtual_address);
                        pageNumber = page_table->getPageNumber(newVariable->virtual_address);
                        variables[i]->virtual_address = addressOfFreeSpace + theNewVariableSize;

                        uint32_t tempAddress = addressOfFreeSpace + theNewVariableSize - 1;
                        int endOfVariablePage = page_table->getPageNumber(tempAddress);

                        
                        int i = pageNumber;
                        while (i <= endOfVariablePage){
                            page_table->addEntry(pid,i);
                            i++;
                        }
                        
                        if(var_name != "<TEXT>" && var_name != "<GLOBALS>" && var_name != "<STACK>"){
                            std::cout << addressOfFreeSpace << std::endl;
                        }
                            
                        break;
                    }else{
                            //the variables size is bigger than the new size of the free space
                        mmu->addVariableToProcess(pid, var_name, type, theNewVariableSize, addressOfFreeSpace);

                    }
                }else if(spaceLeftOnpage < elementSize){
                    while(page_table->getPageNumber(addressOfFreeSpace) == pageNumber){
                        addressOfFreeSpace++;
                    }
                    if(sizeOfFreeSpace >= theNewVariableSize){
                        variables[i]->size = sizeOfFreeSpace - theNewVariableSize;
                        mmu->addVariableToProcess(pid, var_name, type, theNewVariableSize, newVariable->virtual_address);
                        pageNumber = page_table->getPageNumber(newVariable->virtual_address);
                        variables[i]->virtual_address = addressOfFreeSpace + theNewVariableSize;

                        uint32_t tempAddress = addressOfFreeSpace + theNewVariableSize - 1;
                        int endOfVariablePage = page_table->getPageNumber(tempAddress);


            
                        int i = pageNumber;
                        while (i <= endOfVariablePage){
                            page_table->addEntry(pid,i);
                            i++;
                        }
                
                        if(var_name != "<TEXT>" && var_name != "<GLOBALS>" && var_name != "<STACK>"){
                            std::cout << addressOfFreeSpace << std::endl;
                        }
                        break;
                    }else{
                            //the variables size is bigger than the new size of the free space
                        mmu->addVariableToProcess(pid, var_name, type, theNewVariableSize, addressOfFreeSpace);
                    }
                }
            }
        }//end of for loop
    }else{
        std::cout << "error: allocation would exceed system memory" << std::endl;
    }
    

    
}

/** Sets the value for a variable starting at an offset **/
void setVariable(uint32_t pid, std::string var_name, uint32_t offset, void *value, Mmu *mmu, PageTable *page_table, void *memory)
{
    

    Variable* current_var = mmu->getVariable(pid, var_name);
    int physical_address = page_table->getPhysicalAddress(pid, current_var->virtual_address+offset);
    uint32_t elementSize = element_size(current_var->type);
    memcpy((uint8_t*)memory + physical_address, value, elementSize);
}

/** Deallocates memory on the heap that is associated with a variable **/
void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table)
{
    // Check if the pid exists, if not, print an error and do nothing
    // Remove entry from MMU by by changing the variable name and type to represent free space (using set()?)

    if(!mmu->findProcess(pid)) {
        std::cout << "error: process not found" << std::endl;
        return;
    }

    // Check if variable exists, if not, print and error and do nothing
    if(!mmu->findVariable(pid, var_name)) {
        std::cout << "error: variable not found" << std::endl;
        return;
    }

    Variable* curVar = mmu->getVariable(pid, var_name);
    std::vector<Variable*> allVariable = mmu->getVariables(pid);
    int page = page_table->getPageNumber(curVar->virtual_address);
    int endVarpage = page_table->getPageNumber(curVar->virtual_address + curVar->size - 1);
    int num_pages = endVarpage - page + 1;

    //we check if other var are on the same starting page
    //we check the same page until we get to the ending page

    for(int i=page; i <= endVarpage; i++){
        bool check = true;
        for(int j=0; j < allVariable.size(); j++){
            if(page_table->getPageNumber(allVariable[j]->virtual_address) == i
            && allVariable[j]->name != "<FREE_SPACE>"){
                check = false;
            }
        }
        if(check == true){
            page_table->freeSinglePage(pid, i);
        }
    }


    mmu->freeVariable(pid, curVar);
    

    
}

/** Kills the specified process and frees all memory associated with it **/
void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table)
{
    // If the process does not exist, display a message and do nothing
    if(mmu->findProcess(pid) == false) {
        std::cout << "error: process not found" << std::endl;
        return;
    }
    // Otherwise, remove the process from MMU
    mmu->removeProcess(pid);
    // Also free all pages associated with given process
    page_table->freeAllPagesOfProcess(pid);
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

int element_size(DataType type){
    int elementSize = 0;
    if(type == DataType::Char){
        elementSize = 1;
    }else if(type == DataType::Short){
        elementSize = 2;
    }else if(type == DataType::Int){
        elementSize = 4;
    }else if(type == DataType::Float){
        elementSize = 4;
    }else if(type == DataType::Double){
        elementSize = 8;
    }else if(type == DataType::Long){
        elementSize = 8;
    }
    return elementSize;
}
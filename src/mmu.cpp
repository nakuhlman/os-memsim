#include "mmu.h"

Mmu::Mmu(int memory_size)
{
    _next_pid = 1024;
    _max_size = memory_size;
}

Mmu::~Mmu()
{
}

uint32_t Mmu::createProcess()
{
    Process *proc = new Process();
    proc->pid = _next_pid;

    Variable *var = new Variable();
    var->name = "<FREE_SPACE>";
    var->type = DataType::FreeSpace;
    var->virtual_address = 0;
    var->size = _max_size;
    proc->variables.push_back(var);

    _processes.push_back(proc);

    _next_pid++;
    return proc->pid;
}

void Mmu::addVariableToProcess(uint32_t pid, std::string var_name, DataType type, uint32_t size, uint32_t address)
{
    int i;
    Process *proc = NULL;
    for (i = 0; i < _processes.size(); i++)
    {
        if (_processes[i]->pid == pid)
        {
            proc = _processes[i];
        }
    }

    Variable *var = new Variable();
    var->name = var_name;
    var->type = type;
    var->virtual_address = address;
    var->size = size;
    if (proc != NULL)
    {
        proc->variables.push_back(var);
    }
}

void Mmu::print()
{
    int i, j;

    std::cout << " PID  | Variable Name | Virtual Addr | Size" << std::endl;
    std::cout << "------+---------------+--------------+------------" << std::endl;

    // For all processess...
    for (i = 0; i < _processes.size(); i++)
    {
        // For each variable associated with the current process...
        for (j = 0; j < _processes[i]->variables.size(); j++)
        {
            // If the current variable is not a <FREE_SPACE> entry...
            if(_processes[i]->variables[i]->name != "<FREE_SPACE>") 
            {
                // Print the PID of the process associated with the variable
                std::cout << _processes[i]->pid << " ";
                // Print the variable name
                std::cout << _processes[i]->variables[i]->name << " ";
                // Print the virtual address of the variable
                std::cout << _processes[i]->variables[i]->virtual_address << " ";
                // Print the size of the variable
                std::cout << _processes[i]->variables[i]->size << " ";
            }
        }
    }
}

bool Mmu::findProcess(uint32_t pid){
    for(int i=0; i < _processes.size(); i++){
        if(_processes[i]->pid == pid){
            return true;
        }
    }
    return false;
}

void Mmu::printProcesses(){
    for(int i=0; i < _processes.size(); i++){
        std::cout << _processes[i]->pid << std::endl;
    }
}

std::vector<Variable*> Mmu::getVariables(uint32_t pid){
    std::vector<Variable*> empty;
    if(findProcess(pid)){
        for(int i=0; i < _processes.size(); i++){
            if(_processes[i]->pid == pid){
                return _processes[i]->variables;
            }
        }
    }else{
        return empty;
    }
    return empty;
}

int Mmu::getFreeSpaceLeftOnPage(uint32_t pid, int page_number, int page_size, uint32_t address){
    int spaceLeft = page_size;
    if(findProcess(pid)){
        for(int i=0; i < _processes.size(); i++){
            if(_processes[i]->pid == pid){
                for(int j=0; j<_processes[i]->variables.size(); j++){
                    int variable_pageNumber = _processes[i]->variables[j]->virtual_address / page_size;
                    if(_processes[i]->variables[j]->name != "<FREE_SPACE>" && variable_pageNumber == page_number){
                        spaceLeft = spaceLeft - _processes[i]->variables[j]->size;
                    }
                }
            }
        }

        if(spaceLeft <= 0){
            return 0;
        }else{
            return spaceLeft;
        }



    }else{
        return 0;
    }
}
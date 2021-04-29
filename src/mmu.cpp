#include "mmu.h"
#include <sstream>
#include <iomanip>

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
            if(_processes[i]->variables[j]->name != "<FREE_SPACE>1") 
            {
                std::stringstream ss;
                ss << "  0x" << std::setfill('0') << std::setw(8) << std::uppercase << std::hex << _processes[i]->variables[j]->virtual_address;
                std::string hex_virtual_address(ss.str());

                printf("%5u | %-13.13s | %s | %10u\n", _processes[i]->pid, _processes[i]->variables[j]->name.c_str(),
                hex_virtual_address.c_str(), _processes[i]->variables[j]->size);
            }
        }
    }
}

bool Mmu::removeProcess(uint32_t pid) {
    for(int i=0; i < _processes.size(); i++){
        if(_processes[i]->pid == pid) {
            _processes.erase(_processes.begin() + i);
            return true;
        }
    }
    return false;
}

bool Mmu::findProcess(uint32_t pid){
    for(int i=0; i < _processes.size(); i++){
        if(_processes[i]->pid == pid){
            return true;
        }
    }
    return false;
}

//This function check the total space left on the process before adding new variable
bool Mmu::checkTotalSpace(uint32_t newVariableSize){
    //uint32_t mem_size = 67108864;
    uint32_t mem_size = _max_size;
    uint32_t totalSpaceUsed = newVariableSize;
        // For all processess...
    for (int i = 0; i < _processes.size(); i++)
    {
        // For each variable associated with the current process...
        for (int j = 0; j < _processes[i]->variables.size(); j++)
        {
            if(_processes[i]->variables[j]->name != "<FREE_SPACE>") 
            {
               totalSpaceUsed = totalSpaceUsed + _processes[i]->variables[j]->size;
            }
        }
    }
    if(totalSpaceUsed > mem_size){
        return false;
    }else{
        return true;
    }
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

bool Mmu::findVariable(uint32_t pid, std::string var_name) {
    if(findProcess(pid)){
        for(int i=0; i < _processes.size(); i++){
            if(_processes[i]->pid == pid) {
                int j;
                for(j = 0; j < _processes[i]->variables.size(); j++) { 
                    if(_processes[i]->variables[j]->name == var_name) {
                        return true;
                    }
                }
                return false;
            }
        }
        return false;
    }else{
        return false;
    }
}

Variable* Mmu::getVariable(uint32_t pid, std::string var_name) {
    Variable* empty = NULL;
    if(findProcess(pid)){
        for(int i=0; i < _processes.size(); i++){
            if(_processes[i]->pid == pid) {
                int j;
                for(j = 0; j < _processes[i]->variables.size(); j++) { 
                    if(_processes[i]->variables[j]->name == var_name) {
                        return _processes[i]->variables[j];
                    }
                }
                return empty;
            }
        }
        return empty;
    }else{
        return empty;
    }
}

void Mmu::freeVariable(uint32_t pid, Variable* curVar){
    std::vector<Variable*> allVariables = getVariables(pid);
    int indexOfCurVar = getVariableWithaddress(pid, curVar->virtual_address);
    int indexOfNextVariable = getVariableWithaddress(pid, curVar->virtual_address + curVar->size);

    
    for(int i=0; i < _processes.size(); i++){
        int indexOfprev = _processes[i]->variables[indexOfCurVar]->virtual_address;

        if(_processes[i]->pid == pid) {
            if(_processes[i]->variables[indexOfNextVariable]->name == "<FREE_SPACE>"){
                _processes[i]->variables[indexOfNextVariable]->virtual_address = _processes[i]->variables[indexOfCurVar]->virtual_address;

                _processes[i]->variables[indexOfNextVariable]->size = _processes[i]->variables[indexOfNextVariable]->size
                + _processes[i]->variables[indexOfCurVar]->size;

                _processes[i]->variables.erase(_processes[i]->variables.begin() + indexOfCurVar);
                indexOfprev = _processes[i]->variables[indexOfNextVariable]->virtual_address - 1;
            }

            while(indexOfprev != 0){
                if(getVariableWithaddress(pid, indexOfprev) != -1){
                    indexOfprev = getVariableWithaddress(pid, indexOfprev);
                    break;
                }
                indexOfprev--;
            }


            if(_processes[i]->variables[indexOfprev]->name == "<FREE_SPACE>"){
                _processes[i]->variables[indexOfprev]->size = _processes[i]->variables[indexOfprev]->size
                + _processes[i]->variables[indexOfCurVar]->size;
                _processes[i]->variables.erase(_processes[i]->variables.begin() + indexOfprev);
            }
            if(_processes[i]->variables[indexOfNextVariable]->name != "<FREE_SPACE>" && 
            _processes[i]->variables[indexOfprev]->name != "<FREE_SPACE>"){
                _processes[i]->variables[indexOfCurVar]->name = "<FREE_SPACE>";
                _processes[i]->variables[indexOfCurVar]->type = DataType::FreeSpace;

            }
            
        }
    }


}

int Mmu::getVariableWithaddress(uint32_t pid, uint32_t address){
    if(findProcess(pid)){
        for(int i=0; i < _processes.size(); i++){
            if(_processes[i]->pid == pid) {
                int j;
                for(j = 0; j < _processes[i]->variables.size(); j++) { 
                    if(_processes[i]->variables[j]->virtual_address == address) {
                        return j;
                    }
                }
                return -1;
            }
        }
        return -1;
    }else{
        return -1;
    }   
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
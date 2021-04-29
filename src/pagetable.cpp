#include "pagetable.h"
#include <cmath>

PageTable::PageTable(int page_size)
{
    _page_size = page_size;
}

PageTable::~PageTable()
{
}

std::vector<std::string> PageTable::sortedKeys()
{
    std::vector<std::string> keys;

    std::map<std::string, int>::iterator it;
    for (it = _table.begin(); it != _table.end(); it++)
    {
        keys.push_back(it->first);
    }

    std::sort(keys.begin(), keys.end(), PageTableKeyComparator());

    return keys;
}

/** Adds an entry to the page table **/
void PageTable::addEntry(uint32_t pid, int page_number)
{
    // Combination of pid and page number act as the key to look up frame number
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);

    // Starting at frame 0, check mappings...
    int frame = 0; 
    for (std::map<std::string, int>::iterator it=_table.begin(); it!=_table.end(); ++it) {
        // Check and see if the current frame has been mapped to
        if(it->second == frame) {
            // If so, look at the next frame
            frame++;
        }
    }

    // Once a free frame has been found, add the key-value pair
    _table.insert(std::make_pair(entry, frame));
}

/** Calculates the physical address given a PID and a virtual address **/
int PageTable::getPhysicalAddress(uint32_t pid, uint32_t virtual_address)
{
    // Page offset can be found using modulus; page offset is the distance (in bytes) relative to the start of the page
    int page_offset = virtual_address % _page_size;
    // Call getPageNumber() to find the page number for the passed-in virtual address
    int page_number = PageTable::getPageNumber(virtual_address);
    // Combination of pid and page number act as the key to look up frame number in the page table
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);
    
    // If entry exists, look up frame number in the page table
    int address = -1;
    int frame_number = 0;
    if (_table.count(entry) > 0) 
    { 
        frame_number = _table.at(entry); 
    }

    // Physical address = [physical page number (a.k.a. frame number) * page size] + offset
    address = (frame_number * _page_size) + page_offset;

    // Return the physical address
    return address;
}

/** Prints all pages in the page table **/
void PageTable::print()
{
    int i;

    std::cout << " PID  | Page Number | Frame Number" << std::endl;
    std::cout << "------+-------------+--------------" << std::endl;

    std::vector<std::string> keys = sortedKeys();

    // For all keys, in sorted order...
    for (i = 0; i < keys.size(); i++)
    {   
        // Format the current key-value pair for printing
        /*
        std::string pid = keys[i].substr(0, 3);
        std::string page_number = keys[i].substr(), 3);
        */
        int frame_number = _table[keys[i]];
        

        keys[i].insert(4, " ");
        if(stoi(keys[i].substr(6)) < 10) { keys[i].insert(6, "           "); }
        else { keys[i].insert(6, "          "); }

        // Print the key (which includes the PID and Page Number) and also the value associated with that key (the Frame Number) 
        printf(" %5s | %12u\n", keys[i].c_str(), frame_number);
    }
}

/** Getter method for the page size, defined by the user at program startup **/
int PageTable::getPageSize(){ return _page_size; }

void PageTable::freePagesOfProcess(uint32_t pid) {
    std::vector<std::string> keys = sortedKeys();
    for (int i = 0; i < keys.size(); i++)
    {
        if(std::to_string(pid).find(keys[i]) != std::string::npos) {
            _table.erase(keys[i]);
        }
    }
}

/** Calculates a page number using a virtual address and a page size **/
int PageTable::getPageNumber(uint32_t address) {
    // The page number is the number of pages counting up from 0
    return floor(address / _page_size);
}

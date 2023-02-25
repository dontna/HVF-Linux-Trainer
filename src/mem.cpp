#include <cstdio>
#include <cstring>
#include <iostream>
#include <sys/uio.h>
#include <fstream>
#include <sstream>

#include "mem.h"

uintptr_t strtoptr(const char* str, char** endptr, int base){
    #ifdef _WIN32
        return strtoull(str, endptr, base);
    #else
        return strtoull(str, endptr, base);
    #endif
}

// Thanks to obdr at GuidedHacking for the base ReadProcessMemory function.
// I, Dontna, added error handling, and added a bool return value for easier debugging.
// See their code here: https://guidedhacking.com/threads/linux-game-hacking-full-guide.16411/post-101577
bool mem::ReadProcessMemory(pid_t processId, void* src, void* dst, size_t size){
    /*
    processId  = target process id
    src  = address to read from on the target process
    dst  = address to write to on the caller process
    size = size of the buffer that will be read
    */

    struct iovec iosrc;
    struct iovec iodst;
    iodst.iov_base = dst;
    iodst.iov_len  = size;
    iosrc.iov_base = src;
    iosrc.iov_len  = size;

    ssize_t bytes_read = process_vm_readv(processId, &iodst, 1, &iosrc, 1, 0);
    if (bytes_read == -1) {
        std::cerr << "Error: Failed to read process memory at address " << src << " in process " << processId << ". " << strerror(errno) << "\n";
        return false;
    } else if ((size_t)bytes_read != size) {
        std::cerr << "Error: Only " << bytes_read << " bytes were read from address " << src << " in process " << processId << ", expected " << size << " bytes\n";
        return false;
    }
    return true;
}

// Thanks to obdr at GuidedHacking for the base WriteProcessMemory function.
// I, Dontna, added error handling, and added a bool return value for easier debugging.
// See their code here: https://guidedhacking.com/threads/linux-game-hacking-full-guide.16411/post-101577
bool mem::WriteProcessMemory(pid_t processId, void* dst, void* src, size_t size){
    /*
    processId  = target process id
    dst  = address to write to on the target process
    src  = address to read from on the caller process
    size = size of the buffer that will be read
    */

    struct iovec iosrc;
    struct iovec iodst;
    iosrc.iov_base = src;
    iosrc.iov_len  = size;
    iodst.iov_base = dst;
    iodst.iov_len  = size;

    if (process_vm_writev(processId, &iosrc, 1, &iodst, 1, 0) == -1) {
        std::cerr << "Error writing process memory.\n";
        return false;
    }
    return true;
}

/**
 * This function takes a base address and a vector of offsets, and uses them to traverse a 
 * pointer chain in a remote process, ultimately returning the memory address pointed to by 
 * the final pointer in the chain. 
 *
 * @param processId The process ID of the remote process.
 * @param baseAddress The starting address of the pointer chain.
 * @param offsets A vector of offsets to be added to each pointer in the chain.
 * @param debugText A boolean indicating whether to print debug information.
 *
 * @return The memory address pointed to by the final pointer in the chain.
 */
uintptr_t mem::GetAddressFromPointers(pid_t processId, const uintptr_t& baseAddress, const std::vector<int>& offsets, bool debugText) {
    void* pointerAddress;
    bool readSuccess;

    // Perform an initial read to get the value the base address is pointing to.
    if (debugText){
        printf("Number of offsets: %zu\n", offsets.size());
        printf("Base address before: %lx\n", baseAddress);
    }
    readSuccess = mem::ReadProcessMemory(processId, (void*)baseAddress, &pointerAddress, sizeof(pointerAddress));
    if (!readSuccess) {
        if (debugText){
            std::cerr << "Error: Failed to read " << sizeof(pointerAddress) << " bytes from address " << baseAddress << " in process " << processId << "\n";
        }
        return 0;
    }

    // Loop through all, but the final, pointers in the offets and read the address they point to.
    for (std::vector<int>::size_type i = 0; i < offsets.size() - 1; i++) {
        if (debugText) {
            printf("Base address after offset %lu: %lx\n", i, (uintptr_t)pointerAddress);
        }
        readSuccess = mem::ReadProcessMemory(processId, (void*)((uintptr_t)pointerAddress + offsets.at(i)), &pointerAddress, sizeof(pointerAddress));
        if (!readSuccess) {
            if (debugText){
                std::cerr << "Error: Failed to read " << sizeof(pointerAddress) << " bytes from address " << ((uintptr_t)pointerAddress + offsets.at(i)) << " in process " << processId << "\n";
            }
            return 0;
        }
    }

    // Add the last offset in the list to the pointer address to get the correct memory address for our value.
    return (uintptr_t)pointerAddress + offsets.at(offsets.size() - 1);
}


uintptr_t mem::GetModuleBaseAddress(pid_t pid, const std::string& module_name) {
    // Get the path of the 'maps' file for the given process
    std::stringstream maps_file_path;
    maps_file_path << "/proc/" << pid << "/maps";

    // Open the 'maps' file for the given process
    std::ifstream maps_file_fs(maps_file_path.str(), std::ios::binary);
    if (!maps_file_fs.is_open()) return 0; // Return 0 if file can't be opened

    // Read the content of the 'maps' file into a stringstream
    std::stringstream maps_file;
    maps_file << maps_file_fs.rdbuf();

    // Find the position of the module name in the 'maps' file
    size_t module_path_pos = maps_file.str().find(module_name);

    // Find the position of the base address of the module
    size_t base_address_pos = maps_file.str().rfind('\n', module_path_pos) + 1;
    size_t base_address_end = maps_file.str().find('-', base_address_pos);

    // Return 0 if the module name or base address couldn't be found
    if (base_address_pos == maps_file.str().npos || base_address_end == maps_file.str().npos) return 0;

    // Extract the base address string from the 'maps' file
    std::string base_address_str = maps_file.str().substr(base_address_pos, base_address_end - base_address_pos);

    // Convert the base address string to a uintptr_t
    std::uintptr_t base_address = std::stoull(base_address_str, nullptr, 16);

    // Close the 'maps' file stream
    maps_file_fs.close();

    // Return the base address of the module
    return base_address;
}
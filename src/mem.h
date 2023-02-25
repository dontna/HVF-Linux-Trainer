#pragma once

#include <cstdint>
#include <sched.h>
#include <vector>
#include <string>

namespace mem {
    bool ReadProcessMemory(pid_t pid, void* src, void* dst, size_t size);
    bool WriteProcessMemory(pid_t pid, void* dst, void* src, size_t size);
    uintptr_t GetAddressFromPointers(pid_t processId, const uintptr_t& baseAddress, const std::vector<int>& offsets, bool debugText=false);
    uintptr_t GetModuleBaseAddress(pid_t pid, const std::string& module_name);
};

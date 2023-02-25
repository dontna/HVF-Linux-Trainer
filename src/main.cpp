#include <cstdint>
#include <cstring>
#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <linux/input.h>
#include <linux/input-event-codes.h>
#include <string>
#include <sys/types.h>
#include <system_error>
#include <unistd.h>
#include <cstddef>
#include <vector>

#include "mem.h"

// Keybinds, if you change these please use the 'linux/input-event-codes.h' as a guide.
const int EXIT_KEY = KEY_F8;
const int GODMODE_KEY = KEY_KP1;
const int ADD_MONEY_KEY = KEY_KP2;
const int INFAMMO_KEY = KEY_KP3;

// Name of the process, you shouldn't have to change this; but if the code provides an error
// Change this to whatever comes up when you do 'ps -e | grep AVF2' in terminal.
std::string processName = "AVF2-Win64-Ship";

// Event type for the keyboard, change this to yours or the binds will not work.
std::string KEYBOARD_EVENT = "/dev/input/event5";

// Ammount of time, in milliseconds, the program will sleep for after each loop.
// You may want to change this if the hack is taking up too much of your CPU.
__useconds_t SLEEP_MILLISECONDS = 250;

pid_t GetPIDFromProcessName(const char* process_name) {
    char cmd[256];
    // Create a command to run in the shell, which will list all running processes and
    // filter out the process with the specified name using grep.
    snprintf(cmd, sizeof(cmd), "ps -e | grep -m1 %s", process_name);

    // Open a pipe to run the command and read its output.
    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        // If the pipe couldn't be opened, return an error code (-1).
        return -1;
    }

    // Read the first line of the output from the pipe.
    char line[256];
    fgets(line, sizeof(line), pipe);

    // Close the pipe.
    pclose(pipe);

    // If no output was produced by the command, return an error code (-1).
    if (strlen(line) == 0) {
        return -1;
    }

    // Parse the PID from the first token in the output line.
    char* pid_str = strtok(line, " ");
    if (!pid_str) {
        // If no token was found, return an error code (-1).
        return -1;
    }

    // Convert the PID string to an integer and return it.
    return atoi(pid_str);
}

void ClearTerminal(){
    // ANSI escape sequence to clear a terminal.
    // It is used instead of ClearTerminal(), since it is more efficient in a loop.
    std::cout << "\033[2J\033[1;1H";
}

void UpdateTerminalPrompts(bool godmodeActivated, bool infAmmoActivated){
    ClearTerminal();

    printf("[NUMPAD_1] Godmode Activated: %d\n", godmodeActivated);
    printf("[NUMPAD_2] Add 1000 Gold\n");
    printf("[NUMPAD_3] Infinite Ammo Activated: %d\n", infAmmoActivated);
    printf("[F8] Exit Hack\n");
}

int main(){
    // Get Process ID for the game.
    pid_t processID = GetPIDFromProcessName(processName.c_str());
    if (processID == -1){
        std::string errorMsg = "Could not get the PID for '" + processName + "'";
        perror(errorMsg.c_str());
        return 1;
    }

    // Get Base Address for the 'libnvidia-glcore.so.525.89.02' and 'libcuda.so.525.89.02' modules.
    uintptr_t glCoreModuleBase = mem::GetModuleBaseAddress(processID, "libnvidia-glcore.so.525.89.02");
    uintptr_t libCudaModuleBase = mem::GetModuleBaseAddress(processID, "libcuda.so.525.89.02");
    
    uintptr_t healthAddress = mem::GetAddressFromPointers(processID, glCoreModuleBase+0x0014BBF8, {0x8, 0x240, 0x768});
    uintptr_t moneyAddress = mem::GetAddressFromPointers(processID, glCoreModuleBase+0x0014BBC8, {0x38, 0x110, 0x7D4});
    uintptr_t numOfWeaponsAddress = mem::GetAddressFromPointers(processID, glCoreModuleBase+0x000175E0, {0x64, 0x310, 0x260, 0x790});

    // Ammo Addresses
    std::vector<uintptr_t> vectorOfWeaponAmmoAddresses;
    
    // Add ammo addresses to vector list.
    // There's a better way to do this, but it's 4am and my brain is running on fumes.
    vectorOfWeaponAmmoAddresses.push_back(mem::GetAddressFromPointers(processID, libCudaModuleBase+0x00034A78, {0xC0, 0x270, 0x148, 0x0, 0x2B8}));
    vectorOfWeaponAmmoAddresses.push_back(mem::GetAddressFromPointers(processID, libCudaModuleBase+0x00034A78, {0xC0, 0x270, 0x148, 0x8, 0x2B8}));
    vectorOfWeaponAmmoAddresses.push_back(mem::GetAddressFromPointers(processID, libCudaModuleBase+0x00034A78, {0xC0, 0x270, 0x148, 0x10, 0x2B8}));
    vectorOfWeaponAmmoAddresses.push_back(mem::GetAddressFromPointers(processID, libCudaModuleBase+0x00034A78, {0xC0, 0x270, 0x148, 0x18, 0x2B8}));

    float healthValue;
    float maxHealthValue = 1.0;
    
    int moneyValue;
    
    // Vector of weapn ammo values, which allows me to use a loop later.
    // No real reason to do this other than cleaner code.
    std::vector<int> vectorOfWeaponAmmoValues = {0, 0, 0, 0};
    int numOfWeapons;
    int currentWeaponAmmo;
    int maxWeaponAmmo;

    bool godmodeActivated = false;
    bool infAmmoActivated = false;
    
    // Open the event controlling the keyboard, so we can check keypresses.
    int fd = open(KEYBOARD_EVENT.c_str(), O_RDONLY);
    if (fd < 0) {
        std::cerr << ("Failed to open keyboard device. Try running with sudo?");
        return 1;
    }

    // Monitor keyboard events.
    struct input_event ev;
    bool exitKeyPressed = false;

    UpdateTerminalPrompts(godmodeActivated, infAmmoActivated);
    while (!exitKeyPressed) {
        if (read(fd, &ev, sizeof(ev)) < static_cast<ssize_t>(sizeof(ev))) {
            perror("Failed to read keyboard event. Try running with sudo?");
            break;
        }

        // Inital reads of addresses.
        mem::ReadProcessMemory(processID, (void*)healthAddress, &healthValue, sizeof(healthValue));
        mem::ReadProcessMemory(processID, (void*)moneyAddress, &moneyValue, sizeof(moneyValue));
        mem::ReadProcessMemory(processID, (void*)numOfWeaponsAddress, &numOfWeapons, sizeof(numOfWeapons));

        // If godmode activated, and health isn't max.
        if (godmodeActivated && healthValue < maxHealthValue){
            // Set the health to the maximum.
            mem::WriteProcessMemory(processID, (void*)healthAddress, &maxHealthValue, sizeof(maxHealthValue));
        }

        // If InfAmmo Activated
        if (infAmmoActivated){
            // Loop through our active weapons.
            if (numOfWeapons > 4){
                numOfWeapons = 4;
            }

            for (int i = 0; i < numOfWeapons; i++){
                // Read the MAX ammo the mag can store.
                // Note: we do this each time, since the weapon could be swapped out.
                mem::ReadProcessMemory(processID, (void*)(vectorOfWeaponAmmoAddresses.at(i)+0x04), &maxWeaponAmmo, sizeof(maxWeaponAmmo));

                // Read the current ammo in the mag.
                mem::ReadProcessMemory(processID, (void*)vectorOfWeaponAmmoAddresses.at(i), &currentWeaponAmmo, sizeof(currentWeaponAmmo));

                // If our current ammo isn't max
                if (currentWeaponAmmo < maxWeaponAmmo){
                    // Make it maxed.
                    // Note: I'm doing it this way to not have to continuesly write when it's not needed. There's probably a better way to do this.
                    mem::WriteProcessMemory(processID, (void*)(vectorOfWeaponAmmoAddresses.at(i)), &maxWeaponAmmo, sizeof(maxWeaponAmmo));
                }
            }
        }

        if (ev.type == EV_KEY && ev.value == 0){
            switch (ev.code){
            case EXIT_KEY:
                // Exit the Program
                exitKeyPressed = true;
                break;
            case GODMODE_KEY:
                // Toggle GodMode
                godmodeActivated = !godmodeActivated;
                UpdateTerminalPrompts(godmodeActivated, infAmmoActivated);
                break;
            case INFAMMO_KEY:
                // Toggle Infinite Ammo
                infAmmoActivated = !infAmmoActivated;
                UpdateTerminalPrompts(godmodeActivated, infAmmoActivated);
                break;
            case ADD_MONEY_KEY:
                // Add 1000 Money
                int newMoneyValue = moneyValue + 1000;
                mem::WriteProcessMemory(processID, (void*)moneyAddress, &newMoneyValue, sizeof(moneyValue));
                break;
            }
        }

        usleep(SLEEP_MILLISECONDS);
    }

    // Close the keyboard device and exit the program.
    close(fd);

    return 0;
}
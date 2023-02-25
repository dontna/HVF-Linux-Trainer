# HVF-Linux-Trainer

This project was created because a friend of mine said "What's the stupidest game you could make a trainer for?", and we chose the HvF game. Here's the result, if anyone actually wants to use it.



Buy the game here: https://store.steampowered.com/app/1208260/Hentai_Vs_Furries/

# What options does the trainer have?
Since this was a joke, I only added a few options; but feel free to add more yourself!

1. GodMode
2. Infinite Ammo
3. Add 1,000 Money


# How to build?

The trainer uses g++ and make to build the program. Once you have make and g++ installed, just CD into the directory and do:



`make`



This will create the file 'hvf_trainer' in the 'build' directory.



If this doesn't work, or you don't want to use 'make' you can simply compile with g++ alone.



`g++ src/main.cpp src/mem.cpp -o build/hvf_trainer -Wall`



# Setup

You will need to do some setup,



1. Open the 'main.cpp' and at the top of the file there will be

```

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

```



2. You need to change the `std::string KEYBOARD_EVENT = "/dev/input/event5"` to your keyboard event, or the keybinds will not work.



To find your keyboard event, follow these steps:

1. open a terminal and type `cat /proc/bus/input/devices/` this will list all devices on your system.

2. Find your keyboard, you can tell it's your keyboard because the `Name=` varible will tell you.

For example, this is mine:

```I: Bus=0003 Vendor=046d Product=c33f Version=0111

N: Name="Logitech G815 RGB MECHANICAL GAMING KEYBOARD"

P: Phys=usb-0000:00:14.0-7/input0

S: Sysfs=/devices/pci0000:00/0000:00:14.0/usb1/1-7/1-7:1.0/0003:046D:C33F.0003/input/input6

U: Uniq=176B307B3232

H: Handlers=sysrq kbd leds event5 

B: PROP=0

B: EV=120013

B: KEY=1000000000007 ff9f207ac14057ff febeffdfffefffff fffffffffffffffe

B: MSC=10

B: LED=7

```
3. From here we can look at the `Handlers=` and find our keyboard event there. Mine is 'event5'
4. That is the number you put into the code.

Also feel free to change any of the keybinds to fit your tastes.

# Running the trainer
The trainer uses 'process_vm_read' and 'process_vm_write' to read and write values into the game, so the script needs to be run as root.
So the commands would look like
`cd build`
`sudo ./hvf_trainer`

# Using my code
You can use my code in any other projects, this includes the 'mem.cpp' and 'mem.h' files also. All I ask in return is some credit.

# Will I be updating this?
Probably not, no. This was just a silly idea, made completely because I thought it was a silly game to reverse engineer.


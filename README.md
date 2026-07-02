# Purpose of the Project
This is a personal project of mine that is based on a python tic tac toe game made by @erikfalor.
I have translated it to C++, and will be using this project to help me learn how to use socket/network programming by making a server and client to add online multiplayer to the basic game.

# Progress
This project is in the middle of development still.
The server and client are largely finished.
If you run the server and some clients, you can get basic multiplayer going.
The server and client are currently coded to work only on localhost, but that can be easily fixed.
Right now, you can play the tic tac toe game by yourself by building the project and running the `Client` program.
This currently works only on Linux.
I have yet to test anything on Windows, and I need to include the right header files for socket programming in Windows.

# Some Design Notes
As of now, I have the server designed such that connecting to a client creates a thread that handles all the interaction with that user.
I am aware that this design does not scale well, but I am having fun attempting to think through all the logic that goes along with this design.

# Soon TODOs
* Add the simple code to make this work on windows
* Change the board design to use `char` instead of `std::string` since I treat the cells like `char`s anyway
* Use a config/env file to let you set whatever IP address/DNS you want the server to be at
* Make more helpful server logs

# Future Ideas
Once I mostly finish this project, I hope to rewrite it using a more scalable design.
I also don't currently have any tests, or even a way to monitor the server's state, so I will change that.

# Building
Here are the steps for building this project.
As of now, this project doesn't compile on windows since it relies on unix libraries.
I believe that it may be a simple fix to add Windows functionality, but I haven't tested it yet.
Both programs work on Linux (I use Linux Mint, so your mileage may vary, depending on your distribution).

This project uses the C++ 23 standard, so you will need a C++ compiler, Cmake, and Clang Format with versions that support C++ 23.
## Linux
In the project's root directory, run the following.
```bash
mkdir build & cmake -B build -S . && cmake --build build
```

Run the `Server` or `Client` by doing `build/Server` or `build/Client`, respectively.

## Windows
TODO: Verify this

Open a windows command prompt in the project's root directory.
In the project's root directory, run the following.
```
mkdir build
cmake -B build -S .
cmake --build build
```

This will create the MSVC project files that you can open and then compile using MSVC.

TODO: finish these instructions for windows

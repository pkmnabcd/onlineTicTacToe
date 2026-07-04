# Purpose of the Project
This is a personal project of mine that is based on a python tic tac toe game made by @erikfalor.
I have translated it to C++, and will be using this project to help me learn how to use socket/network programming by making a server and client to add online multiplayer to the basic game.

# Progress
This project is nearing the end of development.
The server and client are largely finished.
There are a couple big bugs to fix, but this system seems to work very well.
This project currently works on Linux and windows (the server and the client).

# Some Design Notes
As of now, I have the server designed such that connecting to a client creates a thread that handles all the interaction with that user.
I am aware that this design does not scale well, but I am having fun attempting to think through all the logic that goes along with this design.
There are also ways that I could improve the networking logic by making it so the program doesn't rely on the order of packages.
This could be done by having tags on the packages, but this design would be a part of an overhaul of the whole design to make it scale better.

# Soon TODOs
* Make more helpful server logs

# Future Ideas
The following are some ideas that I might implement in the future.
* Rewrite in a more scalable design
    * Change the system so that instead of having one thread per user, have a networking thread that gives work to a small number of threads that do the matchmaking/gamestate work.
* Add a way to monitor the server state
    * Currently no way to do so. This would probably require another thread (in current design) added to the server and a basic client to send requests to it.

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
Open a windows command prompt in the project's root directory.
In the project's root directory, run the following.
```
mkdir build
cmake -B build -S .
```

This will create the MSVC project files that you can open and then compile using MSVC.
I usually open the `build/OnlineTicTacToe.sln` file and use visual studio 17 to build it.

If you compile in `Release` mode, you can run the program executables by doing `.\build\Release\Server.exe` and `.\build\Release\Client.exe`.

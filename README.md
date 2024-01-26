# Goal
This is my assignment for a ComputerNetworks2 class. The goal was to implement a server and a client in c++. There should be one server and multiple clients that can shutdown each other if they have permissions to do that.
All of them can view all available clients and their permissions (which clients they can shutdown). All data about clients is stored on a server. Client first register to the server, then it can choose command from the available ones. According to it the server will respond with appropiate data.
## How to compile project
```bash
g++ -pthread -o server.out server.cpp
g++ -pthread -o client.out client.cpp
```

## Usage
To run server
```bash
./server.out
```
To run client (client with <id_of_client>=1 is and admin client)
```bash
./client.out <id_of_client>
```

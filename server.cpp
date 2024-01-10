#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>  // for open
#include <unistd.h> // for close
#include <pthread.h>
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <algorithm>
using namespace std;
char client_message[2000];
char buffer[1024];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;
map<int, int> clientSockets;
map<int, bool> activeClients;
map<int, vector<int> > permisssions;
void addPermission(map<int, vector<int> > &clientPermissions, int client, int permission_to)
{
    
     if (clientPermissions.find(client) != clientPermissions.end()) {
        // Check if the permission is not already in the vector for the client
        if (find(clientPermissions[client].begin(), clientPermissions[client].end(), permission_to) == clientPermissions[client].end()) {
            // Permission is not in the vector, so add it
            clientPermissions[client].push_back(permission_to);
        } 
    } else {
        // Client does not exist, create a new entry in the map
        clientPermissions[client].push_back(permission_to);
    }
}
bool hasPermission(const std::map<int, std::vector<int> > &clientPermissions, int client, int permission_to)
{
    // Check if the client exists in the map
    auto clientIterator = clientPermissions.find(client);
    if (clientIterator != clientPermissions.end()) {
        // Check if the permission_to value is in the vector associated with the client
        const std::vector<int> &permissions = clientIterator->second;
        for (int permission : permissions) {
            if (permission == permission_to) {
                return true;  // Client has the specified permission
            }
        }
    }

    return false;  // Client does not have the specified permission
}

void printMap(const std::map<int, int> &myMap)
{
    for (const auto &entry : myMap)
    {
        cout << "Key: " << entry.first << ", Value: " << entry.second << std::endl;
    }
}

struct Request
{
    int client_id;
    string message;
    int receiver_id;
    int receiver_id_permission;
};
Request receiveRequest(char *buff)
{
    istringstream iss(buff);
    Request request;
    iss >> request.client_id >> request.message >> request.receiver_id >> request.receiver_id_permission;
    return request;
}
void deleteClient(map<int, int> &myMap, const int &client)
{
    auto it = myMap.find(client);
    if (it != myMap.end())
    {
        myMap.erase(it);
        printf("client deleted");
    }
    else
    {
        printf("no client with that id");
    }
}
void makeClientInactive(map<int, bool> &myMap, const int &client, bool deletion)

{
    if (deletion)
    {
        auto it = myMap.find(client);
        if (it != myMap.end())
        {
            myMap.erase(it);
            printf("client deleted");
        }
        else
        {
            printf("no client with that id");
        }
    }
    else
    {
    }
}
string showAllClients(map<int, bool> &activeClients){
    stringstream keysStream;
    keysStream << "List of clients:\n";
    for (const auto& pair : activeClients) {
            keysStream << "Client: " << pair.first << ", ";
        }
    return keysStream.str();
}
string showPermission(map<int, vector<int> > &clientPermissions, int client){
    stringstream permissionStream;
    vector <int> permissions = clientPermissions[client];
    int size = permissions.size();
    permissionStream <<"Available to shutdown for client: "<<client << ":[ ";
    for (int i = 0; i < size; i++)
    {
        permissionStream<< permissions[i]<< ", ";
    }
    permissionStream <<"]";
    return permissionStream.str();
}
void *socketThread(void *arg)
{
     printf("1\n");
    int newSocket = *((int *)arg);
    int n;
   
    int client_id;
    int receiver_id;
    for (;;)
    {
        n = recv(newSocket, client_message, sizeof(client_message), 0);

        if (n <= 0)
        {
           perror("Client disconnected");
            break;
        }

        Request request;
        request = receiveRequest(client_message);
        
        client_id = request.client_id;
        clientSockets[client_id] = newSocket;
        activeClients[client_id] = true;
        receiver_id = request.receiver_id;
        
        printf("%s \n", request.message.c_str());
        
        if (request.message == "REGISTER")
        {   
            pthread_mutex_lock(&mutex_lock);
            addPermission(permisssions,client_id,client_id);
            //admin can do everytjing with everyone
            addPermission(permisssions,1,client_id);
            pthread_mutex_unlock(&mutex_lock);
            if (send(clientSockets[request.client_id],request.message.c_str(),request.message.length(),0)<0)
            {
                printf("blad");
            }
            
        }
        else if (request.message =="SHOW_MY_PERMISSIONS"){
            pthread_mutex_lock(&mutex_lock);
            string message = showPermission(permisssions,request.client_id);
            if (send(clientSockets[request.client_id],message.c_str(),message.length(),0)<0)
                {
                    printf("blad");
                }
            pthread_mutex_unlock(&mutex_lock);
        }
        
        else if (request.message =="SHOW_CLIENTS")
        {
            pthread_mutex_lock(&mutex_lock);

           strcpy(client_message, showAllClients(activeClients).c_str());
            if (send(clientSockets[request.client_id],client_message,strlen(client_message),0)<0)
            {
                printf("send failed");
            }
            pthread_mutex_unlock(&mutex_lock);
        }
        else if (request.message == "ADD_PERMISSION")
        { 
              pthread_mutex_lock(&mutex_lock);
              if (request.client_id==1)
              {
                    addPermission(permisssions,request.receiver_id,request.receiver_id_permission);
                    string message = showPermission(permisssions,request.receiver_id);
                    if (send(clientSockets[request.client_id],message.c_str(),message.length(),0)<0)
                    {
                        printf("blad");
                    }
            
              }
              else{
                string error = "You do not have permission for this\n";
                if (send(clientSockets[request.client_id],error.c_str(),error.length(),0)<0)
                    {
                        printf("blad");
                    }
              }
              
            
            pthread_mutex_unlock(&mutex_lock);
        }
        else if (request.message == "SHUTDOWN")
        {
            if (hasPermission(permisssions,request.client_id,request.receiver_id))
            {
                if (send(clientSockets[request.receiver_id],request.message.c_str(),request.message.length(),0)>=0)
                {
                    string successShutdown = "Shutdown done successfully";
                    send(clientSockets[request.client_id],successShutdown.c_str(),successShutdown.length(),0);
                }

                
                
            }
            else{
                string no_permission_shutdown = "You do not have rights to do this";
                    send(clientSockets[request.client_id],no_permission_shutdown.c_str(),no_permission_shutdown.length(),0);
            }
            
        }
        
        

        
        
        memset(&client_message, 0, sizeof(client_message));
    }

    
    pthread_mutex_lock(&mutex_lock);
    deleteClient(clientSockets, client_id);
    makeClientInactive(activeClients, client_id, true);

    pthread_mutex_unlock(&mutex_lock);

    close(newSocket);
    

    pthread_exit(NULL);
}

int main()
{
    int serverSocket, newSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;

    // Create the socket.
    serverSocket = socket(PF_INET, SOCK_STREAM, 0);

    // Configure settings of the server address struct
    // Address family = Internet
    serverAddr.sin_family = AF_INET;

    // Set port number, using htons function to use proper byte order
    serverAddr.sin_port = htons(1100);

    // Set IP address to localhost
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Set all bits of the padding field to 0
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    // Bind the address struct to the socket
    bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    // Listen on the socket
    if (listen(serverSocket, 50) == 0)
        printf("Listening\n");
    else
        printf("Error\n");
    pthread_t thread_id;
   
    while (1)
    {
        addr_size = sizeof serverStorage;
        newSocket = accept(serverSocket, (struct sockaddr *)&serverStorage, &addr_size);
        printf("%d\n", newSocket);
        // pthread_mutex_lock(&mutex_lock);

        // pthread_mutex_unlock(&mutex_lock);

        if (pthread_create(&thread_id, NULL, socketThread, &newSocket) != 0)
            printf("Failed to create thread\n");

        pthread_detach(thread_id);
    }
    return 0;
}
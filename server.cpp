#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#include <unistd.h> 
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
    pthread_mutex_lock(&mutex_lock);
     if (clientPermissions.find(client) != clientPermissions.end()) {
       
        if (find(clientPermissions[client].begin(), clientPermissions[client].end(), permission_to) == clientPermissions[client].end()) {
            
            clientPermissions[client].push_back(permission_to);
        } 
    } else {

        clientPermissions[client].push_back(permission_to);
    }
    pthread_mutex_unlock(&mutex_lock);
}
bool hasPermission(const map<int, vector<int> > &clientPermissions, int client, int permission_to)
{
    pthread_mutex_lock(&mutex_lock);
    auto clientIterator = clientPermissions.find(client);
    if (clientIterator != clientPermissions.end()) {
        const vector<int> &permissions = clientIterator->second;
        for (int permission : permissions) {
            if (permission == permission_to) {
                pthread_mutex_unlock(&mutex_lock);
                return true;  
            }
        }
    }
    pthread_mutex_unlock(&mutex_lock);
    return false;  
}

void printMap(const map<int, int> &myMap)
{   
    pthread_mutex_lock(&mutex_lock);
    for (const auto &entry : myMap)
    {
        cout << "Key: " << entry.first << ", Value: " << entry.second << std::endl;
    }
    pthread_mutex_unlock(&mutex_lock);
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
    pthread_mutex_lock(&mutex_lock);
    auto it = myMap.find(client);
    if (it != myMap.end())
    {
        myMap.erase(it);
        cout << "client deleted\n";
    }
    else
    {
       cout << "no client with that id\n";
    }
    pthread_mutex_unlock(&mutex_lock);
}
void makeClientInactive(map<int, bool> &myMap, const int &client, bool deletion)

{
    pthread_mutex_lock(&mutex_lock);
    if (deletion)
    {
        auto it = myMap.find(client);
        if (it != myMap.end())
        {
            myMap.erase(it);
            cout << "client deleted\n";
        }
        else
        {
           cout << "no client with that id\n";
        }
    }
    else
    {
    }
    pthread_mutex_unlock(&mutex_lock);
}
// string showAllClients(map<int, bool> &activeClients){
//     pthread_mutex_lock(&mutex_lock);
//     stringstream keysStream;
//     keysStream << "List of clients:\n";
//     for (const auto& pair : activeClients) {
//             keysStream << "Client: " << pair.first << ", ";
//         }
//         pthread_mutex_unlock(&mutex_lock);
//     return keysStream.str();
// }
string showAllClients(map<int, bool> &activeClients){
    pthread_mutex_lock(&mutex_lock);
    stringstream keysStream;
    keysStream << "[";
    for (auto it = activeClients.begin(); it != activeClients.end(); ++it) {
        keysStream << "Client" << it->first;
        if (next(it) != activeClients.end()) {
            keysStream << ", ";
        }
    }
    keysStream << "]";
    pthread_mutex_unlock(&mutex_lock);
    return keysStream.str();
}
string showPermission(map<int, vector<int> > &clientPermissions, int client){
    pthread_mutex_lock(&mutex_lock);
    stringstream permissionStream;
    

    vector <int> permissions = clientPermissions[client];
    pthread_mutex_unlock(&mutex_lock);
    int size = permissions.size();
    permissionStream <<"Available to shutdown for client: "<<client << ":[ ";
    for (int i = 0; i < size; i++)
    {
        permissionStream<< permissions[i];
         if (i!=size-1)
            {
            permissionStream<<",";
            }
    
    }
    permissionStream <<"]";
 
 
    return permissionStream.str();
}
void *socketThread(void *arg)
{
   
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
        
        cout << request.message << "\n";
        
        if (request.message == "REGISTER")
        {   
           
            addPermission(permisssions,client_id,client_id);
            //admin can do everytjing with everyone
            addPermission(permisssions,1,client_id);
         
            if (send(clientSockets[request.client_id],request.message.c_str(),request.message.length(),0)<0)
            {
                perror("Send failed: ");
            }
            
        }
        else if (request.message =="SHOW_MY_PERMISSIONS"){
         
            string message = showPermission(permisssions,request.client_id);
            if (send(clientSockets[request.client_id],message.c_str(),message.length(),0)<0)
                {
                    perror("Send failed: ");
                }
          
        }
        
        else if (request.message =="SHOW_CLIENTS")
        {
           
           strcpy(client_message, showAllClients(activeClients).c_str());
           
            if (send(clientSockets[request.client_id],client_message,strlen(client_message),0)<0)
            {
                perror("Send failed: ");
            }
          
        }
        else if (request.message == "ADD_PERMISSION")
        { 
            
              if (request.client_id==1)
              {
                    addPermission(permisssions,request.receiver_id,request.receiver_id_permission);
                    string message = showPermission(permisssions,request.receiver_id);
                    if (send(clientSockets[request.client_id],message.c_str(),message.length(),0)<0)
                    {
                       perror("Send failed: ");
                    }
            
              }
              else{
                string error = "You do not have permission for this\n";
                if (send(clientSockets[request.client_id],error.c_str(),error.length(),0)<0)
                    {
                        perror("Send failed: ");
                    }
              }
        }
        else if (request.message == "SHUTDOWN")
        {
            if (hasPermission(permisssions,request.client_id,request.receiver_id))
            {
                if (send(clientSockets[request.receiver_id],request.message.c_str(),request.message.length(),0)>=0)
                {
                    string successShutdown = "Shutdown send successfully";
                    send(clientSockets[request.client_id],successShutdown.c_str(),successShutdown.length(),0);
                }
                else{
                    string errorShutdown = "Shutdown send failed";
                    send(clientSockets[request.client_id],errorShutdown.c_str(),errorShutdown.length(),0);
                }
            }
            else{
                string no_permission_shutdown = "You do not have rights to do this";
                send(clientSockets[request.client_id],no_permission_shutdown.c_str(),no_permission_shutdown.length(),0);
            }
            
        }
        
        

        
        
        memset(&client_message, 0, sizeof(client_message));
    }

    

    deleteClient(clientSockets, client_id);
    makeClientInactive(activeClients, client_id, true);


    close(newSocket);
    pthread_exit(NULL);
}

int main()
{
    int serverSocket, newSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;
    serverSocket = socket(PF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1100);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);


    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);


    bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));


    if (listen(serverSocket, 50) == 0)
        cout << "Listening\n";
    else
        perror("Error\n");
    pthread_t thread_id;
   
    while (1)
    {
        addr_size = sizeof serverStorage;
        newSocket = accept(serverSocket, (struct sockaddr *)&serverStorage, &addr_size);
        cout << "new client connected\n";

        if (pthread_create(&thread_id, NULL, socketThread, &newSocket) != 0)
            perror("pthread_create");

        pthread_detach(thread_id);
    }
    close(serverSocket);
    return 0;
}
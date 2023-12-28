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
using namespace std;
char client_message[2000];
char buffer[1024];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;
map<int, int> clientSockets;
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
};
Request receiveRequest(char *buff)
{
    istringstream iss(buff);
    Request request;
    iss >> request.client_id >> request.message >> request.receiver_id;
    return request;
}
void *socketThread(void *arg)
{
    printf("New thread\n");
    int newSocket = *((int *)arg);
    int n;
    printf("socket: %d", newSocket);
    for (;;)
    {
        n = recv(newSocket, client_message, sizeof(client_message), 0);

        if (n <= 0)
        {
            // Client has disconnected or an error occurred
            printf("error");
            break;
        }

        Request request;
        request = receiveRequest(client_message);
        printf("Received message from client %d: %s : %d\n ", request.client_id, request.message.c_str(), request.receiver_id);

        if (request.receiver_id != -1)
        {
            // Find the socket associated with the receiver_id
            pthread_mutex_lock(&mutex_lock);

            auto it = clientSockets.find(request.receiver_id);
            if (it != clientSockets.end())
            {
                // Send the message to the specified client
                send(it->first, client_message, n, 0);
            }
            pthread_mutex_unlock(&mutex_lock);
        }
        else
        {
            // Broadcast the message to all connected clients
            pthread_mutex_lock(&mutex_lock);
            for (auto const &client : clientSockets)
            {
                send(client.first, client_message, n, 0);
            }
            pthread_mutex_unlock(&mutex_lock);
        }

        memset(&client_message, 0, sizeof(client_message));
    }

    // Remove the client socket from the map when the client disconnects
    pthread_mutex_lock(&mutex_lock);
    auto it = clientSockets.begin();
    while (it != clientSockets.end())
    {
        if (it->first == newSocket)
        {
            it = clientSockets.erase(it);
            break;
        }
        else
        {
            ++it;
        }
    }
    pthread_mutex_unlock(&mutex_lock);

    close(newSocket);
    printf("Exit socketThread\n");

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
        printf("%d", newSocket);
        pthread_mutex_lock(&mutex_lock);

        clientSockets[newSocket] = newSocket;
        printMap(clientSockets);
        pthread_mutex_unlock(&mutex_lock);

        if (pthread_create(&thread_id, NULL, socketThread, &newSocket) != 0)
            printf("Failed to create thread\n");

        pthread_detach(thread_id);
    }
    return 0;
}
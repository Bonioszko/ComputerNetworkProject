#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h>  // for open
#include <unistd.h> // for close
#include <pthread.h>
#include <string>
#include <map>
#include <sstream>
#include <iostream>
using namespace std;
struct Request
{
    int client_id;
    string message;
    int receiver_id;
};
string makeRequest(Request request)
{
    char message[2000];
    strcpy(message, (to_string(request.client_id) + " " + request.message.c_str() + " " + to_string(request.receiver_id)).c_str());
    return message;
}

void *serverThread(void *arg)
{
    char buffer[1024];
    int clientSocket = *((int *)arg);

    for (;;)
    {
        // Wait for server response
        if (recv(clientSocket, buffer, 1024, 0) <=0)
        {
            printf("Receive failed\n");
            break;
        }

        // Print the received message
        printf("Data received: %s\n", buffer);
        memset(&buffer, 0, sizeof(buffer));

    }
   
    pthread_exit(NULL);
}
int main(int argc, char *argv[])
{

    char message[1000];
    char buffer[1024];
    int clientSocket;
    int clientId = stoi(argv[1]);
    string action;
    Request request;
    request.client_id = clientId;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;
    pthread_t serverThreadId;

    // Create the socket.
    clientSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        printf("socket");
        return 1;
    }

    // Configure settings of the server address
    // Address family is Internet
    serverAddr.sin_family = AF_INET;

    // Set port number, using htons function
    serverAddr.sin_port = htons(1100);

    // Set IP address
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
    
    // Connect the socket to the server using the address
    addr_size = sizeof serverAddr;
    connect(clientSocket, (struct sockaddr *)&serverAddr, addr_size);
    // strcpy(message,"Hello");

    int msg_scanf_size;
    request.message = "REGISTER";
    request.receiver_id = request.client_id;
   
    strcpy(message, makeRequest(request).c_str());
    if (send(clientSocket, message, strlen(message), 0) < 0)
    {
        printf("Send failed\n");
    }
    
    if (recv(clientSocket, buffer, 1024, 0) < 0)
    {
        printf("Receive failed\n");
    }
    printf("Data received: %s\n", buffer);

    memset(&message, 0, sizeof(message));
    memset(&buffer, 0, sizeof(buffer));
  
    if (pthread_create(&serverThreadId, NULL, serverThread, (void *)&clientSocket) != 0)
    {
        perror("pthread_create");
        close(clientSocket);
        return 1;
        
    }
    for (;;)
    {
        cout << "please enter action to do:\n "<<
        "1.exit 2.show admins 3. show clients \n "<<
        "4.shutdown 5.add admins\n";
        // printf("please enter action to do: \n 1. exit, 2\n");
        cin >> action;
        // msg_scanf_size = scanf("%s", message);
        if (action =="1")
        {
            break;
        }
        else if(action =="2"){
            strcpy(message,"SHOW_ADMINS");
            request.receiver_id = request.client_id;
            request.message = message;
            strcpy(message, makeRequest(request).c_str());
            printf("%d\n", clientSocket);
            if (send(clientSocket, message, strlen(message), 0) < 0)
                {
                    printf("Send failed\n");
                }
        }  
        else if(action =="3"){
            strcpy(message,"SHOW_CLIENTS");
            request.receiver_id = request.client_id;
            request.message = message;
            strcpy(message, makeRequest(request).c_str());
            printf("%d\n", clientSocket);
            if (send(clientSocket, message, strlen(message), 0) < 0)
                {
                    printf("Send failed\n");
                }
        }
        else if(action =="4"){

            strcpy(message,"SHUTDOWN");
             if (scanf("%d", &request.receiver_id) != 1)
            {
                printf("Invalid input. Please enter an integer.\n");
                return 1;
            }
          
            request.message = message;
            strcpy(message, makeRequest(request).c_str());
            printf("%d\n", clientSocket);
            if (send(clientSocket, message, strlen(message), 0) < 0)
                {
                    printf("Send failed\n");
                }
        }
         else if(action =="5"){

            strcpy(message,"ADD_ADMIN");
             if (scanf("%d", &request.receiver_id) != 1)
            {
                printf("Invalid input. Please enter an integer.\n");
                return 1;
            }
          
            request.message = message;
            strcpy(message, makeRequest(request).c_str());
            printf("%d\n", clientSocket);
            if (send(clientSocket, message, strlen(message), 0) < 0)
                {
                    printf("Send failed\n");
                }
        }
        else{
            cout << "choose valid action\n";
            return 1;
        }

    
        
       
        // char *s;
        // s = strstr(message, "exit");
        // if (s != NULL)
        // {
        //     printf("Exiting\n");
        //     break;
        // }
        //
        // ponizej do dokomentowania jakby thread nie dzialal

        // printf("Waiting\n");
    
        // if (recv(clientSocket, buffer, 1024, 0) < 0)
        // {
        //     printf("Receive failed\n");
        // }
        // // Print the received message
        // printf("Data received: %s\n", buffer);

        // memset(&message, 0, sizeof(message));
        // memset(&buffer, 0, sizeof(buffer));
    }

    close(clientSocket);

    return 0;
}
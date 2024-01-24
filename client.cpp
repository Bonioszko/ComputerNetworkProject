#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h>  
#include <unistd.h> 
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
    int receiver_id_permission;
};

string makeRequest(Request request)
{
    char mess_request[1000];
    strcpy(mess_request, (to_string(request.client_id) + " " + request.message.c_str() + " " + to_string(request.receiver_id)+ " " + to_string(request.receiver_id_permission)).c_str());
    return mess_request;
}
bool server_on=false;
void *serverThread(void *arg)
{
    char buffer[1024];
    int serverSocket = *((int *)arg);
    server_on=true;
    for (;;)
    {
    
        if (recv(serverSocket, buffer, 1024, 0) <=0)
        {
            cout << "Receive failed\n";
            break;
        }
       if (strcmp(buffer, "SHUTDOWN") == 0)
        {
        //    system("shutdown -h now");
            system("echo 'Test command executed\n'");
         
           
        }
    cout<< "Data received: " << buffer << "\n";
        memset(&buffer, 0, sizeof(buffer));
    

    }
    server_on=false;
    cout<< "server thread exit, click anything to close client\n";
    close(serverSocket);
    pthread_exit(NULL);
}
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "U need to enter client id\n";
        return 1;
    }
    
    char message[1000];
    char buffer[1000];
    int clientSocket;
    int clientId = stoi(argv[1]);
    string action;
    Request request;
    request.client_id = clientId;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;
    pthread_t serverThreadId;

    clientSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
       perror("Could not create socket");
        return 1;
    }
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1100);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
    addr_size = sizeof serverAddr;
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, addr_size) < 0) {
        perror("Connection failed");
        return -1;
    }
    memset(&message, 0, sizeof(message));
    int msg_scanf_size;
    request.message = "REGISTER";
    request.receiver_id = request.client_id;
    strcpy(message, makeRequest(request).c_str());
    if (send(clientSocket, message, strlen(message), 0) < 0)
    {
        perror("Send failed: ");
        close(clientSocket);
        return 1;
    }
    
    if (pthread_create(&serverThreadId, NULL, serverThread, (void *)&clientSocket) != 0)
    {
        perror("Create thread failed: ");
        close(clientSocket);
        return 1;
    }
    pthread_detach(serverThreadId);
    for (;;)
    {   
     
        sleep(1);
        if (!server_on)
        {
            cout << "server is down\n"; 
            break;
        }
        cout << "please enter action to do:\n "<<
        "1.exit 2. Show my permissions 3. show clients \n "<<
        "4.shutdown 5.add permission\n";
        cin >> action;
        if (action =="1")
        {
            break;
        }
        else if(action =="2"){
            strcpy(message,"SHOW_MY_PERMISSIONS");
            request.receiver_id = request.client_id;
            request.message = message;
            strcpy(message, makeRequest(request).c_str());
           
            if (send(clientSocket, message, strlen(message), 0) < 0)
                {
                    perror("Send failed: ");
                }
        }  
        else if(action =="3"){
            strcpy(message,"SHOW_CLIENTS");
            request.receiver_id = request.client_id;
            request.message = message;
            strcpy(message, makeRequest(request).c_str());
            
            if (send(clientSocket, message, strlen(message), 0) < 0)
                {
                    perror("Send failed: ");
                }
        }
        else if(action =="4"){

            strcpy(message,"SHUTDOWN");
            cout<< "Enter which client you want to shutdown: ";
             if (scanf("%d", &request.receiver_id) != 1)
            {
                cout << "Invalid input. Please enter an integer.\n";
                continue;
            }
            request.message = message;
            strcpy(message, makeRequest(request).c_str());
           
            if (send(clientSocket, message, strlen(message), 0) < 0)
                {
                   perror("Send failed: ");
                }
        }
         else if(action =="5"){

            strcpy(message,"ADD_PERMISSION");
            cout << "which client do you want to add permission to: ";

             if (scanf("%d", &request.receiver_id) != 1)
            {
                cout << "Invalid input. Please enter an integer.\n";
                continue;
            }
                        cout << "to which client: ";
            if (scanf("%d", &request.receiver_id_permission) != 1)
            {
                cout << "Invalid input. Please enter an integer.\n";
                continue;
            }
          
            request.message = message;
            strcpy(message, makeRequest(request).c_str());
            
            if (send(clientSocket, message, strlen(message), 0) < 0)
                {
                    perror("Send failed: ");
                }
        }
        else{
            cout << "choose valid action\n";
            continue;
        }
    }
    close(clientSocket);
    return 0;
}
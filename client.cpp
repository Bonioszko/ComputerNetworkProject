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
int main(int argc, char *argv[])
{

    char message[1000];
    char buffer[1024];
    int clientSocket;
    int clientId = stoi(argv[1]);
    Request request;
    request.client_id = clientId;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;

    // Create the socket.
    clientSocket = socket(PF_INET, SOCK_STREAM, 0);

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

    for (;;)
    {
        printf("Please enter a message: ");
        msg_scanf_size = scanf("%s", message);
        if (scanf("%d", &request.receiver_id) != 1)
        {
            printf("Invalid input. Please enter an integer.\n");
            return 1;
        }

        char *s;
        s = strstr(message, "exit");
        if (s != NULL)
        {
            printf("Exiting\n");
            break;
        }
        request.message = message;
        strcpy(message, makeRequest(request).c_str());
        printf("%s\n", message);
        if (send(clientSocket, message, strlen(message), 0) < 0)
        {
            printf("Send failed\n");
        }

        // Read the message from the server into the buffer
        if (recv(clientSocket, buffer, 1024, 0) < 0)
        {
            printf("Receive failed\n");
        }
        // Print the received message
        printf("Data received: %s\n", buffer);

        memset(&message, 0, sizeof(message));
    }

    close(clientSocket);

    return 0;
}
#include <iostream>
#include <map>
#include <vector>
using namespace std;
void addPermission(map<int, std::vector<int>> &clientPermissions, int client, int permission_to)
{
    clientPermissions[client].push_back(permission_to);
}
bool hasPermission(map<int, vector<int>> &clientPermissions, int client, int permission_to)
{
    // Check if the client exists in the map
    auto clientIterator = clientPermissions.find(client);
    if (clientIterator != clientPermissions.end())
    {
        // Check if the permission_to value is in the vector associated with the client
        const std::vector<int> &permissions = clientIterator->second;
        for (int permission : permissions)
        {
            if (permission == permission_to)
            {
                return true; // Client has the specified permission
            }
        }
    }

    return false; // Client does not have the specified permission
}

int main()
{
    // Example usage:
    std::map<int, std::vector<int>> clientPermissions;

    // Add permissions for clients
    addPermission(clientPermissions, 1, 100);
    addPermission(clientPermissions, 1, 200);
    addPermission(clientPermissions, 2, 100);

    // Check if a client has a specific permission
    int clientToCheck = 2;
    int permissionToCheck = 100;

    if (hasPermission(clientPermissions, clientToCheck, permissionToCheck))
    {
        std::cout << "Client " << clientToCheck << " has permission " << permissionToCheck << std::endl;
    }
    else
    {
        std::cout << "Client " << clientToCheck << " does not have permission " << permissionToCheck << std::endl;
    }

    return 0;
}

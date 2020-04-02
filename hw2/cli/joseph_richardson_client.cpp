/* Joseph Keegan Richardson
 * 1678875
 * COSC3360 Mon/Wed 4-5:30
 * Due: 04/01/2020
*/

// This is the client that connects to the server.
// Asks for address, port, and city.
// Port and address must be the same as the server.
// City is case sensitive.
// Client shuts down after message is recieved and printed.
// Example input run on a single machine
// 	127.0.0.1
// 	8080
// 	Houston

#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h>
#include <iostream>
#include <limits>

using namespace std;
   
int main(int argc, char const *argv[]) 
{ 
    //Client setup
	char host[256]={0};
	int port;
	char city[256]={0};

	cout << "Enter the server host name: ";
	cin.getline(host, 256);
	while(cout << "Enter the server port number: " && !(cin >> port)){
		cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		cout << "Invalid input\n";
	}
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	cout << "Enter a city name: ";
	cin.getline(city, 256);


    //Set up socket
        int sock = 0, valread; 
        struct sockaddr_in serv_addr; 
        char buffer[1024] = {0}; 
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        { 
            printf("\n Socket creation error \n"); 
            return -1; 
        } 
   
        serv_addr.sin_family = AF_INET; 
        serv_addr.sin_port = htons(port); 
       
        // Convert IPv4 and IPv6 addresses from text to binary form 
        if(inet_pton(AF_INET, host, &serv_addr.sin_addr)<=0)  
        { 
            printf("\nInvalid address/ Address not supported \n"); 
            return -1; 
        } 
   
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
        { 
            printf("\nConnection Failed \n"); 
            return -1; 
        }

        //Client message and printing
        send(sock , city , strlen(city) , 0 ); 
        valread = read( sock , buffer, 1024); 
        printf("%s",buffer ); 

	return 0; 
} 

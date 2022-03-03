/* 
 * COSC3360 Mon/Wed 4-5:30
 * Due: 04/01/2020
*/

// This is the server program that responds to the message recieved by the client.
// The server holds and creates all the data.
// The client message is case sensitive.
// Server stays alive until manually shut down.
// Example input to match client
// 	8080

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <sstream>
#include <unistd.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <limits>

using namespace std;

vector<vector<string>> makeData(vector<string> a);
vector<string> readFile();
vector<string> weatherReport(string city, vector<vector<string>> data);

int main(){

	//Set up server data
	
	//Socket variables
	int port;

	//Server variables
	vector<string> raw_data;
	vector<vector<string>> data;
	vector<string> report;


        while(cout << "Enter the port number: " && !(cin >> port)){
                 cin.clear();
                 std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                 cout << "Invalid input\n";
	}

	//read data & make data
	raw_data = readFile();
	data = makeData(raw_data);
	

	//Create socket
	
	//socket variables
        int server_fd, new_socket, valread; 
        struct sockaddr_in address; 
        int opt = 1; 
        int addrlen = sizeof(address); 
        char buffer[1024] = {0}; 
       
        // Creating socket file descriptor 
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
        { 
            perror("socket failed"); 
            exit(EXIT_FAILURE); 
        } 
       
        // Forcefully attaching socket to the port 
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                      &opt, sizeof(opt))) 
        { 
            perror("setsockopt"); 
            exit(EXIT_FAILURE); 
        } 
        address.sin_family = AF_INET; 
        address.sin_addr.s_addr = INADDR_ANY; 
        address.sin_port = htons( port ); 
       
        // Forcefully attaching socket to the port 
        if (bind(server_fd, (struct sockaddr *)&address,  
                                     sizeof(address))<0) 
        { 
            perror("bind failed"); 
            exit(EXIT_FAILURE); 
        }
	
	//Loop for the server
        while(true){
        	if (listen(server_fd, 3) < 0) 
        	{ 
            	perror("listen"); 
            	exit(EXIT_FAILURE); 
        	} 
        	if ((new_socket = accept(server_fd, (struct sockaddr *)&address,  
                	           (socklen_t*)&addrlen))<0) 
        	{ 
            	perror("accept"); 
            	exit(EXIT_FAILURE); 
        	}

        	//Server
        	valread = read( new_socket , buffer, 1024);
        	report = weatherReport(buffer, data);
        	const char *c;
        	char holder[256];
        	memset(holder, 0, sizeof(holder));
        	for(int i = 0; i < report.size(); i++){
    	    		strcat(holder, report.at(i).c_str());
        	}
        	c = holder;
        	send(new_socket , c , strlen(c) , 0 ); 
        	memset(buffer, 0, sizeof(buffer));
        }

	return 0;

}

//Creates raw data from the input file so makeData can use it
//The file is read line by line and put into a vector
vector<string> readFile(){
	ifstream file("weather20.txt");
        vector<string> vect;
	for(string line; getline(file, line);){
		vect.push_back(line);
	}

	file.close();
        return vect;
}
//Creates data from the raw data
//Splits lines and puts into its respective vectors
vector<vector<string>> makeData(vector<string> a){
	vector<vector<string>> vect(a.size());
	for(int i = 0 ; i < a.size(); i++){
		stringstream buffer(a.at(i));
		while(buffer.good()){
			string sub;
			getline(buffer, sub, ',');
			vect.at(i).push_back(sub);
		}
	}
	return vect;
}
//weatherReport creates the message to send back to the client
//Doesn't test for different cases of the city name
vector<string> weatherReport(string city, vector<vector<string>> data){
	bool found = false;
	vector<string> msg(1);
	for(int i = 0; i < data.size(); i++){
		if(data.at(i).at(0).compare(city) == 0){
			found = true;
			msg.resize(2);
			msg.at(0) = "Tomorrow's maximum temperature is " + data.at(i).at(1) + " F\n";
			msg.at(1) = "Tomorrow's sky condition is " + data.at(i).at(2) + "\n";
			cout << "Weather report for " + city + "\n";
			cout << msg.at(0);
			cout << msg.at(1);
			break;
		}
	}
	if(found == false){
		msg.resize(1);
		msg.at(0) = "No Data\n";
		cout << "Weather report for " + city + "\n";
		cout << msg.at(0);
	}
	return msg;
}

/*
 *	COSC3360 Mon/Wed 4-5:30
 *	Due 02/19/2020
*/

/*
 * This program runs a loop of a processes. There are 3 checkpoints in the process: Core, SSD, and TTY.
 * Core simulates the CPU of a computer, SSD simulated an SSD and TTY simulates user interface.
 * This program deals with program processes that go through each of these stages for a certain time.
 * Core leads to SSD and TTY. SSD leads to core. TTY leads to Core.
 * Output of this program produces process simulation in data tables stating the time started, ended and so on.
 * Output also includes near the end with the total simulation time, number of processes completed, number of ssd accesses, average number of busy cores, and SSD utiliaztion
 *
 * Classes Core, SSD, and TTY are very similar in the structure can you can almost assume they do the same things.
*/

//used
#include <iostream>
#include <iomanip>
#include <stdio.h>
//#include <conio.h>
#include <queue>
#include <vector>
#include <string>

using namespace std;

//Function and class declaration
class a_Core;
class SSD;
class TTY;
class ProcID;
vector<ProcID*> scheduleTasks(vector<vector<string>> a);
vector<vector<string>> readInput();
int readSSD(vector<string> a);
int timea;

//Holds all the data for the processes. Each process has its own object
class ProcID {
	private:
		vector<string> process;
		vector<int> time;
		bool isInteractive;
		int startTime;
		bool isEmpty;
		int id;

		string status;
		int timeLeft;
		bool done;//For when the process is fully completed
	public:
		//Get methods
		int getTime() { return time[0]; }
		int getStartTime() { return startTime; }
		string getProcess() { return process[0]; }
		void getNextProcess(){
			//Check if exactly 1 element is left
			if(process.size() == 1){
				done = true;
			} else {
			process.erase(process.begin());
			time.erase(time.begin());
			}
		}
		bool empty() { return isEmpty; }
		bool getInteractive() { return isInteractive; }
		int getID() { return id; }
		//Set methods
		void setStart(int t) { startTime = t; }
		void setInteractive(bool i) { isInteractive = i; }
		void addProcess(string p) { process.push_back(p); }
		void addTime(int t) { time.push_back(t); }
		void setID(int i) { id = i; }

		string getStatus(){ return status; }
		void setStatus(string s){ status = s; }
		int getTimeLeft(){ return timeLeft; }
		void setTimeLeft(int t){ timeLeft = t; }
		void increment(){ timeLeft -= 1; }
		bool isDone(){ return done; }
};

//Manages the SSD part of the program. Has a check loop to determine the next steps needed.
class SSD {
	private:
		queue<ProcID*> ssdqueue;
		bool inUse;
		ProcID* usingSSD;
		ProcID* holder;
		int ssdfree;
	public:
		int getssdfree(){
			return ssdfree;
		}
		bool check(){
			bool done = false;

			if(inUse && usingSSD->getTimeLeft() == 0){
				done = true;
				holder = usingSSD;
				holder->setStatus("READY");
				holder->getNextProcess();
				if(!ssdqueue.empty()){
					usingSSD = ssdqueue.front();
					usingSSD->setTimeLeft(usingSSD->getTime());
					ssdqueue.pop();
					//increment???
				} else {
					inUse = false;
					//usingSSD = new ProcID();
					ssdfree += 1;
				}

			} else if(inUse){
				usingSSD->increment();
			} else {
				if(!ssdqueue.empty()){
					usingSSD = ssdqueue.front();
					ssdqueue.pop();
				} else {
					ssdfree += 1;
				}
			}
			return done;
		}
		//ssd_request requests to use the ssd. If the ssd is busy it's get added to the ssdqueue, otherwise the process gets put into the ssd
		void ssd_request(ProcID* id){
			//cout << "Process " << id->getID() << " requests SSD access at time " << timea << " ms for " << id->getTime() << " ms.\n";
			id->setInteractive(false);
			id->setTimeLeft(id->getTime());
			if(inUse == false && ssdqueue.empty()){
				//cout << "Process " << id->getID() << " will release the SSD at time " << (timea + id->getTime()) << " ms.\n";
				inUse = true;
				id->setStatus("RUNNING");
				usingSSD = id;
				//time calculations
			} else {
				id->setStatus("READY");
				ssdqueue.push(id);
			}
		}
		//ssd_release releases the item done that finished in the ssd
		ProcID* ssd_release(){//ProcID ussd){
			//move to core
			//cout << "SSD completion event for process " << holder->getID() << " at time " << timea << " ms.\n";

			return holder;
		}
};

//Class TTY manages the tty section of the program. It has a vector of all the running processes and releases a queue whenever a process(es) is finished.
class TTY {
	private:
		vector<ProcID*> ttyqueue;
		queue<ProcID*> finished;
	public:
		//Main loop of the program. Returns true if a process is done
		bool check(){
			bool found = false;

			//Empty queue just incase
			while(!finished.empty()){
				finished.pop();
			}

			for(int i = 0; i < ttyqueue.size(); i++){
				if(ttyqueue[i]->getTimeLeft() == 0){
					//cout << "TTY completion event for process " << ttyqueue[i]->getID() << " at time " << timea << " ms.\n";
					ttyqueue[i]->setStatus("READY");
					finished.push(ttyqueue[i]);
					finished.back()->getNextProcess();
					ttyqueue.erase(ttyqueue.begin() + i);
					i--;
					found = true;
				} else {
					ttyqueue[i]->increment();
				}
			}
			return found;
		}
		//Used to request the tty. Puts the item in the running vector
		void user_request(ProcID* id){
			//cout << "Process " << id->getID() << " will interact with a user at time " << timea << " ms for " << id->getTime() << " ms.\n";
			id->setInteractive(true);
			id->setStatus("BLOCKED");
			id->setTimeLeft(id->getTime());
			//id->increment();//MAYBE NEED TO DELETE
			ttyqueue.push_back(id);
		}
		//Returns the queue of done processes
		queue<ProcID*> user_release(){
			//cout << "Relasing TTY with size " << finished.size() << "\n";
			return finished;
		}
};

//a_Core manages the core section of the project.
class a_Core {
	private:
		vector<ProcID*> interactive;
		vector<ProcID*> nonInteractive;
		vector<ProcID*> usingCore;
		int nCores;
		int nfreecores;
		int corefree;

		queue<ProcID*> finished;
		SSD *s;
		TTY *t;
	public:
		int getNumCores(){
			return nCores;
		}
		int getcorefree(){
			return corefree;
		}
		void init(SSD *a, TTY *b){
			s = a;
			t = b;
			corefree = 0;
		}
		//Used for adding new arival items to the core
		void additem(ProcID* a){
			a->setStatus("READY");
			a->setInteractive(false);
			a->setTimeLeft(a->getTime());
			nonInteractive.push_back(a);
		}
		//Main loop of a_Core. Checks if a process is completed
		bool check(){
			bool done = false;

			//Empty of finished queue just incase
			while(!finished.empty()){
				finished.pop();
			}
			
			//If a core is free and one of the queues has an item in it push that item to a core.
			while(nfreecores > 0 && (nonInteractive.size() > 0 || interactive.size() > 0)){
				if(!interactive.empty()){
					//cout << "EMPTYING INTERACTIVE QUEUE\n";
					usingCore.push_back(interactive.front());
					usingCore.back()->setStatus("RUNNING");
					interactive.erase(interactive.begin());
					nfreecores--;
					//cout << "Process " << usingCore.back()->getID() << " requests a core at time " << timea << " ms for " << usingCore.back()->getTime() << " ms\n";
					//cout << "Process " << usingCore.back()->getID() << " will release a core at time " << (timea + usingCore.back()->getTime()) << " ms.\n";
				} else if(!nonInteractive.empty()){
					//cout << "EMPTYING NONINTERACTIVE QUEUE\n";
					usingCore.push_back(nonInteractive.front());
					usingCore.back()->setStatus("RUNNING");
					nonInteractive.erase(nonInteractive.begin());
					nfreecores--;
				//cout << "Process " << usingCore.back()->getID() << " requests a core at time " << timea << " ms for " << usingCore.back()->getTime() << " ms\n";
				//cout << "Process " << usingCore.back()->getID() << " will release a core at time " << (timea + usingCore.back()->getTime()) << " ms.\n";
				}
			}
			
			corefree += nfreecores;

			//Check if a process is done. Else increment time
			for(int i = 0; i < usingCore.size(); i++){
				if(usingCore[i]->getTimeLeft() == 0){
					usingCore[i]->setStatus("READY");
					usingCore[i]->getNextProcess();
					//cout << "CORE completion event for process " << usingCore[i]->getID() << " at time " << timea << " ms\n";
						finished.push(usingCore[i]);
						usingCore.erase(usingCore.begin() + i);
						i--;
						if(!interactive.empty()){//Empty out queues
							//cout << "EMPTYING INTERACTIVE QUEUE\n";
							usingCore.push_back(interactive.front());
							usingCore.back()->setStatus("RUNNING");
							//usingCore.back()->setTimeLeft(usingCore.back()->getTime());
							interactive.erase(interactive.begin());
							//cout << "Process " << usingCore.back()->getID() << " requests a core at time " << timea << " ms for " << usingCore.back()->getTime() << " ms\n";
							//cout << "Process " << usingCore.back()->getID() << " will relase a core at time " << (timea + usingCore.back()->getTime()) << " ms.\n";
						} else if(!nonInteractive.empty()){
							//cout << "EMPTYING NONINTERACTIVE QUEUE\n";
							usingCore.push_back(nonInteractive.front());
							usingCore.back()->setStatus("RUNNING");
							//usingCore.back()->setTimeLeft(usingCore.back()->getTime());
							nonInteractive.erase(nonInteractive.begin());
						//cout << "Process " << usingCore.back()->getID() << " requests a core at time " << timea << " ms for " << usingCore.back()->getTime() << " ms\n";
						//cout << "Process " << usingCore.back()->getID() << " will relase a core at time " << (timea + usingCore.back()->getTime()) << " ms.\n";
						} else {
							nfreecores++;
						}
						done = true;
				} else {
					usingCore[i]->increment();
				}
			}

			return done;
		}
		void setCores(int n){
			nCores = n;
			nfreecores = nCores;
		}
		//When an item requests a core it checks the interactivity and moves it to its respective queues
		void core_request(ProcID* id){
			//cout << "Process " << id->getID() << " requests a core at time " << timea << " ms for " << id->getTime() << " ms\n";
			if(nfreecores > 0){
				nfreecores--;
				usingCore.push_back(id);
				usingCore.back()->setTimeLeft(usingCore.back()->getTime());
				usingCore.back()->setStatus("RUNNING");
				usingCore.back()->increment();
			} else {
				id->setStatus("READY");
				id->setTimeLeft(id->getTime());
				//cout << "Process " << id->getID() << " must wait for a core.\n";
				if(id->getInteractive() == true){
					interactive.push_back(id);
					//cout << "I Queue now contains " << interactive.size() << " process(es) waiting for a core.\n";
				} else {
					nonInteractive.push_back(id);
					//cout << "NI Queue now contains " << nonInteractive.size() << " process(es) waiting for a core.\n";
				}
				//for(int i = 0; i < usingCore.size(); i++){
				//	cout << "THE CORE IS RUNNING PROCESS " << usingCore[i]->getID();
				//}
				//cout <<"\n CORE SIZE " << usingCore.size() << " AND CORES FREE " << nfreecores << "\n";
			}	
		}
		//Releases a queue of finished Processes
		queue<ProcID*> core_release(){
			return finished;
		}
};

//Manages the core, ssd, tty. Runs the entire program
class EventManager {
	private:
		a_Core* c;
		SSD* s;
		TTY* t;
		bool running;
		vector<ProcID*>activeProc;
		queue<ProcID*>inactiveProc;
		queue<ProcID*>ssdholder;//pickup whenever a process at ssd finishes
		queue<ProcID*>ttyholder;//pickup whenever a processes at tty finishes
		queue<ProcID*>coreholder;//pickup whenever a process at core finishes
		int procDone;

	public:
		void init(a_Core* d, SSD* a, TTY* b){
			c = d;
			s = a;
			t = b;
			procDone = 0;
		}
		void print(){
			cout << "Process Table:\n";
			if(activeProc.empty()){
				cout << "There are no active processes.\n";
			} else {
				for(int i = 0; i < activeProc.size(); i++){
					cout << "Process " << activeProc[i]->getID() << " is " << activeProc[i]->getStatus() << "\n";
				}
			}
			cout << "\n";

		}
		int getTime(){
			return timea;
		}
		//Main loop of the program. Inserts items when their time arrives. Checks the core, checks the ssd and checks the tty
		void runProgram(){
			cout << "\n";
			running = true;
			timea = 0;
			while(running){
				running = false;
				//Inserts items into core when their start time arrives
				while(!inactiveProc.empty() && inactiveProc.front()->getStartTime() == timea){
					//cout << "ARRIVAL event for process " << inactiveProc.front()->getID() << " at time " << timea << " ms.\n";
					cout << "Process " << inactiveProc.front()->getID() << " starts at time " << timea << " ms\n";
					print();
					activeProc.push_back(inactiveProc.front());
					inactiveProc.pop();
					activeProc.back()->setStatus("READY");
					activeProc.back()->setInteractive(false);
					c->additem(activeProc.back());
				}
				
				//check if processes is done
				if(c->check() == true){
					//Empty out queue just incase
					while(coreholder.size() > 0){
						coreholder.pop();
					}
					coreholder = c->core_release();			
				}
				//Check if items done in core are finished in the program or move to SSD or TTY
				while(!coreholder.empty()){
					//If the process is done
					if(coreholder.front()->isDone()){
						cout << "Process " << coreholder.front()->getID() << " terminates at time " << timea << " ms.\n";
						coreholder.front()->setStatus("TERMINATED");
						print();
						procDone++;
						//Removes the process from event manager
						for(int i = 0; i < activeProc.size(); i++){
							if(activeProc[i]->getID() == coreholder.front()->getID()){
								activeProc.erase(activeProc.begin() + i);
								i--;
							}
						}
						coreholder.pop();
					} else {
						//If next item is SSD or TTY
						if(coreholder.front()->getProcess().compare("SSD") == 0){
							s->ssd_request(coreholder.front());
							coreholder.pop();
						} else {
							t->user_request(coreholder.front());
							coreholder.pop();
						}
					}
				}
				//Empty of ssd queue just incase
				while(!ssdholder.empty()){
					ssdholder.pop();
				}

				//Check ssd if a process is done
				if(s->check() == true){
					ssdholder.push(s->ssd_release());
				}
				//Move ssd item back to core
				while(!ssdholder.empty()){
					c->core_request(ssdholder.front());
					ssdholder.pop();
				}

				//Empty out tty queue just incase
				while(!ttyholder.empty()){
					ttyholder.pop();
				}
				
				//Check tty if a process is done
				if(t->check() == true){
					ttyholder = t->user_release();
				}
				//Push processes to core queues
				while(!ttyholder.empty()){
					c->core_request(ttyholder.front());
					ttyholder.pop();
				}
				
				//cout << "INACTIVE PROC COUNT " << inactiveProc.size() << "\n";
				//cout << "ACTIVE PROC COUNT " << activeProc.size() << "\n";
				if(!inactiveProc.empty() || !activeProc.empty()){
					running = true;
				}
				//cout << "TIME " << time << "\n";
				timea++;
			}
			timea--;
		}
		//DONE
		void addProc(vector<ProcID*> a){//contains all process vectors
			for(int i = 0; i < a.size(); i++){
				//cout << a[i].getID() << "\n";
				inactiveProc.push(a[i]);
				//cout << inactiveProc.size() << "\n";
				//cout << inactiveProc.front().getID() << "\n";
				//inactiveProc.pop();
				
				//cout << "TESTING\n";
				//cout << "ADDING PROC ID " <<  a[i].getID() << " TO INACTIVEPROC\n";
			}
			//cout << inactiveProc.front().getID();
		}
		int getProcs(){
			return procDone;
		}
};

//Main call
int main(){
	
	//variables
	int numOfCores = 0;
	int time = 0;
	int numOfProcesses = 0;
	int ssdAccess = 0;
	double avgNumBusy = 0.0;;
	double ssdUtilization = 0.0;


	//cout << "Start Input\n";

	vector<vector<string>> input;
	input = readInput();
	
	numOfCores = stoi(input[1][0]);
	//cout << numOfCores;

	vector<ProcID*> sortedInput;
	sortedInput = scheduleTasks(input);
	//cout<< sortedInput[0].getID();
	//cout << "End Input\n";
	
	int count = 0;

	//#2
	numOfProcesses = sortedInput.size();
	//cout << numOfProcesses;
	//cout << "\n";
	//#3
	ssdAccess = readSSD(input[0]);
	//cout << ssdAccess;
	//cout << "\n";

	a_Core *c = new a_Core();
	SSD *s = new SSD();
	TTY *t = new TTY();
	EventManager* e = new EventManager();

	//c->init(s,t);
	c->setCores(numOfCores);

	for(int i = 0; i < sortedInput.size(); i++){
		sortedInput[i]->getNextProcess();
		sortedInput[i]->getNextProcess();
	}

	e->init(c,s,t);
	e->addProc(sortedInput);

	//cout << "Running Program\n";
	e->runProgram();

	avgNumBusy = c->getcorefree();

	avgNumBusy /= timea;
	if(numOfCores == 1)
		avgNumBusy = 1.0 - avgNumBusy;
	else
		avgNumBusy = avgNumBusy - (numOfCores - 1.0);

	ssdUtilization = s->getssdfree();

	ssdUtilization /= timea;
	ssdUtilization = 1.0 - ssdUtilization;

	cout << "\nSUMMARY:\n";
	cout << "Total elapsed time: " << timea << " ms\n";
	cout << "Number of processes that completed: " << e->getProcs() << "\n";
	cout << "Total number of SSD accesses: " << ssdAccess << "\n";
	cout << "Average number of busy cores: " << setprecision(3) << avgNumBusy << "\n";
	cout << "SSD utilization: " << setprecision(3) << ssdUtilization << "\n";
	cout << "\n";
	
	//cout << "END OF PROGRAM\n";
	return 0;
}

//Reads the total amount of SSD calls in the file
int readSSD(vector<string> a){
	int count = 0;

	for(unsigned int i = 0; i < a.size(); i++){
		if(a[i].compare("SSD") == 0){
			count += 1;
		}
	}
	return count;
}

//Creates ProcID classes, sorts them by start time, and tidy things up
vector<ProcID*> scheduleTasks(vector<vector<string>> a){
	vector<ProcID*> unsorted;
	vector<ProcID*> sorted;
	
	//Set up classes
	for(unsigned int i = 1; i < a[0].size(); i++){
		if(a[0][i].compare("START") == 0){
			ProcID* c = new ProcID();
			unsorted.push_back(c);
			unsorted.back()->setStart(stoi(a[1][i]));
		}
		if(a[0][i].compare("PID") == 0){
			unsorted.back()->setID(stoi(a[1][i]));
		}
		if(a[0][i].compare("END") == 0){
			break;
		}
		unsorted.back()->addProcess(a[0][i]);
		unsorted.back()->addTime(stoi(a[1][i]));
	}
	
	//Sort classes by start times
	for(unsigned int i = 0; i < unsorted.size(); i++){
		for(unsigned int j = 0; j < sorted.size(); j++){
			if(sorted.at(j)->getStartTime() >= unsorted.at(i)->getStartTime()){
				break;
			}
			if(j == sorted.size() - 1){
				sorted.push_back(unsorted.at(i));
				break;
			}
		}
	
		if(i == 0){
			sorted.push_back(unsorted.at(0));
		}
	}
	
	return sorted;
}

//Reads the standard input of a file and arranges a vector
vector<vector<string>> readInput(){
	char c[1024];
	vector<vector<string>> vect(2);
	//scanf("%{^\n]", c);
	//cout << c;
	//cout <<"asdasdas\n";
	bool alt = false;
	while(scanf( "%s", c) == 1){
		//cout << "In Loop";
		//cout << c;
		//cout << "\n";
		if(!alt){//if command
			vect[0].push_back(c);
		} else {//if time
			vect[1].push_back(c);
		}
		alt = !alt;
	}
	
	return vect;
}

using namespace std;
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include "../include/ConnectionHandler.h"

int main(int argc, char *argv[]) 
{
	//thread for keyboard
	//therad for connections
	//there is a code in the assignment to implement
	return 0;
}
class StompClient
{
private:
	std::atomic<bool> shouldTerminate;
    ConnectionHandler connectionHandler;

	//methods for the commands
	void login(const);
public:
	StompClient(/* args */);
	~StompClient();
};

StompClient::StompClient(/* args */)
{
}

StompClient::~StompClient()
{
}

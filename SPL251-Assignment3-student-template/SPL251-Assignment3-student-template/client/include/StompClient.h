using namespace std;
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <fstream>
#include "../include/ConnectionHandler.h"
#include <map>
#include <vector>
#include <string>
#include "../include/event.h" 

class StompClient
{
private:
	bool connected = false;
	int currReceiptId;
	int currSubscriptionId;
	std::mutex consoleMutex;
	//thread keyboardThread;
	//thread socketThread;
	//map<int, string> receiptIdToCommand;			//receipt id and the frame
	std::map<std::string, std::string> loginToPasscode;      // login and passcode
	map<string, vector<Event>> userToReports;		//user and his reports
	map<int, vector<string>> idAndInfo;				//id and login info


public:
	StompClient();
	~StompClient() = default;
	void start();
	std::vector<std::string> getFrame(string command);
	void makeSummary(string channel, string userName, string filePath);
	std::string epochToDate(int epochTime);
	void readFromSocket(ConnectionHandler &connectionHandler);
	void readFromKeyboard(ConnectionHandler &connectionHandler);
	std::map<std::string, std::string> getLoginToPasscode() const;


};
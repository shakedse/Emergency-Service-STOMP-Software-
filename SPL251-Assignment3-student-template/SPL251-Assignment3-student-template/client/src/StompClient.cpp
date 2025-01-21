using namespace std;
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include "../include/ConnectionHandler.h"

int main(int argc, char *argv[])
{
	// thread for keyboard
	// therad for connections
	// there is a code in the assignment to implement
	return 0;
}
class StompClient
{
private:
	std::atomic<bool> shouldTerminate;
	ConnectionHandler connectionHandler;
	bool connected;
	int currReceiptId;
	int currSubscriptionId;
	thread keyboardThread;
	thread socketThread;

	// methods for the commands
	void login(co);

public:
	StompClient(const std::string &host, short port);
	~StompClient();
};

// constructor
stompClient::StompClient(const std::string &host, short port)
{
	connectionHandler(host, port);
	shouldTerminate(false);
	connected(false);
	currReceiptId(0);
	currSubscriptionId(0);
}

void start()
{
	if(!connectionHandler.connect())
	{
		std::cerr << "Cannot connect to server: " << host << ":" << port << std::endl;
		return;
	}
	cout << "Connected to server: " << host << ":" << port << endl;
	keyboardThread = std::thread(this);
	socketThread = std::thread(this);

}
//creating a frame for each user's command
std::vector<std::string> StompClient::getFrame(string command)
{
	vector<String> Frame;
	string currFrame;
	int fisrtSpaceIndex = command.find(" ");
	string commandType = command.substr(0, fisrtSpaceIndex);
	if (commandType == "login")
	{
		if(connected)
		{
			cout << "”The client is already logged in, log out before trying again”" << endl;
			return {};
		}
		int hostIndex = command.find(":");
		// ????????string host = command.substr(fisrtSpaceIndex + 1, hostIndex - fisrtSpaceIndex - 1);
		string host = "stomp.cs.bgu.ac.il";
		cout << "host:" host << endl;

		int loginIndex = command.find(" ", fisrtSpaceIndex + 1);								   // finding the login index
		int passcodeIndex = command.find(" ", loginIndex + 1);									   // finding the passcode index
		string loginName = command.substr(loginIndex + 1, passcodeIndex - loginIndex - 1);		   // getting the login name
		string passcode = command.substr(passcodeIndex + 1, command.length() - passcodeIndex - 1); // getting the passcode
		cout << "login:" loginName << endl;
		cout << "passcode:" passcode << endl;
		connected = true; // after login the user is connected

		currFrame = "CONNECT\n" + "accept-version:1.2\n" + "host:" + host + "\n" + "login:" + loginName + "\n" + "passcode:" + passcode + "\n" + "\n" + "\n\n^@";
		Frame.push_back(currFrame);
	}
	else if (commandType == "join") // subscribing to a channel
	{
		if(!connected)
		{
			cout << "”The client is not logged in, log in before trying to join" << endl;
			return {};
		}
		int channelIndex = command.find(" ", fisrtSpaceIndex + 1);								// finding the channel index
		string channel = command.substr(channelIndex + 1, command.length() - channelIndex - 1); // getting the channel
		cout << "channel sub:" channel << endl;

		currFrame = "SUBSCRIBE\n" + "destination:/" + channel + "\n" + "id:0\n" + "receipt:1\n" + "\n" + "\n\n^@";
		Frame.push_back(currFrame);
	}
	else if (commandType == "exit")
	{
		if(!connected)
		{
			cout << "”The client is not logged in, log in before trying to exit" << endl;
			return {};
		}
		int channelIndex = command.find(" ", fisrtSpaceIndex + 1);								// finding the channel index
		string channel = command.substr(channelIndex + 1, command.length() - channelIndex - 1); // getting the channel
		cout << "channel unsub:" channel << endl;

		currFrame = "UNSUBSCRIBE\n" + "id:0\n" + "receipt:1\n" + "\n" + "\n\n^@";
		Frame.push_back(currFrame);
	}
	else if (commandType == "report")
	{
		if(!connected)
		{
			cout << "”The client is not logged in, log in before trying to report" << endl;
			return {};
		}
		int fileIndex = command.find(" ", fisrtSpaceIndex + 1);						   // finding the file index
		string file = command.substr(fileIndex + 1, command.length() - fileIndex - 1); // getting the file
		names_and_events parsedFile = parseEventsFile(file);						   // parsing the file

		for (Event event : parsedFile.events)
		{
			currFrame = "SEND\n" +
						"destination:/" + event.get_channel_name() + "\n\n" +
						"user:" + event.getEventOwnerUser() + "\n" +
						"city:" + event.get_city() + "\n" +
						"event name:" + event.get_name() + "\n" +
						"date time:" + to_string(event.get_date_time()) + "\n" +
						"general information:" + "\n" +
						"    active:" + event.get_general_information().at("active") + "\n" +
						"    forces arrival at scene:" + event.get_general_information().at("forces arrival at scene") + "\n" +
						"description:" + event.get_description() + "\n" + event.get_description() + "\n\n^@";
		}
		Frame.push_back(currFrame);
	}
	else if (commandType == "summary")
	{
		if(!connected)
		{
			cout << "”The client is not logged in, log in before trying to summary" << endl;
			return {};
		}
	}
	else if (commandType == "logout")
	{
		if(!connected)
		{
			cout << "”The client is not logged in, log in before trying to logout" << endl;
			return {};
		}
		string Frame = "DISCONNECT\n" + "receipt:1\n" + "\n" + "\n\n^@";
	}
	return Frame;
}
void StompClient::makeSummary()
{
}

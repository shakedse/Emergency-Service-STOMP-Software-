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
	Integer currReceiptId;
	Integer currSubscriptionId;
	thread keyboardThread;
	thread socketThread;
	map<Integer, string> receiptIdToCommand;			//receipt id and the frame
	map<string,string> loginToPasscode;					//login and passcode
	map<string, vector<string> userToReports;			//user and his reports
	map<string, vector<string>> idAndInfo;				//id and login info
	
	//first in the vector - login name, second- passcode
	map<string, vector<Event>> topicToEvents;//channel and his events


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
	keyboardThread = std::thread([this]);
<<<<<<< HEAD
	socketThread = std
=======
	socketThread = std::thread(this);
>>>>>>> d7edf11b52f0b7d2b4075cdf82d23cec61ad963c
}

//creating a frame for each user's command
std::vector<std::string> StompClient::getFrame(string command)
{
	vector<String> Frame;
	string currFrame;
	int fisrtSpaceIndex = command.find(" ");
	string commandType = command.substr(0, fisrtSpaceIndex);
	
	//login command
	if (commandType == "login")
	{
		if(connected)
		{
			cout << "The client is already logged in, log out before trying again" << endl;
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

		connected = true; //after login the user is connected

		//checking if the login name is already in the system
		if(loginToPasscode.find(loginName) != loginToPasscode.end())
		{
			cout << "Wrong password" << endl;
			return {};
		}
		idandinfo[currSubscriptionId] = {loginName, passcode};
		loginToPasscode[loginName]= passcode;
		currSubscriptionId++;

		currFrame = "CONNECT\n" + "accept-version:1.2\n" + "host:" + host + "\n" + "login:" + loginName + "\n" + "passcode:" + passcode + "\n" + "\n" + "\n\n^@";
		Frame.push_back(currFrame);
	}

	//join command
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

		currFrame = "SUBSCRIBE\n" + "destination:/" + channel + "\n" + "id:" + currSubscriptionId +"\n" + "receipt:" + currReceiptId +"\n" + "\n\n^@";
		currReceiptId++;
		Frame.push_back(currFrame);
	}

	//exit command
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

	//report command
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

			userToReports[event.getEventOwnerUser()].push_back(currFrame);
			Frame.push_back(currFrame);
		}
	}

	//summary command
	else if (commandType == "summary")
	{
		if(!connected)
		{
			cout << "”The client is not logged in, log in before trying to summary" << endl;
			return {};
		}
		int channelIndex = 9; //summary+" "
		int usernameIndex = command.find(" ", channelIndex + 1); // finding the index after the summary
		int fileIndex = command.find(" ", usernameIndex + 1); // finding the index after the username
		
		string channel = command.substr(channelIndex, usernameIndex - channelIndex - 1); // getting the channel
		string userName = command.substr(usernameIndex + 1, fileIndex - usernameIndex - 1); // getting the user name
		string filePath = command.substr(fileIndex + 1, command.length() - fileIndex - 1); // getting the file path
		cout << "user name:" userName << endl;
		Frame = makeSummary(channel, userName, filePath);
		//מה עושים עם הקובץ של הסאמרי??????
	}

	//logout command
	else if (commandType == "logout")
	{
		if(!connected)
		{
			cout << "”The client is not logged in, log in before trying to logout" << endl;
			return {};
		}
		currFrame =  "DISCONNECT\n" + "receipt:1\n" + "\n" + "\n\n^@";
		Frame.push_back(currFrame);
	}
	return Frame;
}

vector<std::string> StompClient::makeSummary(string channel, string userName, string filePath)
{
	//ig the user has no reports
	if(userToReports.find(userName) == userToReports.end())
	{
		cout << "No reports for this user" << endl;
		return {};
	}
	std::ofstream file(filePath, std::ios::out);
	if(!file.is_open())
	{
		cout << "Could not open the file" << endl;
		return {};
	} 
	vector<std::string> finalSummary = {};
	vector<std::string> relevantReports = {};

	//initiating the statistics
	int reportsNum = 0;
	int activeEventsNum = 0;
	int forceArrivalNum = 0;

	//going over all the reports of the user
	for(const auto &report : userToReports[userName])
	{
		if(report.get_channel_name() == channel)//if the report is about the channel
		{
			relevantReports.push_back(report);
			reportsNum++;
			if(report.get_general_information().at("forces arrival at scene") == "true")
			{
				forceArrivalNum++;
			}
			if(report.get_general_information().at("active") == "true")
			{
				activeEventsNum++;
			}
		}
	}

	//sorting the reports by the date time
	std::sort(relevantReports.begin(), relevantReports.end(), [](const Event &a, const Event &b) 
	{
	if (a.get_date_time() == b.get_date_time()) {
		return a.get_name() < b.get_name();
	}
	return a.get_date_time() < b.get_date_time();
	});

//the beginning of the summary
	file << "Channel " << channel << "\n" ;
    file << "Stats:\n" ;
    file << "Total: " << std::to_string(reportsNum) << "\n" ;
    file << "active: " << std::to_string(activeEventsNum) << "\n" ;
    file << "forces arrival at scene: " << std::to_string(forceArrivalNum) << "\n" +
    file << "Event Reports:\n\n";

	//converting the epoch time to date
	std::string StompClient::epochToDate(int epochTime) 
	{
		time_t rawTime = epochTime;
    	struct tm *timeInfo = localtime(&rawTime);
    	char buffer[20];
    	strftime(buffer, sizeof(buffer), "%d/%m/%y %H:%M", timeInfo);
    	return std::string(buffer);
	}

	//creating the summary for each report
	for(const auto &report : relevantReports)
	{
		int i=1;
		std::string description = report.get_description();
		if(description.length() > 27)
		{
			description = description.substr(0, 27) + "...";
		}

		file <<  "Report_" << std::to_string(i) <<":\n" ;
		file << "    city:" << report.get_city() << "\n" ;
		file << "    date time:" << epochToDate((report.get_date_time()) << "\n" ;
		file << "    event name:" << report.get_name() << "\n" ;
		file << "    summary:" << description << "\n";
		
		i++;
	}
	//writing the summary to the file

	
	file.close();
	return finalSummary;
	cout << "Summary has been written to the file" << filepath << endl;

}

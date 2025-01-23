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

int main(int argc, char *argv[])
{
	if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " host port" << std::endl
                  << std::endl;
        return -1;
    }
    std::string host = argv[1];
    short port = atoi(argv[2]);

    ConnectionHandler connectionHandler(host, port);
    if (!connectionHandler.connect())
    {
        std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
        return 1;
    }

    // initialize threads
	StompClient stompClient(host, port);
	std::thread keyboardThread(&StompClient::readFromKeyboard, &stompClient, std::ref(connectionHandler));
	std::thread socketThread(&StompClient::readFromSocket, &stompClient, std::ref(connectionHandler));

    // wait for threads to finish
    keyboardThread.join();
    socketThread.join();
}

class StompClient
{
private:
	std::atomic<bool> shouldTerminate;
	ConnectionHandler connectionHandler;
	bool connected;
	int currReceiptId;
	int currSubscriptionId;
	std::mutex consoleMutex;
	//thread keyboardThread;
	//thread socketThread;
	map<int, string> receiptIdToCommand;			//receipt id and the frame
	map<string,string> loginToPasscode;				//login and passcode
	map<string, vector<Event>> userToReports;		//user and his reports
	map<int, vector<string>> idAndInfo;				//id and login info
	

public:
	StompClient(const std::string &host, short port);
	~StompClient();
	void start();
	std::vector<std::string> getFrame(string command);
	void makeSummary(string channel, string userName, string filePath);
	std::string epochToDate(int epochTime);
	void readFromSocket(ConnectionHandler &connectionHandler);
	void readFromKeyboard(ConnectionHandler &connectionHandler);
};

// constructor
StompClient::StompClient(const std::string &host, short port):
connectionHandler(host, port),
	shouldTerminate(false),
	connected(false),
	currReceiptId(0),
	currSubscriptionId(0),
	receiptIdToCommand(),
	loginToPasscode(),
	userToReports(),
	idAndInfo() 	
{}


void StompClient::start()
{
	if(!connectionHandler.connect())
	{
		std::cerr << "Cannot connect to server: " << connectionHandler.getHost() << ":" << connectionHandler.getPort() << std::endl;
		return;
	}
	cout << "Connected to server: host - " << connectionHandler.getHost() << "port -" << connectionHandler.getPort() << endl;
	//keyboardThread = std::thread([this]);
	//socketThread = std

	
}
// read from keyboard
void  StompClient::readFromKeyboard(ConnectionHandler &connectionHandler)
{
    // From here we will see the rest of the ehco client implementation:
    while (!shouldTerminate)
    {
        const short bufsize = 1024;                     // maximal size of message
        char buf[bufsize];                              // buffer array for the message
        std::cin.getline(buf, bufsize);                 // read the message from the keyboard
        std::string lineRead(buf);                      // convert the message to string
        int len = lineRead.length();                    // get the length of the message
        std::lock_guard<std::mutex> lock(consoleMutex); // lock the thread

        std::string command(lineRead); 	// get the command from the user
        vector<string> currFrame = getFrame(command); // get the frame for the command
        // if the message was not sent
        while (!currFrame.empty())
        {
            int index = 0;
            if (!connectionHandler.sendLine(currFrame[index]))
            {
                std::cout << "Frame did not sent - fail\n"
                          << std::endl;
                std::cout << "Could not connect to server\n"
                          << std::endl;
                shouldTerminate = true; // terminate the program
                break;
            }
            index++;
        }

        // if the message was sent
        // connectionHandler.sendLine(line) appends '\n' to the message. Therefor we send len+1 bytes.
        std::cout << "Sent " << len + 1 << " bytes to server" << std::endl;
    }
}
void  StompClient::readFromSocket(ConnectionHandler &connectionHandler)
{
    while (!shouldTerminate)
    {
        std::string answer; // the message from the server

        // Get back an answer: by using the expected number of bytes (len bytes + newline delimiter)
        // We could also use: connectionHandler.getline(answer) and then get the answer without the newline char at the end

        std::lock_guard<std::mutex> lock(consoleMutex); // lock the thread
        if (!connectionHandler.getLine(answer))         // get the message from the server
        {                                               // if the message was not received
            std::cout << "Disconnected. Exiting...\n"
                      << std::endl;
            shouldTerminate = true;
            break;
        }

        // if the message was received
        int len = answer.length(); // get the length of the message

        // A C string must end with a 0 char delimiter.  When we filled the answer buffer from the socket
        // we filled up to the \n char - we must make sure now that a 0 char is also present. So we truncate last character.

        answer.resize(len - 1); // remove the '\n' character from the message
        std::cout << "Reply: " << answer << " " << len << " bytes " << std::endl
                  << std::endl;
        if (answer == "bye")
        {
            std::cout << "Exiting...\n"
                      << std::endl;
            break;
        shouldTerminate = true;
        }
    }
}
//creating a frame for each user's command
std::vector<std::string> StompClient::getFrame(string command)
{
	vector<string> Frame;
	string currFrame;
	int firstSpaceIndex = command.find(" ");
	string commandType = command.substr(0, firstSpaceIndex);
	
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
		string hostUser = "stomp.cs.bgu.ac.il";
		cout << "host:" <<hostUser << endl;

		int loginIndex = command.find(" ", firstSpaceIndex + 1);								   // finding the login index
		int passcodeIndex = command.find(" ", loginIndex + 1);									   // finding the passcode index
		string loginName = command.substr(loginIndex + 1, passcodeIndex - loginIndex - 1);		   // getting the login name
		string passcode = command.substr(passcodeIndex + 1, command.length() - passcodeIndex - 1); // getting the passcode
		
		cout << "login:" <<loginName << endl;
		cout << "passcode:" <<passcode << endl;

		connected = true; //after login the user is connected

		//checking if the login name is already in the system
		if(loginToPasscode.find(loginName) != loginToPasscode.end())
		{
			cout << "Wrong password" << endl;
			return {};
		}
		idAndInfo[currSubscriptionId] = {loginName, passcode};
		loginToPasscode[loginName]= passcode;
		currSubscriptionId++;

		currFrame = "CONNECT"+ "\n" + "accept-version:1.2\n" + "host:" + hostUser + "\n" + "login:" + loginName + "\n" + "passcode:" + passcode + "\n" + "\n" + "\n\n\0";
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
		int channelIndex = command.find(" ", firstSpaceIndex + 1);								// finding the channel index
		string channel = command.substr(channelIndex + 1, command.length() - channelIndex - 1); // getting the channel
		cout << "channel sub:" <<channel << endl;

		currFrame = "SUBSCRIBE\n" + "destination:/" + channel + "\n" + "id:" + std::to_string(currSubscriptionId) +"\n" + "receipt:" + std::to_string(currReceiptId) +"\n" + "\n\n\0";
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
		int channelIndex = command.find(" ", firstSpaceIndex + 1);								// finding the channel index
		string channel = command.substr(channelIndex + 1, command.length() - channelIndex - 1); // getting the channel
		cout << "channel unsub:" << channel << endl;

		currFrame = "UNSUBSCRIBE\n" + "id:" + std::to_string(currSubscriptionId) + "\n" + "receipt:" + std::to_string(currReceiptId) + "\n" + "\n\n\0";
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
		int fileIndex = command.find(" ", firstSpaceIndex + 1);						   // finding the file index
		string file = command.substr(fileIndex + 1, command.length() - fileIndex - 1); // getting the file
		names_and_events parsedFile = parseEventsFile(file);						   // parsing the file

		for (Event event : parsedFile.events)
		{
			currFrame = "SEND\n" +
						"destination:/" + std::string(event.get_channel_name()) + "\n\n" +
						"user:" + event.getEventOwnerUser() + "\n" +
						"city:" + event.get_city() + "\n" +
						"event name:" + event.get_name() + "\n" +
						"date time:" + to_string(event.get_date_time()) + "\n" +
						"general information:" + "\n" +
						"    active:" + event.get_general_information().at("active") + "\n" +
						"    forces arrival at scene:" + event.get_general_information().at("forces arrival at scene") + "\n" +
						"description:" + event.get_description() + "\n" + event.get_description() + "\n\n"+"\0";

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
		cout << "user name:" <<userName << endl;
		makeSummary(channel, userName, filePath);
	}

	//logout command
	else if (commandType == "logout")
	{
		if(!connected)
		{
			cout << "”The client is not logged in, log in before trying to logout" << endl;
			return {};
		}
		currFrame =  "DISCONNECT\n" + "receipt:1\n" + "\n" + "\n\n\0";
		Frame.push_back(currFrame);
	}
	return Frame;
}

void StompClient::makeSummary(string channel, string userName, string filePath)
{
	//ig the user has no reports
	if(userToReports.find(userName) == userToReports.end())
	{
		cout << "No reports for this user" << endl;
		return ;
	}
	std::ofstream file(filePath, std::ios::out);
	if(!file.is_open())
	{
		cout << "Could not open the file" << endl;
		return ;
	} 
	vector<Event> finalSummary = {};
	vector<Event> relevantReports = {};

	//initiating the statistics
	int reportsNum = 0;
	int activeEventsNum = 0;
	int forceArrivalNum = 0;

	//going over all the reports of the user
	for( Event &report : userToReports[userName])
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
    file << "forces arrival at scene: " << std::to_string(forceArrivalNum) << "\n" ;
    file << "Event Reports:\n\n";


	//creating the summary for each report
	for(const Event &report : relevantReports)
	{
		int i=1;
		std::string description = report.get_description();
		if(description.length() > 27)
		{
			description = description.substr(0, 27) + "...";
		}

		file <<  "Report_" << std::to_string(i) <<":\n" ;
		file << "    city:" << report.get_city() << "\n" ;
		file << "    date time:" << epochToDate(report.get_date_time()) << "\n" ;
		file << "    event name:" << report.get_name() << "\n" ;
		file << "    summary:" << description << "\n";
		
		i++;
	}
	//writing the summary to the file

	
	file.close();
	cout << "Summary has been written to the file" << filePath << endl;

}
//converting the epoch time to date
	std::string StompClient::epochToDate(int epochTime) 
	{
		time_t rawTime = epochTime;
    	struct tm *timeInfo = localtime(&rawTime);
    	char buffer[20];
    	strftime(buffer, sizeof(buffer), "%d/%m/%y %H:%M", timeInfo);
    	return std::string(buffer);
	}

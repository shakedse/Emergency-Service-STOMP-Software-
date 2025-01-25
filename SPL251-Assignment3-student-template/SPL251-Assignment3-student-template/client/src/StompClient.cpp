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

bool connected = false;
int currReceiptId;
int currSubscriptionId;
std::mutex consoleMutex;
// thread keyboardThread;
// thread socketThread;
map<string, int> receiptIdToCommand;			//receipt id and the frame
map<string, string> loginToPasscode;	  // login and passcode
map<string, vector<Event>> userToReports; // user and his reports
map<int, vector<string>> idAndInfo;
std::string loginUser;
/*
// constructor
StompClient::StompClient():
	shouldTerminate(false),
	connected(false),
	currReceiptId(0),
	currSubscriptionId(0),
	receiptIdToCommand(),
	loginToPasscode(),
	userToReports(),
	idAndInfo()
{}
*/
/*
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
*/

// converting the epoch time to date
std::string epochToDate(int epochTime)
{
	time_t rawTime = epochTime;
	struct tm *timeInfo = localtime(&rawTime);
	char buffer[20];
	strftime(buffer, sizeof(buffer), "%d/%m/%y %H:%M", timeInfo);
	return std::string(buffer);
}

std::atomic<bool> shouldTerminate(false);
void makeSummary(string channel, string userName, string filePath)
{
	// ig the user has no reports
	if (userToReports.find(userName) == userToReports.end())
	{
		std::cout << "No reports for this user" << endl;
	}
	std::ofstream file(filePath, std::ios::out);
	if (!file.is_open())
	{
		std::cout << "Could not open the file" << endl;
	}
	vector<Event> finalSummary = {};
	vector<Event> relevantReports = {};
	//vector<Event> vr = userToReports.find(userName)->second;


	// initiating the statistics
	int reportsNum = 0;
	int activeEventsNum = 0;
	int forceArrivalNum = 0;

	std::cout << "2" << endl;
	// going over all the reports of the user
	for (Event &report : userToReports.find(userName)->second)
	{
		if (report.get_channel_name() == channel) // if the report is about the channel
		{
			relevantReports.push_back(report);
			reportsNum++;
			if (report.get_general_information().at("forces_arrival_at_scene") == "true")
			{
				forceArrivalNum++;
			}
			if (report.get_general_information().at("active") == "true")
			{
				activeEventsNum++;
			}
		}
	}

	// sorting the reports by the date time
	/*std::sort(relevantReports.begin(), relevantReports.end(), [](const Event &a, const Event &b)
			  {
		if (a.get_date_time() == b.get_date_time()) {
		return a.get_name() < b.get_name();
	}

	return a.get_date_time() < b.get_date_time(); });
*/
	// the beginning of the summary
	file << "Channel " << channel << "\n";
	file << "Stats:\n";
	file << "Total: " << std::to_string(reportsNum) << "\n";
	file << "active: " << std::to_string(activeEventsNum) << "\n";
	file << "forces arrival at scene: " << std::to_string(forceArrivalNum) << "\n";
	file << "Event Reports:\n\n";

	// creating the summary for each report
	for (const Event &report : relevantReports)
	{
		int i = 1;
		std::string description = report.get_description();
		if (description.length() > 27)
		{
			description = description.substr(0, 27) + "...";
		}

		file << "Report_" << std::to_string(i) << ":\n";
		file << "    city:" << report.get_city() << "\n";
		file << "    date time:" << epochToDate(report.get_date_time()) << "\n";
		file << "    event name:" << report.get_name() << "\n";
		file << "    summary:" << description << "\n";

		i++;
	}
	// writing the summary to the file

	file.close();
	cout << "Summary has been written to the file" << filePath << endl;
}

// creating a frame for each user's command
std::vector<std::string> getFrame(string command, ConnectionHandler &connectionHandler)
{
	vector<string> Frame;
	int firstSpaceIndex = command.find(' ');
	if (firstSpaceIndex > 7)
	{
		cout << "please enter a command" << endl;
		return {};
	}
	string commandType = command.substr(0, firstSpaceIndex);
	
	// login command
	if (commandType == "login")
	{
		if (connected)
		{
			cout << "The client is already logged in" << endl;
		}
		else
		{
			std::string arg;
			std::stringstream ss(command);
			std::vector<std::string> args;
			short port = 0;

			// Split the string by space
			while (ss >> arg)
			{
				args.push_back(arg); // Add each arg to the vector
			}

			if (args.size() != 4)
			{
				std::cout << "login command needs 3 args: {host:port} {username} {password}" << std::endl;
				return Frame;
			}

			string hostUser = "stomp.cs.bgu.ac.il";

			if (args[1].find(':') == string::npos)
			{
				std::cout << "Invalid host:port" << std::endl;
				return Frame;
			}

			string host = args[1].substr(0, args[1].find(':'));
			std::cout << host;

			std::string portString = args[1].substr(args[1].find(':') + 1, args[1].size() - 1);
			std::cout << portString;
			
			try
			{
				int tempPort = std::stoi(portString);	   // Convert string to int
				port = static_cast<short>(tempPort); // Cast to short
			}
			catch (const std::invalid_argument &e)
			{
				std::cerr << "Invalid argument: " << e.what() << std::endl;
				return Frame;
			}
			catch (const std::out_of_range &e)
			{
				std::cerr << "Out of range: " << e.what() << std::endl;
				return Frame;
			}

			std::string username = args[2];
			loginUser = username;
			std::string password = args[3];

			// checking if the login name is already in the system
			if (loginToPasscode[username] != "" && loginToPasscode[username] != password)
			{
				std::cout << "Wrong password" << std::endl;
				return Frame;
			}
			else
			{
				connectionHandler.setHost(host);
				connectionHandler.setPort(port);
				if (!connectionHandler.connect())
				{
					std::cerr << "Cannot connect to " << host << ":" << std::endl;
					return Frame;
				}
				else
				{
					connected = true; // after login the user is connected
					idAndInfo[currSubscriptionId] = {username, password};
					loginToPasscode[username] = password;
					Frame.push_back("CONNECT\naccept-version:1.2\nhost:" + host + "\n" + "login:" + username + "\n" + "passcode:" + password + "\n" + "\0");
				}
			}
		}
	}
	// join command
	else if (commandType == "join") // subscribing to a channel
	{
		std::vector<std::string> args;
		if (!connected)
		{
			cout << "The client is not logged in, log in before trying to join" << endl;
		}
		else
		{
			std::string arg;
			std::stringstream ss(command);

			// Split the string by space
			while (ss >> arg)
			{
				args.push_back(arg); // Add each arg to the vector
			}
			if (args.size() != 2)
			{
				std::cout << "join command needs 1 args: {channel}" << std::endl;
			}
			else
			{
				// getting the channel
				string channel = args[1];
				cout << "channel sub:" << channel << endl;
				std::cout << "SUBSCRIBE\n destination:" + channel + "\nid:" + std::to_string(currSubscriptionId) + "\nreceipt:" + std::to_string(currReceiptId) + "\n\0" << std::endl;
				Frame.push_back("SUBSCRIBE\n destination:" + channel + "\nid:" + std::to_string(currSubscriptionId) + "\nreceipt:" + std::to_string(currReceiptId) + "\n\0");
				receiptIdToCommand[channel] = currSubscriptionId;
				currReceiptId++;
				currSubscriptionId++;
			}
		}
	}
	// exit command
	else if (commandType == "exit")
	{
		std::vector<std::string> args;
		if (!connected)
		{
			cout << "The client is not logged in, log in before trying to exit" << endl;
		}
		else
		{
			std::string arg;
			std::stringstream ss(command);
			// Split the string by space
			while (ss >> arg)
			{
				args.push_back(arg); // Add each arg to the vector
			}
			if (args.size() != 2)
			{
				std::cout << "exit command needs 1 args: {channel}" << std::endl;
			}
			else
			{
				// getting the channel
				string channel = args[1]; // getting the channel
				std::cout << "channel unsub:" << channel << std::endl;
				Frame.push_back("UNSUBSCRIBE\nid:" + std::to_string(receiptIdToCommand[channel]) + "\nreceipt:" + std::to_string(currReceiptId) + "\n\n" + "\0");
			}
		}
	}
	// report command
	else if (commandType == "report")
	{
		std::vector<std::string> args;
		if (!connected)
		{
			cout << "The client is not logged in, log in before trying to report" << endl;
		}
		else
		{
			std::string arg;
			std::stringstream ss(command);
			// Split the string by space
			while (ss >> arg)
			{
				args.push_back(arg); // Add each arg to the vector
			}
			if (args.size() != 2)
			{
				std::cout << "report command needs 1 args: {channel}" << std::endl;
			}
			else
			{
				string file = args[1];								 // getting the file
				names_and_events parsedFile = parseEventsFile(file); // parsing the file
				for (const Event &event : parsedFile.events)
				{
					std::ostringstream oss;
					oss << "SEND\n"
						<< "destination:/" << event.get_channel_name() << "\n\n"
						<< "user:" << loginUser << "\n"
						<< "city:" << event.get_city() << "\n"
						<< "event name:" << event.get_name() << "\n"
						<< "date time:" << std::to_string(event.get_date_time()) << "\n"
						<< "general information:\n"
						<< "    active:" << event.get_general_information().at("active") << "\n"
						<< "    forces arrival at scene:" << event.get_general_information().at("forces_arrival_at_scene") << "\n"
						<< "description:" << event.get_description() << "\n";
					std::string currFrame = oss.str();
					std::cout << currFrame << std::endl;
					userToReports[loginUser].push_back(event);
					Frame.push_back(currFrame);
				}
			}
		}
	}
	// summary command
	else if (commandType == "summary")
	{
		std::vector<std::string> args;
		if (!connected)
		{
			cout << "The client is not logged in, log in before trying to summary" << endl;
		}
		else
		{
			std::string arg;
			std::stringstream ss(command);
			// Split the string by space
			while (ss >> arg)
			{
				args.push_back(arg); // Add each arg to the vector
			}
			std::cout << "size:" + args.size() << std::endl;
			if (args.size() != 4)
			{
				std::cout << "summary command needs 4 args: {channel} {username} {filepath}" << std::endl;
			}
			else
			{
			}

			string channel = args[1];  // getting the channel
			string userName = args[2]; // getting the user name
			string filePath = args[3]; // getting the file path
			std::cout << "1" << std::endl;
			makeSummary(channel, userName, filePath);
		}
		// logout command
	}
	else if (commandType == "logout")
	{
		if (!connected)
		{
			cout << "The client is not logged in, log in before trying to logout" << endl;
		}
		std::string sum = "DISCONNECT\nreceipt:" + std::to_string(currReceiptId) + "\n\n";
		Frame.push_back(sum);
		connected = false;
		loginUser = "";
	}
	else
	{
		cout << "please enter an existing commend" << endl;
	}
	return Frame;
}

void readFromKeyboard(ConnectionHandler &connectionHandler)
{
	// From here we will see the rest of the ehco client implementation:
	while (!shouldTerminate)
	{

		const short bufsize = 1024;								// maximal size of message
		char buf[bufsize];										// buffer array for the message
		std::cin.getline(buf, bufsize);							// read the message from the keyboard
		std::string lineRead(buf);								// convert the message to string
		int len = lineRead.length();							// get the length of the message
		std::vector<std::string> commands = getFrame(lineRead, connectionHandler); // get the command from the user

		// if the message was not sent
		int index = 0;
		index++;
		for (std::string command : commands)
		{
			std::lock_guard<std::mutex> lock(consoleMutex); // lock the thread
			if (!connectionHandler.sendLine(command))
			{
				std::cerr << "Frame did not sent - fail\n"
						  << std::endl;
				std::cerr << "Could not connect to server\n"
						  << std::endl;
				shouldTerminate = true; // terminate the program
				break;
			}
		}
		index++;

		// if the message was sent
		// connectionHandler.sendLine(line) appends '\n' to the message. Therefor we send len+1 bytes.
		std::cout << "Sent " << len + 1 << " bytes to server" << std::endl;
	}
}

void readFromSocket(ConnectionHandler &connectionHandler)
{
	while (!shouldTerminate && connected)
	{
		std::string answer; // the message from the server

		// Get back an answer: by using the expected number of bytes (len bytes + newline delimiter)
		// We could also use: connectionHandler.getline(answer) and then get the answer without the newline char at the end

		// std::lock_guard<std::mutex> lock(consoleMutex); // lock the thread
		if (!connectionHandler.getLine(answer)) // get the message from the server
		{										// if the message was not received
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
		std::cout << "Reply: " << answer << " " << len << " bytes " << std::endl;

		std::string command = answer.substr(0, answer.find("\n"));
		std::cout << command << std::endl;

		if(command == "ERROR") // disconnecting in case of error
			connected = false;

		if (answer == "bye")
		{
			std::cout << "Exiting...\n"
					  << std::endl;
			shouldTerminate = true;

			break;
		}
	}
}

int main(int argc, char *argv[])
{
	/*if (argc < 3)
	{
		std::cerr << "Usage: " << argv[0] << " host port" << std::endl
				  << std::endl;
		return -1;
	}
	std::cout << "step 1" << std::endl;
	std::string host = argv[1];
	short port = atoi(argv[2]);*/

	std::cerr << " host port" << std::endl;
	
	ConnectionHandler connectionHandler("0", 0);

	// initialize threads
	std::thread keyboardThread(readFromKeyboard, std::ref(connectionHandler));
	std::thread socketThread(readFromSocket, std::ref(connectionHandler));

	// wait for threads to finish
	keyboardThread.join();
	socketThread.join();
	return 0;
}
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
#include "../include/StompProtocol.h"

std::mutex consoleMutex;
StompProtocol *stompProtocol = new StompProtocol();

void readFromKeyboard(ConnectionHandler &connectionHandler)
{
	while (stompProtocol->getConnected())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		const short bufsize = 1024; // maximal size of message
		char buf[bufsize];			// buffer array for the message
		if (stompProtocol->getConnected())
			std::cin.getline(buf, bufsize); // read the message from the keyboard
		else
			break;

		std::string lineRead(buf);																  // convert the message to string
		int len = lineRead.length();															  // get the length of the message
		std::vector<std::string> commands = stompProtocol->getFrame(lineRead, connectionHandler); // get the command from the user

		// if the message was not sent
		//std::lock_guard<std::mutex> lock(consoleMutex); // lock the thread

		for (std::string command : commands)
		{
			if (!connectionHandler.sendLine(command))
			{
				std::cerr << "Frame did not sent - fail\n"
						  << std::endl;
				std::cerr << "Could not connect to server\n"
						  << std::endl;
				stompProtocol->setShouldTerminate(true); // terminate the program
				break;
			}
					cout << command << endl;

		}
	}
}

void readFromSocket(ConnectionHandler &connectionHandler)
{
	while (!stompProtocol->getShouldTerminate() && stompProtocol->getConnected())
	{
		std::string answer; // the message from the server

		// Get back an answer: by using the expected number of bytes (len bytes + newline delimiter)
		// We could also use: connectionHandler.getline(answer) and then get the answer without the newline char at the end

		//std::lock_guard<std::mutex> lock(consoleMutex); // lock the thread
		if (!connectionHandler.getLine(answer))			// get the message from the server
		{												// if the message was not received
			std::cout << "Disconnected. Exiting...\n"
					  << std::endl;
			stompProtocol->setShouldTerminate(true);
			break;
		}

		// if the message was received
		int len = answer.length(); // get the length of the message

		// A C string must end with a 0 char delimiter.  When we filled the answer buffer from the socket
		// we filled up to the \n char - we must make sure now that a 0 char is also present. So we truncate last character.

		answer.resize(len - 1); // remove the '\n' character from the message

		std::string command = answer.substr(0, answer.find("\n"));

		if (stompProtocol->getWaitingToDisconnect() && command == "RECEIPT")
		{
			const std::string key = "receipt-id:";
			size_t startPos = answer.find(key) + 11;
			size_t endPos = answer.find_first_not_of("0123456789", startPos);
			int receiptId = std::stoi(answer.substr(startPos, endPos));
			if (receiptId == stompProtocol->getCurrReceiptId())
			{
				stompProtocol->setConnected(false);
				stompProtocol->setWaitingToDisconnect(false);
				connectionHandler.close();
			}
		}

		if (command == "ERROR") // disconnecting in case of error
		{
			stompProtocol->setConnected(false);
			connectionHandler.close();
		}
	}
}

int main(int argc, char *argv[])
{
	string host;
	short port;
	std::string username;
	std::string password;
	ConnectionHandler connectionHandler(host, port);
	while (true)
	{
		while (!stompProtocol->getConnected())
		{
			std::vector<std::string> args = stompProtocol->logIn();

			std::string hostPort = args[1];
			host = hostPort.substr(0, hostPort.find(':'));
			std::string portString = hostPort.substr(hostPort.find(':') + 1, hostPort.size() - 1);

			try
			{
				int tempPort = std::stoi(portString); // Convert string to int
				port = static_cast<short>(tempPort);  // Cast to short
			}
			catch (const std::invalid_argument &e)
			{
				std::cerr << "Invalid argument: " << e.what() << std::endl;
				break;
			}
			catch (const std::out_of_range &e)
			{
				std::cerr << "Out of range: " << e.what() << std::endl;
				break;
			}

			username = args[2];
			stompProtocol->setLoginUser(username);
			password = args[3];
			std::map<std::string, std::string> loginToPasscode = stompProtocol->getLoginToPasscode();
			// checking if the login name is already in the system
			if (loginToPasscode[username] != "" && loginToPasscode[username] != password)
			{
				std::cout << "Wrong password" << std::endl;
				break;
			}
			connectionHandler.setHost(host);
			connectionHandler.setPort(port);
			if (!connectionHandler.connect())
			{
				std::cerr << "Cannot connect to " << host << ":" << std::endl;
				connectionHandler.close();
				continue;
			}
			else
			{
				std::map<int, std::vector<std::string>> idAndInfo = stompProtocol->getIdAndInfo();
				stompProtocol->setConnected(true); // after login the user is connected
				idAndInfo[stompProtocol->getCurrSubscriptionId()] = {username, password};
				loginToPasscode[username] = password;
				std::lock_guard<std::mutex> lock(consoleMutex); // lock the thread
				string frame = "CONNECT\naccept-version:1.2\nhost:" + host + "\n" + "login:" + username + "\n" + "passcode:" + password + "\n" + "\0";
				if (!connectionHandler.sendLine(frame))
				{
					std::cerr << "Frame did not sent - fail\n"
							  << std::endl;
					std::cerr << "Could not connect to server\n"
							  << std::endl;
					stompProtocol->setShouldTerminate(true); // terminate the program
					break;
				}

				string answer;
				if (!connectionHandler.getLine(answer)) // get the message from the server
				{										// if the message was not received
					std::cout << "Disconnected. Exiting...\n"
							  << std::endl;
					stompProtocol->setShouldTerminate(true);
					break;
				}

				int len = answer.length(); // get the length of the message

				// A C string must end with a 0 char delimiter.  When we filled the answer buffer from the socket
				// we filled up to the \n char - we must make sure now that a 0 char is also present. So we truncate last character.

				answer.resize(len - 1); // remove the '\n' character from the message

				std::string command = answer.substr(0, answer.find("\n"));
				std::cout << "this is the command in login: " + command << std::endl;

				if (command == "CONNECTED")
				{
					stompProtocol->setConnected(true);
				}
				else
				{
					stompProtocol->setConnected(false);
					connectionHandler.close();
				}
			}
		}

		cout << "we are out of login" << endl;

		// initialize threads
		std::thread keyboardThread(readFromKeyboard, std::ref(connectionHandler));
		std::thread socketThread(readFromSocket, std::ref(connectionHandler));
		// wait for threads to finish
		keyboardThread.join();
		socketThread.join();
		delete stompProtocol;
	}
	return 0;
}
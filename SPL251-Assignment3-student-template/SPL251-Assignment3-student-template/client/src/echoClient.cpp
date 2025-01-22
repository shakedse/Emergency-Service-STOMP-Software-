#include <stdlib.h>
#include "../include/ConnectionHandler.h"
#include <thread>
#include <mutex>

using namespace std;

std::mutex consoleMutex;
std::atomic<bool> shouldTerminate(false);
std::string loggedUserName = "";

/**
 * This code assumes that the server replies the exact text the client sent it (as opposed to the practical session example)
 */

// read from keyboard
void readFromKeyboard(ConnectionHandler &connectionHandler)
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

        std::string command(lineRead); // get the command from the user
        
        // if the message was not sent
            int index = 0;
            if (!connectionHandler.sendLine(command))
            {
                std::cout << "Frame did not sent - fail\n"
                          << std::endl;
                std::cout << "Could not connect to server\n"
                          << std::endl;
                shouldTerminate = true; // terminate the program
                break;
            }
            index++;

        // if the message was sent
        // connectionHandler.sendLine(line) appends '\n' to the message. Therefor we send len+1 bytes.
        std::cout << "Sent " << len + 1 << " bytes to server" << std::endl;
    }
}

// thread for reading from the socket

// We can use one of three options to read data from the server:
// 1. Read a fixed number of characters
// 2. Read a line (up to the newline character using the getline() buffered reader
// 3. Read up to the null

void readFromSocket(ConnectionHandler &connectionHandler)
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
            //??????shouldTerminate = true;
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
            //???????shouldTerminate = true;
        }
    }
}

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
    std::thread keyboardThread(readFromKeyboard, std::ref(connectionHandler));
    std::thread socketThread(readFromSocket, std::ref(connectionHandler));

    // wait for threads to finish
    keyboardThread.join();
    socketThread.join();
    return 0;
}
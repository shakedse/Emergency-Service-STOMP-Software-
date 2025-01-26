#include "../include/StompProtocol.h"
#include <fstream>
#include <vector>

StompProtocol::StompProtocol()
{
    currSubscriptionId = 1;
    connected = false;
    waitingToDisconnect = false;
    logedIn = false;
    currReceiptId = 1;
    loginUser = "";
    shouldTerminate = false;
    receiptIdToCommand = {}; // receipt id and the frame
    loginToPasscode = {};    // login and passcode
    userToReports = {};      // user and his reports
    idAndInfo = {};
    channels = {};
    std::atomic<bool> shouldTerminate(false);
}

// converting the epoch time to date
std::string StompProtocol::epochToDate(int epochTime)
{
    time_t rawTime = epochTime;
    struct tm *timeInfo = localtime(&rawTime);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%d/%m/%y %H:%M", timeInfo);
    return std::string(buffer);
}

void StompProtocol::makeSummary(std::string channel, std::string userName, std::string filePath)
{
    // ig the user has no reports
    if (userToReports.find(userName) == userToReports.end())
    {
        std::cout << "No reports for this user" << std::endl;
    }
    else
    {
        std::ofstream file(filePath, std::ios::out);
        std::vector<Event> finalSummary = {};
        std::vector<Event> relevantReports = {};

        // initiating the statistics
        int reportsNum = 0;
        int activeEventsNum = 0;
        int forceArrivalNum = 0;
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
        // sort the reports by time and name
        std::sort(relevantReports.begin(), relevantReports.end(), [](const Event &e1, const Event &e2)
                  {
                      if (e1.get_date_time() == e2.get_date_time())
                      {
                          return e1.get_channel_name() < e2.get_channel_name(); // Secondary sort by name
                      }
                      return e1.get_date_time() < e2.get_date_time(); // Primary sort by time
                  });

        // the beginning of the summary
        file << "Channel " << channel << "\n";
        file << "Stats:\n";
        file << "Total: " << std::to_string(reportsNum) << "\n";
        file << "active: " << std::to_string(activeEventsNum) << "\n";
        file << "forces arrival at scene: " << std::to_string(forceArrivalNum) << "\n";
        file << "Event Reports:\n\n";
        int i = 0;
        // creating the summary for each report
        for (const Event &report : relevantReports)
        {
            i++;
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
        }
        // writing the summary to the file

        file.close();
        std::cout << "Summary has been written to the file" << filePath << std::endl;
    }
}

// creating a frame for each user's command
std::vector<std::string> StompProtocol::getFrame(std::string command, ConnectionHandler &connectionHandler)
{
    std::vector<std::string> Frame;
    int firstSpaceIndex = command.find(' ');
    if (firstSpaceIndex > 7)
    {
        std::cout << "please enter a command" << std::endl;
        return {};
    }
    std::string commandType = command.substr(0, firstSpaceIndex);

    // login command
    if (commandType == "login")
    {
        std::cout << "The client is already logged in" << std::endl;
    }
    // join command
    else if (commandType == "join") // subscribing to a channel
    {
        std::vector<std::string> args;
        if (!connected)
        {
            std::cout << "The client is not logged in, log in before trying to join" << std::endl;
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
                bool isSubscribed = false;
                for (std::string ch : channels)
                {
                    if (ch == args[1])
                    {
                        std::cout << "You are already subscribed to this channel" << std::endl;
                        isSubscribed = true;
                    }
                }
                if (!isSubscribed)
                {
                    std::string channel = args[1]; // getting the channel
                    Frame.push_back("SUBSCRIBE\ndestination:" + channel + "\nid:" + std::to_string(currSubscriptionId) + "\nreceipt:" + std::to_string(currReceiptId) + "\n");
                    receiptIdToCommand[channel] = currSubscriptionId;
                    currReceiptId++;
                    currSubscriptionId++;
                    channels.push_back(channel);
                }
            }
        }
    }
    // exit command
    else if (commandType == "exit")
    {
        std::vector<std::string> args;
        if (!connected)
        {
            std::cout << "The client is not logged in, log in before trying to exit" << std::endl;
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
                std::string channel = args[1]; // getting the channel
                bool isSubscribed = false;
                for (std::string ch : channels)
                {
                    if (ch == args[1])
                    {
                        isSubscribed = true;
                        break;
                    }
                }
                if(!isSubscribed)
                {
                    std::cout << "can't udsubscribe from a topic you are not subscribed to" << std::endl;
                }
                else
                {
                    Frame.push_back("UNSUBSCRIBE\nid:" + std::to_string(receiptIdToCommand[channel]) + "\nreceipt:" + std::to_string(currReceiptId) + "\n\n");
                    currReceiptId++;
                    channels.erase(std::remove(channels.begin(), channels.end(), channel), channels.end());
                }
                
            }
        }
    }
    //  report command
    else if (commandType == "report")
    {
        std::vector<std::string> args;
        if (!connected)
        {
            std::cout << "The client is not logged in, log in before trying to report" << std::endl;
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
                std::string file = args[1];                          // getting the file
                names_and_events parsedFile = parseEventsFile(file); // parsing the file
                bool isSubscribed = false;
                for (std::string channel : channels)
                {
                    if (channel == parsedFile.channel_name)
                    {
                        isSubscribed = true;
                        break;
                    }
                }
                if (!isSubscribed)
                {
                    std::cout << "You are not subscribed to this channel" << std::endl;
                }
                else
                {
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
                        userToReports[loginUser].push_back(event);
                        Frame.push_back(currFrame);
                    }
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
            std::cout << "The client is not logged in, log in before trying to summary" << std::endl;
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
            if (args.size() != 4)
            {
                std::cout << "summary command needs 4 args: {channel} {username} {filepath}" << std::endl;
            }
            else
            {
                std::string channel = args[1];  // getting the channel
                std::string userName = args[2]; // getting the user name
                std::string filePath = args[3]; // getting the file path
                makeSummary(channel, userName, filePath);
            }
        }
        // logout command
    }
    else if (commandType == "logout")
    {
        if (!connected)
        {
            std::cout << "The client is not logged in, log in before trying to logout" << std::endl;
        }
        std::string sum = "DISCONNECT\nreceipt:" + std::to_string(currReceiptId) + "\n\n\0";
        Frame.push_back(sum);
        loginUser = "";
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        waitingToDisconnect = true;
        connected = false;
    }
    else
    {
        std::cout << "please enter an existing commend" << std::endl;
    }
    return Frame;
}

const std::vector<std::string> StompProtocol::logIn()
{
    while (!connected)
    {
        std::cout << "please log in:" << std::endl;
        const short bufsize = 1024;     // maximal size of message
        char buf[bufsize];              // buffer array for the message
        std::cin.getline(buf, bufsize); // read the message from the keyboard
        std::string lineRead(buf);      // convert the message to string
        int firstSpaceIndex = lineRead.find(' ');
        if (firstSpaceIndex > 7)
        {
            std::cout << "please enter a command" << std::endl;
            continue;
        }
        std::string commandType = lineRead.substr(0, firstSpaceIndex);

        // login command
        if (commandType == "login")
        {
            if (connected)
            {
                std::cout << "The client is already logged in" << std::endl;
                return {};
            }
            else
            {
                std::string arg;
                std::stringstream ss(lineRead);
                std::vector<std::string> args;

                // Split the string by space
                while (ss >> arg)
                {
                    args.push_back(arg); // Add each arg to the vector
                }

                if (args.size() != 4)
                {
                    std::cout << "login command needs 3 args: {host:port} {username} {password}" << std::endl;
                    continue;
                }

                std::string hostUser = "stomp.cs.bgu.ac.il";

                if (args[1].find(':') == std::string::npos)
                {
                    std::cout << "Invalid host:port" << std::endl;
                    continue;
                }
                return args;
            }
        }
        else
        {
            std::cout << "please login first" << std::endl;
        }
    }
    return {};
}
bool StompProtocol::getConnected() const
{
    return connected;
}
bool StompProtocol::getWaitingToDisconnect() const
{
    return StompProtocol::waitingToDisconnect;
}
int StompProtocol::getCurrReceiptId() const
{
    return currReceiptId;
}
int StompProtocol::getCurrSubscriptionId() const
{
    return currSubscriptionId;
}
std::map<std::string, std::string> StompProtocol::getLoginToPasscode() const
{
    return loginToPasscode;
}
std::map<int, std::vector<std::string>> StompProtocol::getIdAndInfo() const
{
    return idAndInfo;
}
const std::atomic<bool>& StompProtocol::getShouldTerminate() const
{
    return shouldTerminate;
}
void StompProtocol::setConnected(bool connected)
{
    StompProtocol::connected = connected;
}   
void StompProtocol::setShouldTerminate(bool change)
{
    shouldTerminate.store(change);
}
void StompProtocol::setWaitingToDisconnect(bool connected)
{
    StompProtocol::waitingToDisconnect = connected;
} 
void StompProtocol::setLoginUser(std::string name)
{
    loginUser = name;
}
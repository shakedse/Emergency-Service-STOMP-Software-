#pragma once

#include "../include/ConnectionHandler.h"
#include <string>
#include <vector>
#include <map>
#include "event.h" // Include the header file where Event is defined

// TODO: implement the STOMP protocol
class StompProtocol
{
private:
    int currSubscriptionId;
    bool connected;
    bool waitingToDisconnect;
    bool logedIn;
    int currReceiptId;
    std::map<std::string, int> receiptIdToCommand;           // receipt id and the frame
    std::map<std::string, std::vector<Event>> userToReports; // user and his reports
    std::map<int, std::vector<std::string>> idAndInfo;
    std::string loginUser;
    std::vector<std::string> channels;
    std::atomic<bool> shouldTerminate;

public:
    StompProtocol();
    std::string epochToDate(int epochTime);
    void makeSummary(std::string channel, std::string userName, std::string filePath);
    std::vector<std::string> getFrame(std::string command, ConnectionHandler &connectionHandler);
    const std::vector<std::string> logIn();
    bool getConnected() const;
    void setConnected(bool connected);
    bool getWaitingToDisconnect() const;
    void setWaitingToDisconnect(bool connected);
    void setLoginUser(std::string name);
    int getCurrReceiptId() const;
    int getCurrSubscriptionId() const;
    std::map<int, std::vector<std::string>> getIdAndInfo() const;
    const std::atomic<bool> &getShouldTerminate() const;
    void setShouldTerminate(bool change);
};

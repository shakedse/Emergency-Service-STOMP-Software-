package bgu.spl.net.impl.stomp;

import java.util.concurrent.ConcurrentHashMap;

import bgu.spl.net.api.*;
import bgu.spl.net.srv.*;

public class StompMessagingProtocolImpl implements MessagingProtocol<StompFrame> {

    private Connections<StompFrame> connections;
    private int connectionId;
    private IdGenarator IdGenarator;
    private boolean shouldTerminate;

    public StompMessagingProtocolImpl()
    {

    }

    @Override
    public StompFrame process(StompFrame msg) 
    {
        String command = msg.getCommand();
        if(command == "CONNECT") // the message in a connect message
        {
            ConcurrentHashMap<String, String> headers = msg.getHeaders();
            String ans = connections.logIn(headers.get("login"), headers.get("passcode"), connectionId);
            if(ans.equals("Login successful")) // the login is successful
            {
                ConcurrentHashMap<String, String> ansHeaders = new ConcurrentHashMap<String, String>();
                ansHeaders.put("version", headers.get("accept -version"));
                return new StompFrame("CONNECTED", ansHeaders,"");
            }
            else if(ans.equals("User already logged in"))
            {
                ConcurrentHashMap<String, String> ansHeaders = new ConcurrentHashMap<String, String>();
                ansHeaders.put("message", "could not connect user: " + headers.get("login"));
                return new StompFrame("ERROR", ansHeaders, ans);
            }
            else if(ans.equals("Wrong password"))
            {
                ConcurrentHashMap<String, String> ansHeaders = new ConcurrentHashMap<String, String>();
                ansHeaders.put("message", "could not connect user: " + headers.get("login"));
                return new StompFrame("ERROR", ansHeaders, ans);
            }
        }
        else if(command == "SEND")
        {
            ConcurrentHashMap<String, String> headers = msg.getHeaders();
            if(!connections.subscribedTo(connectionId, headers.get("destination"))) // first checks if the user is subscribed to the topic it's trying to send massege about
            {
                ConcurrentHashMap<String, String> messageHeaders = new ConcurrentHashMap<String, String>();
                StompFrame message = new StompFrame("MESSAGE", messageHeaders, (String)msg.getFrameBody());
                messageHeaders.put("subscription", connections.getSubID(connectionId, headers.get("destination")));
                messageHeaders.put("message-id", Integer.toString(IdGenarator.GenNewId()));
                connections.send(headers.get("destination"), message); // sending the massege to everyone who is subscribe to the topic
                if(headers.containsKey("receipt")) // if a receipt was asked
                {
                    ConcurrentHashMap<String, String> ansHeaders = new ConcurrentHashMap<String, String>();
                    ansHeaders.put("receipt -id:", headers.get("receipt"));
                    return new StompFrame("RECEIPT", headers, "");
                }
                return null;
            }
            else
            {
                ConcurrentHashMap<String, String> ansHeaders = new ConcurrentHashMap<String, String>();
                ansHeaders.put("massage", headers.get(connectionId + "is not subscribed to: " + headers.get("destination")));
                connections.disconnect(connectionId);
                shouldTerminate = true;
                return new StompFrame("ERROR", ansHeaders, msg.toString());
            }
        }
        if(command == "SUBSCRIBE")
        {
            ConcurrentHashMap<String, String> headers = msg.getHeaders();
            String status = connections.subscribe(connectionId, headers.get("destination") , Integer.parseInt(headers.get("id")));
            if(status == "")
            {
                if(headers.containsKey("receipt")) // if a receipt was asked
                {
                    ConcurrentHashMap<String, String> ansHeaders = new ConcurrentHashMap<String, String>();
                    ansHeaders.put("receipt -id", headers.get("receipt"));
                    return new StompFrame("RECEIPT", headers, "");
                }
                return null;
            }
            else
            {
                ConcurrentHashMap<String, String> ansHeaders = new ConcurrentHashMap<String, String>();
                ansHeaders.put("subscription -id", headers.get("id"));
                ansHeaders.put("massage", headers.get(connectionId + "can't subscribe with the id: " +  headers.get("id")));
                connections.disconnect(connectionId);
                shouldTerminate = true;
                return new StompFrame("ERROR", ansHeaders, status);
            }
        }
        if(command == "UNSUBSCRIBE")
        {
            ConcurrentHashMap<String, String> headers = msg.getHeaders();
            if(connections.unsubscribe(connectionId, Integer.parseInt(headers.get("id"))))
            {
                if(headers.containsKey("receipt")) // if a receipt was asked
                {
                    ConcurrentHashMap<String, String> ansHeaders = new ConcurrentHashMap<String, String>();
                    ansHeaders.put("receipt -id", headers.get("receipt"));
                    return new StompFrame("RECEIPT", headers, "");
                }
                return null;
            }
            else
            {
                ConcurrentHashMap<String, String> ansHeaders = new ConcurrentHashMap<String, String>();
                ansHeaders.put("subscription -id", headers.get("id"));
                ansHeaders.put("massage", headers.get(connectionId + "can't unsubscribe with the id: " +  headers.get("id")));
                connections.disconnect(connectionId);
                shouldTerminate = true;
                return new StompFrame("ERROR", ansHeaders,"");
            }
        }
        if(command == "DISCONNECT")
        {
            ConcurrentHashMap<String, String> headers = msg.getHeaders();
            ConcurrentHashMap<String, String> ansHeaders = new ConcurrentHashMap<String, String>();
            ansHeaders.put("receipt -id", headers.get("receipt"));
            connections.disconnect(connectionId);
            shouldTerminate = true;
            return new StompFrame("RECEIPT", ansHeaders,"");
        }
        return null;
    }

    public void start(int connectionId, Connections<StompFrame> connections) 
    {
        this.connectionId = connectionId;
        this.connections = connections;
        this.shouldTerminate = false;
        this.IdGenarator = IdGenarator.getInstance();
    }

    @Override
    public boolean shouldTerminate() 
    {
        return shouldTerminate;
    }
    
}

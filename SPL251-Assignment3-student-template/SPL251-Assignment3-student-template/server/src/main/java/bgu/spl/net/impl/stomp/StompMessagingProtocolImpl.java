package bgu.spl.net.impl.stomp;

import java.util.concurrent.ConcurrentHashMap;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.*;

public class StompMessagingProtocolImpl<T> implements StompMessagingProtocol<StompFrame> {

    private Connections connections;
    int connectionId;

    public StompMessagingProtocolImpl()
    {

    }

    @Override
    public StompFrame process(StompFrame msg) 
    {
        String command = msg.getCommand();
        if(command == "CONNECT")
        {
            
        }
        if(command == "SEND")
        {
            ConcurrentHashMap<String, String> headers = msg.getHeaders();
            if(!connections.subscribedTo(connectionId, headers.get("destination")))
            {
                connections.send(headers.get("destination"), (String)msg.getFrameBody());
                return null;
            }
            else
            {
                ConcurrentHashMap<String, String> ansHeaders = new ConcurrentHashMap<String, String>();
                ansHeaders.put("massage:", headers.get(connectionId + "is not subscribed to: " + headers.get("destination")));
                ERROR(); // TO impl this
                return new StompFrame("ERROR", ansHeaders,"");
            }
        }
        if(command == "SUBSCRIBE")
        {
            ConcurrentHashMap<String, String> headers = msg.getHeaders();
            String status = connections.subscribe(connectionId, headers.get("destination") , Integer.parseInt(headers.get("id")));
            if(status == "")
            {
                return null;
            }
            else
            {
                ConcurrentHashMap<String, String> ansHeaders = new ConcurrentHashMap<String, String>();
                ansHeaders.put("subscription -id:", headers.get("id"));
                ansHeaders.put("massage:", headers.get(connectionId + "can't subscribe with the id: " +  headers.get("id")));
                ERROR(); // TO impl this
                return new StompFrame("ERROR", ansHeaders, status);
            }
        }
        if(command == "UNSUBSCRIBE")
        {
            ConcurrentHashMap<String, String> headers = msg.getHeaders();
            if(connections.unsubscribe(connectionId, Integer.parseInt(headers.get("id"))))
            {
                return null;
            }
            else
            {
                ConcurrentHashMap<String, String> ansHeaders = new ConcurrentHashMap<String, String>();
                ansHeaders.put("subscription -id:", headers.get("id"));
                ansHeaders.put("massage:", headers.get(connectionId + "can't unsubscribe with the id: " +  headers.get("id")));
                ERROR(); // TO impl this
                return new StompFrame("ERROR", ansHeaders,"");
            }
        }
        if(command == "DISCONNECT")
        {
            ConcurrentHashMap<String, String> headers = msg.getHeaders();
            ConcurrentHashMap<String, String> ansHeaders = new ConcurrentHashMap<String, String>();
            ansHeaders.put("receipt -id:", headers.get("id"));
            return new StompFrame("RECEIPT", ansHeaders,"");
        }
        if(command == "CONNECTED")
        {

        }
        if(command == "MESSAGE")
        {

        }
        if(command == "RECEIPT")
        {

        }
        if(command == "ERROR")
        {

        }
        return null;
    }

    public void start(int connectionId, Connections<StompFrame> connections) 
    {
        this.connectionId = connectionId;
        this.connections = connections;
    }

    @Override
    public boolean shouldTerminate() 
    {
        return false;
    }

    public void ERROR()
    {
        // TO DO!
    }
    
}

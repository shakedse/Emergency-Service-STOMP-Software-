package bgu.spl.net.impl.stomp;
import java.net.Socket;
import java.util.Vector;
import java.util.concurrent.ConcurrentHashMap;

import bgu.spl.net.srv.*;

public class ConnectionsImpl <T> implements Connections<T>
{
    public static class ConnectionsHolder
    {
        private static ConnectionsImpl Connections = new ConnectionsImpl<>();

        public static ConnectionsImpl getInstance()
        {
            return Connections;
        }
    }
    
    private ConcurrentHashMap<Integer, ConnectionHandler<T>> ConnectionMap;
    private ConcurrentHashMap<Integer, Vector<String>> IdToTopics;
    private ConcurrentHashMap<String, Vector<Integer>> TopicsToId;
    private ConcurrentHashMap<Integer, ConcurrentHashMap<Integer,String>> subIds;
    private ConcurrentHashMap<Integer, ConcurrentHashMap<String,Integer>> Idsubs;


    public ConnectionsImpl<T> getInstance()
    {
        return ConnectionsHolder.getInstance();
    }

    public ConnectionsImpl()
    {
        ConnectionMap = new ConcurrentHashMap<>();
    }
    
    //creating a new connection between the server and the client
    public Boolean connect(int connectionId, BlockingConnectionHandler connectionHandler)
    {
        if(ConnectionMap.get(connectionId) != null)
            return false;
        this.ConnectionMap.put(connectionId, connectionHandler);      
        IdToTopics.put(connectionId, new Vector<String>());
        subIds.put(connectionId, new ConcurrentHashMap<Integer,String>());
        Idsubs.put(connectionId, new ConcurrentHashMap<String,Integer>());
        return true;
    }

    public Boolean logIn(int connectionId)
    {
        return true; // TO DO
    }

    // subscribing a client to a topic
    public String subscribe(int connectionId, String topic, int subscribeId)
    {
        if(!ConnectionMap.contains(connectionId))
            return "connectionId does not exists";
        TopicsToId.putIfAbsent(topic, new Vector<Integer>());
        if(TopicsToId.get(topic).contains(connectionId))
            return "user already subscribe to this topic";
        subIds.get(connectionId).put(subscribeId, topic);
        Idsubs.get(connectionId).put(topic, subscribeId);
        TopicsToId.get(topic).add(connectionId);
        IdToTopics.get(connectionId).add(topic);
        return ""; 
    }

    //unsubscribe a client from a topic
    public boolean unsubscribe(int connectionId, int subscribeId)
    {
        if(!ConnectionMap.contains(connectionId))
            return false;
        String topic = subIds.get(connectionId).get(subscribeId);
        if(TopicsToId.get(topic) == null)
            return false;
        if(!TopicsToId.get(topic).contains(connectionId))
            return false;

        subIds.get(connectionId).remove(subscribeId);
        Idsubs.get(connectionId).remove(topic);
        TopicsToId.get(topic).remove(connectionId);
        IdToTopics.get(connectionId).remove(topic);
        return true;
    }

    //sending a message to a specific client
    public boolean send(int connectionId, T msg)
    {
        if(ConnectionMap.get(connectionId) == null)
            return false;
        ConnectionMap.get(connectionId).send(msg);
        return true;
    }

    // sending a message to all client subscribe to the channel given
    public void send(String channel, T msg)
    {
        if(TopicsToId.get(channel) != null)
            for(Integer id: TopicsToId.get(channel)) 
                ConnectionMap.get(id).send(msg);
    }

    public void disconnect(int connectionId)
    {
        ConnectionMap.remove(connectionId);
        IdToTopics.remove(connectionId);
        subIds.remove(connectionId);
        Idsubs.remove(connectionId);
        for(String topic: TopicsToId.keySet())
        {
            for(Integer id: TopicsToId.get(topic))
            {
                if(id == connectionId)
                {
                    TopicsToId.get(topic).remove(id);
                    break;
                }  
            }
        }
    }

    public boolean subscribedTo(int connectionId, String topic)
    {
        return TopicsToId.get(topic).contains(connectionId);
    }

    public void addConnection(int connectionId, ConnectionHandler<T> ConnectionHandler)
    {
        ConnectionMap.put(connectionId, ConnectionHandler);
    }
    
    public String getSubID(int connectionId, String topic)
    {
        return Integer.toString(Idsubs.get(connectionId).get(topic));
    }
}

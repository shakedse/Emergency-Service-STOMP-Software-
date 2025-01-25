package bgu.spl.net.impl.stomp;
import java.io.IOException;
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
    private ConcurrentHashMap<String, Vector<Integer>> TopicsToId;
    private ConcurrentHashMap<Integer, ConcurrentHashMap<Integer,String>> subIds;
    private ConcurrentHashMap<Integer, ConcurrentHashMap<String,Integer>> Idsubs;
    private ConcurrentHashMap<String, String> LogedInUserToPassword;
    private ConcurrentHashMap<String, String> UserToPassword;
    private ConcurrentHashMap<Integer, String> IdtoUser;


    public ConnectionsImpl<T> getInstance()
    {
        return ConnectionsHolder.getInstance();
    }

    public ConnectionsImpl()
    {
        ConnectionMap = new ConcurrentHashMap<Integer, ConnectionHandler<T>>();
        TopicsToId = new ConcurrentHashMap<String, Vector<Integer>>();
        subIds = new ConcurrentHashMap<Integer, ConcurrentHashMap<Integer,String>>();
        Idsubs = new ConcurrentHashMap<Integer, ConcurrentHashMap<String,Integer>>();
        LogedInUserToPassword = new ConcurrentHashMap<String, String>();
        UserToPassword = new ConcurrentHashMap<String, String>();
        IdtoUser = new ConcurrentHashMap<Integer, String>();
    }
    
    //creating a new connection between the server and the client
    public Boolean connect(int connectionId, BlockingConnectionHandler connectionHandler)
    {
        if(ConnectionMap.get(connectionId) != null)
            return false;
        ConnectionMap.put(connectionId, connectionHandler);    
        subIds.put(connectionId, new ConcurrentHashMap<Integer,String>());
        Idsubs.put(connectionId, new ConcurrentHashMap<String,Integer>());
        return true;
    }

    public Boolean connect(int connectionId, NonBlockingConnectionHandler connectionHandler)
    {
        if(ConnectionMap.get(connectionId) != null)
            return false;
        ConnectionMap.put(connectionId, connectionHandler);    
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
        if(!ConnectionMap.containsKey(connectionId))
            return "connectionId does not exists";
        TopicsToId.putIfAbsent(topic, new Vector<Integer>());
        if(TopicsToId.get(topic).contains(connectionId))
            return "user already subscribe to this topic";
        subIds.get(connectionId).put(subscribeId, topic);
        Idsubs.get(connectionId).put(topic, subscribeId);
        TopicsToId.get(topic).add(connectionId);
        return "successful subscribe"; 
    }

    //unsubscribe a client from a topic
    public boolean unsubscribe(int connectionId, int subscribeId)
    {
        if(!ConnectionMap.containsKey(connectionId))
            return false;
        String topic = subIds.get(connectionId).get(subscribeId);
        if(topic == null)
            return false;
        if(TopicsToId.containsKey(topic) == false)
            return false;
        if(!TopicsToId.get(topic).contains(connectionId))
            return false;

        subIds.get(connectionId).remove(subscribeId);
        Idsubs.get(connectionId).remove(topic);
        int toRemoveIndex = TopicsToId.get(topic).indexOf(connectionId);
        TopicsToId.get(topic).remove(toRemoveIndex);
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
        subIds.remove(connectionId);
        Idsubs.remove(connectionId);
        LogedInUserToPassword.remove(IdtoUser.get(connectionId)); // need to log out disconnected users
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
        ConnectionMap.remove(connectionId);
    }

    public boolean subscribedTo(int connectionId, String topic)
    {
        return TopicsToId.get(topic).contains(connectionId);
    }


    public String getSubID(int connectionId, String topic)
    {
        return Integer.toString(Idsubs.get(connectionId).get(topic));
    }

    public String logIn(String user, String password, int connectionId)
    {
        if(UserToPassword.containsKey(user) == false) // new user
        {
            LogedInUserToPassword.put(user, password); // adding to loged in
            UserToPassword.put(user, password); // adding to total users
            IdtoUser.put(connectionId, user);
            return "Login successful";
        }
        else
        {
            if(LogedInUserToPassword.containsKey(user) == true) // old user and loged in
            {
                return "User already logged in";
            }
            else
            {
                if(!UserToPassword.get(user).equals(password)) // old user, not loged in, wrong password
                {
                    return "Wrong password";
                }
                else // old user, loged out, right password
                {
                    LogedInUserToPassword.put(user, password); // adding to loged in
                    IdtoUser.put(connectionId, user);
                    return "Login successful";
                }
            }
        }
    }
}

package bgu.spl.net.impl.stomp;
import java.net.Socket;
import java.util.concurrent.ConcurrentHashMap;

import bgu.spl.net.srv.*;

public class ConnectionsImpl <T> implements Connections<T>
{
    private ConcurrentHashMap<Integer,ConnectionHandler<T>> ConnectionMap;

    public ConnectionsImpl()
    {
        ConnectionMap = new ConcurrentHashMap<>();
    }
    
    //sending a message to a specific client
    public boolean send(int connectionId, T msg)
    {
        if(ConnectionMap.get(connectionId) == null)
            return false;
        ConnectionMap.get(connectionId).send(msg);
        return true;
    }

    //creating a new connection between the server and the client
    public Boolean connect(int connectionId)
    {
        if(ConnectionMap.get(connectionId) != null)
            return false;
        this.ConnectionMap.put(connectionId, new ConnectionHandlerImplTPC<T>(
                                                        new Socket(), new MessageEncoderDecoder<T>(), new StompMessagingProtocolImpl<T>()));
        return true;
    }

    public void send(String channel, T msg)
    {
        for (ConnectionHandler<T> connection : ConnectionMap.values())
        {
            if(connection.isSubscribed(channel))
            {
                connection.send(msg);
            }
        }
    }

    public void disconnect(int connectionId)
    {

    }
}

package bgu.spl.net.impl.stomp;

public class ConnectionHandlerImplReactor 
{
    private int connectionId;
    private ConnectionHandlerImplReactor connectionHandler;
    private ConnectionsImpl connections;
    private boolean shouldTerminate;

    public ConnectionHandlerImplReactor(int connectionId, ConnectionHandlerImplReactor connectionHandler, ConnectionsImpl connections)
    {
        this.connectionId = connectionId;
        this.connectionHandler = connectionHandler;
        this.connections = connections;
        this.shouldTerminate = false;
    }

    public void send(String msg)
    {
        connections.send(connectionId, msg);
    }

    public void disconnect()
    {
        connections.disconnect(connectionId);
        shouldTerminate = true;
    }

    public boolean shouldTerminate()
    {
        return shouldTerminate;
    }

}

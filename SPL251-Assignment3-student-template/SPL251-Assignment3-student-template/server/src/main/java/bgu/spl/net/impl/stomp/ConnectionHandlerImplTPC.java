package bgu.spl.net.impl.stomp;

import java.io.IOException;
import java.net.Socket;
import java.util.List;
import java.util.logging.SocketHandler;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.*;

public class ConnectionHandlerImplTPC<T> implements ConnectionHandler<T> {
    private Socket socket;
    private MessageEncoderDecoder<T> encdec;
    private StompMessagingProtocol<T> protocol;
    private List<String> topics;

    public ConnectionHandlerImplTPC(Socket socket, MessageEncoderDecoder<T> encdec, StompMessagingProtocol<T> protocol)
    {
        this.socket = socket;
        this.encdec = encdec;
        this.protocol = protocol;
    }
    
    public void close() throws IOException
    {
        socket.close();
    }

    public void send(T msg)
    {

    }

    //checking if the client is subscribed to a specific channel
    public boolean isSubscribed(String channel)
    {
        if(topics.contains(channel))
            return true;
        return false;
    }

    //subscribing the client to a specific channel
    public void subscribe(String channel)
    {
        topics.add(channel);
    }

    public void run()
    {

    }

}

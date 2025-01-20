package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.*;

public class StompMessagingProtocolImpl<T> implements StompMessagingProtocol<T> {
    @Override
    public void process(T msg) 
    {

    }

    public void start(int connectionId, Connections<T> connections) 
    {

    }

    @Override
    public boolean shouldTerminate() 
    {
        return false;
    }
    
}

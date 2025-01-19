package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;

public class StompMessagingProtocolImpl <T> implements StompMessagingProtocol<T> {
    @Override
    public String process(String msg) {
        return null;
    }
    public void start(int connectionId, Connections<T> connections) 
    {
    }

    @Override
    public boolean shouldTerminate() {
        return false;
    }
    
}

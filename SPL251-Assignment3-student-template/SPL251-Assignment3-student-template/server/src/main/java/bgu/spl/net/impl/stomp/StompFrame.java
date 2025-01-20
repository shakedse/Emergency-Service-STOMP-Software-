package bgu.spl.net.impl.stomp;

import java.util.concurrent.ConcurrentHashMap;

public class StompFrame {
    private String StompCommand;
    private ConcurrentHashMap<String, String> headers;
    private String FrameBody;

    public StompFrame(String StompCommand, ConcurrentHashMap<String, String> headers, String FrameBody)
    {
        this.StompCommand = StompCommand;
        this.headers = headers;
        this.FrameBody = FrameBody;
    }
}


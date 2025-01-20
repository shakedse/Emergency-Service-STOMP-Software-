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

    public String getCommand()
    {
        return StompCommand;
    }

    public ConcurrentHashMap<String, String> getHeaders()
    {
        return headers;
    }

    public String getFrameBody()
    {
        return FrameBody;
    }

    public String toString()
    {
        String ans = "/n";
        ans = ans + StompCommand + "/n";
        for(String header: headers.keySet())
        {
            ans = ans + header + ":" + headers.get(header) + "/n";
        }
        ans = ans + FrameBody;
        return ans;
    }
}


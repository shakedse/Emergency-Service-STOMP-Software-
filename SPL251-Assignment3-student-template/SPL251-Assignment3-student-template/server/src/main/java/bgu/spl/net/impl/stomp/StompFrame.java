package bgu.spl.net.impl.stomp;

import java.util.concurrent.ConcurrentHashMap;

public class StompFrame {
    private String StompCommand;
    private ConcurrentHashMap<String, String> headers;
    private String FrameBody;
    private char EndingChar;

    public StompFrame(String StompCommand, ConcurrentHashMap<String, String> headers, String FrameBody)
    {
        this.StompCommand = StompCommand;
        this.headers = headers;
        this.FrameBody = FrameBody;
        this.EndingChar = '\u0000';
    }

    public StompFrame(String fullStringFrame)
    {
        // Split the frame into lines
        String[] lines = fullStringFrame.split("\n");
    
        // The first line is the command
        if (lines.length > 0) {
            this.StompCommand = lines[0].trim();
        } else {
            throw new IllegalArgumentException("Invalid STOMP frame: missing command");
        }
    
        // Initialize headers map
        this.headers = new ConcurrentHashMap<>();
    
        // Parse headers
        int i = 1;
        while (i < lines.length && !lines[i].isEmpty()) { // Empty line indicates end of headers
            String line = lines[i];
            int colonIndex = line.indexOf(":");
            if (colonIndex == -1) {
                throw new IllegalArgumentException("Invalid header format: " + line);
            }
    
            String key = line.substring(0, colonIndex).trim();
            String value = line.substring(colonIndex + 1).trim();
            headers.put(key, value);
            i++;
        }
    
        // Skip the empty line after headers
        i++;
    
        // Remaining lines (if any) are the body
        StringBuilder bodyBuilder = new StringBuilder();
        while (i < lines.length && lines[i].charAt(0) != '\u0000') {
            bodyBuilder.append(lines[i]);
            bodyBuilder.append("\n");
            i++;
        }
    
        // Remove trailing newline in the body (if present)
        if (bodyBuilder.length() > 0 && bodyBuilder.charAt(bodyBuilder.length() - 1) == '\n') {
            bodyBuilder.setLength(bodyBuilder.length() - 1);
        }
    
        this.FrameBody = bodyBuilder.toString();
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

    @Override
    public String toString() 
    {
        StringBuilder ans = new StringBuilder();

        // Add command
        ans.append(StompCommand).append("\n");

        // Add headers
        for (String header : headers.keySet()) {
            ans.append(header).append(":").append(headers.get(header)).append("\n");
        }

        // Add a blank line between headers and body
        ans.append("\n");

        // Add body
        if (FrameBody != null && !FrameBody.isEmpty()) {
            ans.append(FrameBody);
        }

        // Add the ending null character
        ans.append('\u0000');

        return ans.toString();
    }
}


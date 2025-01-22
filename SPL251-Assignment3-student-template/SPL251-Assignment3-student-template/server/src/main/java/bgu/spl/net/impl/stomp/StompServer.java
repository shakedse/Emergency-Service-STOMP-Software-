package bgu.spl.net.impl.stomp;

import java.util.concurrent.ConcurrentHashMap;
import java.util.function.Supplier;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.MessagingProtocol;
import bgu.spl.net.srv.BaseServer;
import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {

        ConcurrentHashMap<String, String> headers = new ConcurrentHashMap<>();
        headers.put("destination", "/topic/chat");
        headers.put("content-type", "text/plain");

        StompFrame frame = new StompFrame("SEND", headers, "Hello, World!");

        String frameString = frame.toString();
        System.out.println(frameString);

        StompFrame parsedFrame = new StompFrame(frameString);

        // Verify
        System.out.println(parsedFrame.getCommand()); // Should print "SEND"
        System.out.println(parsedFrame.getHeaders()); // Should print headers
        System.out.println(parsedFrame.getFrameBody()); // Should print "Hello, World!"


        if(args.length < 2)
        {
            System.out.println("Usage: StompServer <port>");
            return;
        }
        if(args[1] == "TPC")
        {
            int port = Integer.parseInt(args[2]); // true????
            Supplier<MessagingProtocol<StompFrame>> protocolSupplier = StompMessagingProtocolImpl::new;

            // Create a Supplier for MessageEncoderDecoder
            Supplier<MessageEncoderDecoder<StompFrame>> encoderDecoderSupplier = MessageEncoderDecoderImpl::new;

            // Start the server
            Server<StompFrame> server = Server.threadPerClient(port, protocolSupplier, encoderDecoderSupplier);
            server.serve();        
        }
        if(args[1] == "Reactor")
        {

        }
    }
}

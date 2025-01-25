package bgu.spl.net.impl.stomp;

import java.util.concurrent.ConcurrentHashMap;
import java.util.function.Supplier;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.MessagingProtocol;
import bgu.spl.net.srv.BaseServer;
import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {

        if(args.length < 2)
        {
            System.out.println("Usage: StompServer <port>");
            return;
        }
        int port = Integer.parseInt(args[0]);
        String type = args[1];
        if(type.equals("tpc"))
        {
            Supplier<MessagingProtocol<StompFrame>> protocolSupplier = StompMessagingProtocolImpl::new;

            // Create a Supplier for MessageEncoderDecoder
            Supplier<MessageEncoderDecoder<StompFrame>> encoderDecoderSupplier = MessageEncoderDecoderImpl::new;

            // Start the server
            Server<StompFrame> server = Server.threadPerClient(port, protocolSupplier, encoderDecoderSupplier);
            server.serve();        
        }
        if(type.equals("reactor"))
        {
            Supplier<MessagingProtocol<StompFrame>> protocolSupplier = StompMessagingProtocolImpl::new;

            // Create a Supplier for MessageEncoderDecoder
            Supplier<MessageEncoderDecoder<StompFrame>> encoderDecoderSupplier = MessageEncoderDecoderImpl::new;

            // Start the server
            Server<StompFrame> server = Server.reactor(10, port, protocolSupplier, encoderDecoderSupplier);
            server.serve(); 
        }
    }
}

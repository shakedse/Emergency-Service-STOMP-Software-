package bgu.spl.net.srv;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.MessagingProtocol;
import bgu.spl.net.impl.stomp.ConnectionsImpl.ConnectionsHolder;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.net.Socket;

public class BlockingConnectionHandler<T> implements Runnable, ConnectionHandler<T> {

    private final MessagingProtocol<T> protocol;
    private final MessageEncoderDecoder<T> encdec;
    private final Socket sock;
    private BufferedInputStream in;
    private BufferedOutputStream out;
    private volatile boolean connected = true;
    
    public BlockingConnectionHandler(Socket sock, MessageEncoderDecoder<T> reader, MessagingProtocol<T> protocol) {
        this.sock = sock;
        this.encdec = reader;
        this.protocol = protocol;
    }

    @Override
    public void run() {
        try (Socket sock = this.sock) { //just for automatic closing
            int read;

            in = new BufferedInputStream(sock.getInputStream());
            out = new BufferedOutputStream(sock.getOutputStream());

            System.out.println("we are in connection handler");

            while (!protocol.shouldTerminate() && connected && (read = in.read()) >= 0) {
                T nextMessage = encdec.decodeNextByte((byte) read);
                if (nextMessage != null) {
                    T response = protocol.process(nextMessage);
                    if (response != null) { 
                        out.write(encdec.encode(response));
                        out.flush();
                    }
                }
            }

            close();
        } catch (IOException ex) {
            ex.printStackTrace();
        }
    }

    @Override
    public void close() throws IOException {
        System.out.println("the server connectionhandler is closed...");
        connected = false;
        sock.close();
    }

    @Override
    public synchronized void send(T msg) { 
        try {
            if (msg != null) {
                byte[] encodedMessage = encdec.encode(msg);
                out.write(encodedMessage);
                out.flush(); // Ensure data is sent immediately
            }
        } catch (IOException e) {
            e.printStackTrace(); // Log the error
            connected = false; // Mark the handler as disconnected on failure
        }
    }

    public void Start(int connectionId)
    {
        protocol.start(connectionId, ConnectionsHolder.getInstance());
    }
    
}

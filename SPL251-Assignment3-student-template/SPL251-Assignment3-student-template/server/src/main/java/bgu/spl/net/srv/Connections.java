package bgu.spl.net.srv;

import java.io.IOException;

public interface Connections<T> {

    boolean send(int connectionId, T msg);

    void send(String channel, T msg);

    void disconnect(int connectionId);

    // we added
    String subscribe(int connectionId, String topic, int subscribeId);

    boolean unsubscribe(int connectionId, int subscribeId);

    boolean subscribedTo(int connectionId, String topic);
}

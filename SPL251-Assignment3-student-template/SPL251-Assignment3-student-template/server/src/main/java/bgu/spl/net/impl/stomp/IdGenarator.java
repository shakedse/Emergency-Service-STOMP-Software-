package bgu.spl.net.impl.stomp;

import java.util.concurrent.atomic.AtomicInteger;

public class IdGenarator {

    // Static nested class for holding the singleton instance
    public static class IdGenaratorHolder {
        private static final IdGenarator INSTANCE = new IdGenarator();
    }

    // Atomic integer to ensure thread-safe ID generation
    private AtomicInteger ID;

    // Private constructor to prevent external instantiation
    private IdGenarator() {
        ID = new AtomicInteger(0); // Proper initialization
    }

    // Public method to get the singleton instance
    public static IdGenarator getInstance() {
        return IdGenaratorHolder.INSTANCE;
    }

    // Generate a new unique ID
    public int GenNewId() {
        return ID.getAndIncrement();
    }
}

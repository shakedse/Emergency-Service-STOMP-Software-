package bgu.spl.net.impl.stomp;

import java.util.concurrent.atomic.AtomicInteger;

public class IdGenarator
{
    public static class IdGenaratorHolder
    {
        private static IdGenarator IdGenarator = new IdGenarator();

        public static IdGenarator getInstance()
        {
            return IdGenarator;
        }
    }

    private AtomicInteger ID;

    public IdGenarator()
    {
        ID.set(0);
    }

    public static IdGenarator getInstance()
    {
        return IdGenaratorHolder.getInstance();
    }

    public int GenNewId()
    {
        return ID.getAndIncrement();
    }


}
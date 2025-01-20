package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.MessageEncoderDecoder;

public class MessageEncoderDecoderImpl<T> implements MessageEncoderDecoder<T> {
    public T decodeNextByte(byte nextByte)
    {
        return null;
    }

    /**
     * encodes the given message to bytes array
     *
     * @param message the message to encode
     * @return the encoded bytes
     */
    public byte[] encode(T message)
    {
        return null;
    }
}

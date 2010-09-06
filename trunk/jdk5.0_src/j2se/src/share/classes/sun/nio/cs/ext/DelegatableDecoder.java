package sun.nio.cs.ext;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.CoderResult;

/**
 * A decoder that can be delegated to by another decoder
 * when normal inheritance cannot be used.
 * Used by autodecting decoders.
 */
interface DelegatableDecoder {
    CoderResult decodeLoop(ByteBuffer src, CharBuffer dst);
    void implReset();
    CoderResult implFlush(CharBuffer out);
}

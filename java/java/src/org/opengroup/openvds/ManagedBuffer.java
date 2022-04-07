/*
 * Copyright 2022 The Open Group
 * Copyright 2022 Bluware, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.opengroup.openvds;

import java.nio.*;

/**
 * This class maintains a Direct ByteBuffer with natively allocated memory, intended
 * for deterministic cleanup (aka try-with-resources). Using the ByteBuffer
 * obtained from this class after it has been closed/disposed
 * will result in undefined behavior, most likely crashing your program.
 */
public class ManagedBuffer extends ManagedBase implements AutoCloseable {
	
	private ByteBuffer bytebuffer;
	
	private int byteoffset;

	public static ByteBuffer ensureByteBufferValid(ByteBuffer buffer, int byteoffset) {
		ManagedBase.requireNonNull(buffer, "buffer may not be null");
		if (buffer.capacity() <= 0) {
			throw new IllegalArgumentException("buffer has no valid capacity");
		}
		if (byteoffset >= buffer.capacity()) {
			throw new IllegalArgumentException("byteoffset greater than buffer capacity");
		}
		if (!buffer.isDirect()) {
			throw new IllegalArgumentException("buffer is not direct");
		}
		return buffer;
	}

	public static ByteBuffer ensureByteBufferValid(ByteBuffer buffer) {
		return ensureByteBufferValid(buffer, 0);
	}

	public static ByteBuffer ensureByteBufferValidOrNull(ByteBuffer buffer, int byteoffset) {
		if (buffer == null) {
			return null;
		} else {
			return ensureByteBufferValid(buffer, byteoffset);
		}
	}

	public static ByteBuffer ensureByteBufferValidOrNull(ByteBuffer buffer) {
		return ensureByteBufferValidOrNull(buffer , 0);
	}

	ManagedBuffer(ByteBuffer bytebuffer, int byteoffset) {
		super(0);
		this.bytebuffer = ensureByteBufferValidOrNull(bytebuffer, byteoffset);
		this.byteoffset = byteoffset;
	}

	private native static Object[] ctorImpl(long capacity);
	private native void dtorImpl(long native_object, boolean is_disposing);

	/**
	 * Create a ManagedBuffer backed by a DirectByteBuffer whose memory will be free'd on close/dispose
	 * @param capacity The new buffer's capacity, in bytes.
	 */
	public ManagedBuffer(long capacity) {
		super(ctorImpl(capacity));
		this.byteoffset = 0;
		this.bytebuffer = (ByteBuffer)this.getContext();
	}

	@Override
	protected void onDisposing(long native_object, boolean is_disposing) {
		this.bytebuffer = null;
		dtorImpl(native_object, is_disposing);
	}

	@Override
	public synchronized void dispose() {
		if (this.bytebuffer != null) {
			this.bytebuffer = null;
			super.dispose();
		}
	}

	public void close() {
		dispose();
	}

	public ByteBuffer getByteBuffer() {
		if (this.bytebuffer == null) {
			throw new RuntimeException("Object has been disposed");
		}
		return this.bytebuffer;
	}

	int getByteOffset() {
		return this.byteoffset;
	}
	
	public ShortBuffer asShortBuffer() {
		return getByteBuffer().asShortBuffer();
	}

	public IntBuffer asIntBuffer() {
		return getByteBuffer().asIntBuffer();
	}

	public LongBuffer asLongBuffer() {
		return getByteBuffer().asLongBuffer();
	}

	public FloatBuffer asFloatBuffer() {
		return getByteBuffer().asFloatBuffer();
	}

	public DoubleBuffer asDoubleBuffer() {
		return getByteBuffer().asDoubleBuffer();
	}
	
	public byte get(int offset) {
		return getByteBuffer().get(this.byteoffset + offset);
	}
	
	public void put(int offset, byte value) {
		getByteBuffer().put(this.byteoffset + offset, value);
	}
	
	public short getShort(int offset) {
		return getByteBuffer().getShort(this.byteoffset + offset);
	}
	
	public void putShort(int offset, short value) {
		getByteBuffer().putShort(this.byteoffset + offset, value);
	}
	
	public int getInt(int offset) {
		return getByteBuffer().getInt(this.byteoffset + offset);
	}
	
	public void putInt(int offset, int value) {
		getByteBuffer().putInt(this.byteoffset + offset, value);
	}
	
	public long getLong(int offset) {
		return getByteBuffer().getLong(this.byteoffset + offset);
	}
	
	public void putLong(int offset, long value) {
		getByteBuffer().putLong(this.byteoffset + offset, value);
	}
	
	public float getFloat(int offset) {
		return getByteBuffer().getFloat(this.byteoffset + offset);
	}
	
	public void putFloat(int offset, float value) {
		getByteBuffer().putFloat(this.byteoffset + offset, value);
	}
	
	public double getDouble(int offset) {
		return getByteBuffer().getDouble(this.byteoffset + offset);
	}
	
	public void putDouble(int offset, double value) {
		getByteBuffer().putDouble(this.byteoffset + offset, value);
	}

	public void put(int offset, byte[] array) {
		for (int i = 0; i < array.length; ++i) {
			put(i * Byte.BYTES + offset, array[i]);
		}
	}
	public void put(int offset, short[] array) {
		for (int i = 0; i < array.length; ++i) {
			putShort(i * Short.BYTES + offset, array[i]);
		}
	}

	public void put(int offset, int[] array) {
		for (int i = 0; i < array.length; ++i) {
			putInt(i * Integer.BYTES + offset, array[i]);
		}
	}

	public void put(int offset, long[] array) {
		for (int i = 0; i < array.length; ++i) {
			putLong(i * Long.BYTES + offset, array[i]);
		}
	}
	public void put(int offset, float[] array) {
		for (int i = 0; i < array.length; ++i) {
			putFloat(i * Float.BYTES + offset, array[i]);
		}
	}

	public void put(int offset, double[] array) {
		for (int i = 0; i < array.length; ++i) {
			putDouble(i * Double.BYTES + offset, array[i]);
		}
	}
}

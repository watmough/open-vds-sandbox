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
 * This class maintains a ByteBuffer with natively allocated memory, intended
 * for deterministic cleanup (aka try-with-resources). Using the ByteBuffer
 * obtained from this class after it has been closed/disposed
 * will result in undefined behavior, most likely crashing your program.
 */
public class ManagedBuffer extends ManagedBase implements AutoCloseable {
	
	private ByteBuffer bytebuffer;
	
	private int byteoffset;

	ManagedBuffer(ByteBuffer bytebuffer, int byteoffset) {
		super(0);
		this.bytebuffer = bytebuffer;
		this.byteoffset = byteoffset;
	}

	private native static long ctorImpl(long capacity);
	private native ByteBuffer getBufferRefImpl(long native_object);
	private native void deleteBufferRefImpl(long native_object);
	private native void dtorImpl(long native_object, boolean is_disposing );

	/**
	 * Create aManagedBuffer backed by a ByteBuffer whose memory will be free'd on close/dispose
	 * @param capacity The new buffer's capacity, in bytes.
	 */
	public ManagedBuffer(long capacity) {
		super(ctorImpl(capacity));
		this.byteoffset = 0;
		this.bytebuffer = getBufferRefImpl(getNativeObject());
		this.bytebuffer.order(ByteOrder.nativeOrder());
		deleteBufferRefImpl(getNativeObject());
	}

	@Override
	protected void onDisposing(long native_object, boolean is_disposing) {
		this.bytebuffer = null;
		dtorImpl(native_object, is_disposing);
	}

	@Override
	public synchronized void dispose() {
		if (this.bytebuffer != null) {
			ByteBuffer byteBuffer = this.bytebuffer;
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
		return this.bytebuffer.get(this.byteoffset + offset);
	}
	
	public void put(int offset, byte value) {
		this.bytebuffer.put(this.byteoffset + offset, value);
	}
	
	public short getShort(int offset) {
		return this.bytebuffer.getShort(this.byteoffset + offset);
	}
	
	public void putShort(int offset, short value) {
		this.bytebuffer.putShort(this.byteoffset + offset, value);
	}
	
	public int getInt(int offset) {
		return this.bytebuffer.getInt(this.byteoffset + offset);
	}
	
	public void putInt(int offset, int value) {
		this.bytebuffer.putInt(this.byteoffset + offset, value);
	}
	
	public long getLong(int offset) {
		return this.bytebuffer.getLong(this.byteoffset + offset);
	}
	
	public void putLong(int offset, long value) {
		this.bytebuffer.putLong(this.byteoffset + offset, value);
	}
	
	public float getFloat(int offset) {
		return this.bytebuffer.getFloat(this.byteoffset + offset);
	}
	
	public void putFloat(int offset, float value) {
		this.bytebuffer.putFloat(this.byteoffset + offset, value);
	}
	
	public double getDouble(int offset) {
		return this.bytebuffer.getDouble(this.byteoffset + offset);
	}
	
	public void putDouble(int offset, double value) {
		this.bytebuffer.putDouble(this.byteoffset + offset, value);
	}

	public void put(byte[] array) {
		for (int i = 0; i < array.length; ++i) {
			put(i, array[i]);
		}
	}
	public void put(short[] array) {
		for (int i = 0; i < array.length; ++i) {
			putShort(i, array[i]);
		}
	}

	public void put(int[] array) {
		for (int i = 0; i < array.length; ++i) {
			putInt(i, array[i]);
		}
	}

	public void put(long[] array) {
		for (int i = 0; i < array.length; ++i) {
			putLong(i, array[i]);
		}
	}
	public void put(float[] array) {
		for (int i = 0; i < array.length; ++i) {
			putFloat(i, array[i]);
		}
	}

	public void put(double[] array) {
		for (int i = 0; i < array.length; ++i) {
			putDouble(i, array[i]);
		}
	}

}

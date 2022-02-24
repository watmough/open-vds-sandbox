/*
 * Copyright 2021 The Open Group
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

public class ByteBufferProxy {
	
	private ByteBuffer bytebuffer;
	
	private int byteoffset;

	public ByteBufferProxy(ByteBuffer bytebuffer, int byteoffset) {
		this.bytebuffer = bytebuffer;
		this.byteoffset = byteoffset;
	}

	public ByteBufferProxy(int capacity) {
		this.bytebuffer = allocateBuffer(capacity);
		this.byteoffset = 0;
	}

	/**
	 * Allocate a direct buffer with native byte order.
	 * @param capacity The new buffer's capacity, in bytes.
	 * @return A new direct buffer with native byte order.
	 */
	public static ByteBuffer allocateBuffer(long capacity) {
		return ByteBuffer.allocateDirect((int)capacity).order(ByteOrder.nativeOrder());
	}

	public ByteBuffer getByteBuffer() {
		return this.bytebuffer;
	}
	
	public int getByteOffset() {
		return this.byteoffset;
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

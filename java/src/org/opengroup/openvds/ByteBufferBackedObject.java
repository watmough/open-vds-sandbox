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

/**
 * This is the base class for structured arithmetic types such as FloatVector3, FloatRange etc. used to
 * interface with the native OpenVDS library.
 */
public class ByteBufferBackedObject {

	private ManagedBuffer managedBuffer;
	
	public ByteBufferBackedObject() {
		this.managedBuffer = new ManagedBuffer(null, 0);
	}
	
	public ByteBufferBackedObject(ByteBuffer bytebuffer, int byteoffset, int bytesize) {
		if (bytebuffer == null) {
			throw new NullPointerException("bytebuffer");
		}
		if (bytebuffer.capacity() < byteoffset + bytesize) {
			throw new IllegalArgumentException("Invalid byteoffset/bytesize for this ByteBuffer");
		}
		this.managedBuffer = new ManagedBuffer(bytebuffer, byteoffset);
	}
	
	public ManagedBuffer getManagedBuffer() {
		return this.managedBuffer;
	}
	
	public int getByteBufferOffset() {
		return this.managedBuffer.getByteOffset();
	}
	
	public ByteBuffer getBackingByteBuffer() {
		return this.managedBuffer.getByteBuffer();
	}
	
	protected void setByteBuffer(ByteBuffer bytebuffer, int byteoffset) {
		this.managedBuffer = new ManagedBuffer(bytebuffer, byteoffset);
	}
	
	protected void createByteBuffer(int capacity) {
		this.managedBuffer = new ManagedBuffer(ByteBuffer.allocateDirect(capacity).order(ByteOrder.nativeOrder()), 0);
	}
}

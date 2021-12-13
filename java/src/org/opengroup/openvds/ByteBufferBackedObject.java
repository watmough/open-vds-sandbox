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

public class ByteBufferBackedObject {

	private ByteBufferProxy bytebufferproxy;
	
	public ByteBufferBackedObject() {
		this.bytebufferproxy = new ByteBufferProxy(null, 0);
	}
	
	public ByteBufferProxy getByteBufferProxy() {
		return this.bytebufferproxy;
	}
	
	public long getByteBufferOffset() {
		return this.bytebufferproxy.getByteOffset();
	}
	
	public ByteBuffer getBackingByteBuffer() {
		return this.bytebufferproxy.getByteBuffer();
	}
	
	protected void setByteBuffer(ByteBuffer bytebuffer, long byteoffset) {
		this.bytebufferproxy = new ByteBufferProxy(bytebuffer, (int)byteoffset);
	}
	
	protected void createByteBuffer(int capacity) {
		this.bytebufferproxy = new ByteBufferProxy(ByteBuffer.allocateDirect(capacity), 0);
	}
}

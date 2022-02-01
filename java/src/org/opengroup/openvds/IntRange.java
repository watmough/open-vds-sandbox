
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

public class IntRange extends ByteBufferBackedObject {

    public IntRange() {
        this.createByteBuffer(Integer.BYTES * 2 * 1);
    }

    public IntRange(int min, int max) {
        this.createByteBuffer(Integer.BYTES * 2);
        this.set(min, max);
    }

    public IntRange(java.nio.ByteBuffer bytebuffer, int byteoffset) {
        super(bytebuffer, byteoffset, Integer.BYTES * 2);
    }

    public IntRange(IntRange rhs) {
        this.createByteBuffer(Integer.BYTES * 2);
        this.set(rhs.getMin(), rhs.getMax());
    }

    public boolean equals(Object other) {
        if (other == this) return true;
        if (other == null) return false;
        if (getClass() != other.getClass()) return false;
        IntRange real_other = (IntRange)other;
        return (this.getMin() == real_other.getMin() &&
                this.getMax() == real_other.getMax());
    }

    void put(ByteBufferProxy bytebufferproxy, int byteoffset) {
        bytebufferproxy.putInt(0 * Integer.BYTES + byteoffset, this.getMin());
        bytebufferproxy.putInt(1 * Integer.BYTES + byteoffset, this.getMax());
    }

    public void set(int min, int max) {
        this.getByteBufferProxy().putInt(0 * Integer.BYTES, min);
        this.getByteBufferProxy().putInt(1 * Integer.BYTES, max);
    }

    public void setMin(int value) {
        this.getByteBufferProxy().putInt(0 * Integer.BYTES, value);
    }


    public void setMax(int value) {
        this.getByteBufferProxy().putInt(1 * Integer.BYTES, value);
    }

    public int getMin() {
        return this.getByteBufferProxy().getInt(0 * Integer.BYTES);
    }


    public int getMax() {
        return this.getByteBufferProxy().getInt(1 * Integer.BYTES);
    }

    public String toString() {
        String value = "(";
        for (int i = 0; i < 2; ++i)
        {
            if (i > 0)
                value = value + ", ";
            value = value + this.getByteBufferProxy().getInt(i * Integer.BYTES);
        }
        value = value + ")";
        return value;
    }
}



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

public class IntVector3 extends ByteBufferBackedObject {

    public IntVector3() {
        this.createByteBuffer(Integer.BYTES * 3 * 1);
    }

    public IntVector3(int x, int y, int z) {
        this.createByteBuffer(Integer.BYTES * 3);
        this.set(x, y, z);
    }

    public IntVector3(java.nio.ByteBuffer bytebuffer, int byteoffset) {
        super(bytebuffer, byteoffset, Integer.BYTES * 3);
    }

    public IntVector3(IntVector3 rhs) {
        this.createByteBuffer(Integer.BYTES * 3);
        this.set(rhs.getX(), rhs.getY(), rhs.getZ());
    }

    public boolean equals(Object other) {
        if (other == this) return true;
        if (other == null) return false;
        if (getClass() != other.getClass()) return false;
        IntVector3 real_other = (IntVector3)other;
        return (this.getX() == real_other.getX() &&
                this.getY() == real_other.getY() &&
                this.getZ() == real_other.getZ());
    }

    void put(ByteBufferProxy bytebufferproxy, int byteoffset) {
        bytebufferproxy.putInt(0 * Integer.BYTES + byteoffset, this.getX());
        bytebufferproxy.putInt(1 * Integer.BYTES + byteoffset, this.getY());
        bytebufferproxy.putInt(2 * Integer.BYTES + byteoffset, this.getZ());
    }

    public void set(int x, int y, int z) {
        this.getByteBufferProxy().putInt(0 * Integer.BYTES, x);
        this.getByteBufferProxy().putInt(1 * Integer.BYTES, y);
        this.getByteBufferProxy().putInt(2 * Integer.BYTES, z);
    }

    public void setX(int value) {
        this.getByteBufferProxy().putInt(0 * Integer.BYTES, value);
    }


    public void setY(int value) {
        this.getByteBufferProxy().putInt(1 * Integer.BYTES, value);
    }


    public void setZ(int value) {
        this.getByteBufferProxy().putInt(2 * Integer.BYTES, value);
    }

    public int getX() {
        return this.getByteBufferProxy().getInt(0 * Integer.BYTES);
    }


    public int getY() {
        return this.getByteBufferProxy().getInt(1 * Integer.BYTES);
    }


    public int getZ() {
        return this.getByteBufferProxy().getInt(2 * Integer.BYTES);
    }

    public String toString() {
        String value = "(";
        for (int i = 0; i < 3; ++i)
        {
            if (i > 0)
                value = value + ", ";
            value = value + this.getByteBufferProxy().getInt(i * Integer.BYTES);
        }
        value = value + ")";
        return value;
    }
}


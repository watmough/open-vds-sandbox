
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

public class DoubleVector3 extends ByteBufferBackedObject {

    public DoubleVector3() {
        this.createByteBuffer(Double.BYTES * 3);
        this.set((double)0, (double)0, (double)0);
    }

    public DoubleVector3(double x, double y, double z) {
        this.createByteBuffer(Double.BYTES * 3);
        this.set(x, y, z);
    }

    public DoubleVector3(java.nio.ByteBuffer bytebuffer, int byteoffset) {
        super(bytebuffer, byteoffset, Double.BYTES * 3);
    }

    public DoubleVector3(DoubleVector3 rhs) {
        this.createByteBuffer(Double.BYTES * 3);
        this.set(rhs.getX(), rhs.getY(), rhs.getZ());
    }

    public boolean equals(Object other) {
        if (other == this) return true;
        if (other == null) return false;
        if (getClass() != other.getClass()) return false;
        DoubleVector3 real_other = (DoubleVector3)other;
        return (this.getX() == real_other.getX() &&
                this.getY() == real_other.getY() &&
                this.getZ() == real_other.getZ());
    }

    public void set(double x, double y, double z) {
        this.getByteBufferProxy().putDouble(0 * Double.BYTES, x);
        this.getByteBufferProxy().putDouble(1 * Double.BYTES, y);
        this.getByteBufferProxy().putDouble(2 * Double.BYTES, z);
    }

    public void setX(double value) {
        this.getByteBufferProxy().putDouble(0 * Double.BYTES, value);
    }


    public void setY(double value) {
        this.getByteBufferProxy().putDouble(1 * Double.BYTES, value);
    }


    public void setZ(double value) {
        this.getByteBufferProxy().putDouble(2 * Double.BYTES, value);
    }

    public double getX() {
        return this.getByteBufferProxy().getDouble(0 * Double.BYTES);
    }


    public double getY() {
        return this.getByteBufferProxy().getDouble(1 * Double.BYTES);
    }


    public double getZ() {
        return this.getByteBufferProxy().getDouble(2 * Double.BYTES);
    }

    public String toString() {
        String value = "(";
        for (int i = 0; i < 3; ++i)
        {
            if (i > 0)
                value = value + ", ";
            value = value + this.getByteBufferProxy().getDouble(i * Double.BYTES);
        }
        value = value + ")";
        return value;
    }
}


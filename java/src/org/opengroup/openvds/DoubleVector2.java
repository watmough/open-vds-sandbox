
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

public class DoubleVector2 extends ByteBufferBackedObject {

    public DoubleVector2() {
        this.createByteBuffer(Double.BYTES * 2);
        this.set((double)0, (double)0);
    }

    public DoubleVector2(double x, double y) {
        this.createByteBuffer(Double.BYTES * 2);
        this.set(x, y);
    }

    public DoubleVector2(java.nio.ByteBuffer bytebuffer, int byteoffset) {
        super(bytebuffer, byteoffset, Double.BYTES * 2);
    }

    public DoubleVector2(DoubleVector2 rhs) {
        this.createByteBuffer(Double.BYTES * 2);
        this.set(rhs.getX(), rhs.getY());
    }

    public boolean equals(Object other) {
        if (other == this) return true;
        if (other == null) return false;
        if (getClass() != other.getClass()) return false;
        DoubleVector2 real_other = (DoubleVector2)other;
        return (this.getX() == real_other.getX() &&
                this.getY() == real_other.getY());
    }

    public void set(double x, double y) {
        this.getByteBufferProxy().putDouble(0 * Double.BYTES, x);
        this.getByteBufferProxy().putDouble(1 * Double.BYTES, y);
    }

    public void setX(double value) {
        this.getByteBufferProxy().putDouble(0 * Double.BYTES, value);
    }


    public void setY(double value) {
        this.getByteBufferProxy().putDouble(1 * Double.BYTES, value);
    }

    public double getX() {
        return this.getByteBufferProxy().getDouble(0 * Double.BYTES);
    }


    public double getY() {
        return this.getByteBufferProxy().getDouble(1 * Double.BYTES);
    }

    public String toString() {
        String value = "(";
        for (int i = 0; i < 2; ++i)
        {
            if (i > 0)
                value = value + ", ";
            value = value + this.getByteBufferProxy().getDouble(i * Double.BYTES);
        }
        value = value + ")";
        return value;
    }
}


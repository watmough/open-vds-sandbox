
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

public class DoubleVector4 extends ByteBufferBackedObject {

    public DoubleVector4() {
        this.createByteBuffer(Double.BYTES * 4);
        this.set((double)0, (double)0, (double)0, (double)0);
    }

    public DoubleVector4(double x, double y, double z, double t) {
        this.createByteBuffer(Double.BYTES * 4);
        this.set(x, y, z, t);
    }

    public DoubleVector4(DoubleVector4 rhs) {
        this.createByteBuffer(Double.BYTES * 4);
        this.set(rhs.getX(), rhs.getY(), rhs.getZ(), rhs.getT());
    }

    public boolean equals(Object other) {
        if (other == this) return true;
        if (other == null) return false;
        if (getClass() != other.getClass()) return false;
        DoubleVector4 real_other = (DoubleVector4)other;
        return (this.getX() == real_other.getX() &&
                this.getY() == real_other.getY() &&
                this.getZ() == real_other.getZ() &&
                this.getT() == real_other.getT());
    }

    public void set(double x, double y, double z, double t) {
                this.getByteBufferProxy().putDouble(0 * Double.BYTES, x);
                this.getByteBufferProxy().putDouble(1 * Double.BYTES, y);
                this.getByteBufferProxy().putDouble(2 * Double.BYTES, z);
                this.getByteBufferProxy().putDouble(3 * Double.BYTES, t);
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


    public void setT(double value) {
        this.getByteBufferProxy().putDouble(3 * Double.BYTES, value);
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


    public double getT() {
        return this.getByteBufferProxy().getDouble(3 * Double.BYTES);
    }

    public String toString() {
        String value = "(";
        for (int i = 0; i < 4; ++i)
        {
            if (i > 0)
                value = value + ", ";
            value = value + this.getByteBufferProxy().getDouble(i * Double.BYTES);
        }
        value = value + ")";
        return value;
    }
}


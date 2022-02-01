
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

public class DoubleRange extends ByteBufferBackedObject {

    public DoubleRange() {
        this.createByteBuffer(Double.BYTES * 2 * 1);
    }

    public DoubleRange(double min, double max) {
        this.createByteBuffer(Double.BYTES * 2);
        this.set(min, max);
    }

    public DoubleRange(java.nio.ByteBuffer bytebuffer, int byteoffset) {
        super(bytebuffer, byteoffset, Double.BYTES * 2);
    }

    public DoubleRange(DoubleRange rhs) {
        this.createByteBuffer(Double.BYTES * 2);
        this.set(rhs.getMin(), rhs.getMax());
    }

    public boolean equals(Object other) {
        if (other == this) return true;
        if (other == null) return false;
        if (getClass() != other.getClass()) return false;
        DoubleRange real_other = (DoubleRange)other;
        return (this.getMin() == real_other.getMin() &&
                this.getMax() == real_other.getMax());
    }

    void put(ByteBufferProxy bytebufferproxy, int byteoffset) {
        bytebufferproxy.putDouble(0 * Double.BYTES + byteoffset, this.getMin());
        bytebufferproxy.putDouble(1 * Double.BYTES + byteoffset, this.getMax());
    }

    public void set(double min, double max) {
        this.getByteBufferProxy().putDouble(0 * Double.BYTES, min);
        this.getByteBufferProxy().putDouble(1 * Double.BYTES, max);
    }

    public void setMin(double value) {
        this.getByteBufferProxy().putDouble(0 * Double.BYTES, value);
    }


    public void setMax(double value) {
        this.getByteBufferProxy().putDouble(1 * Double.BYTES, value);
    }

    public double getMin() {
        return this.getByteBufferProxy().getDouble(0 * Double.BYTES);
    }


    public double getMax() {
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


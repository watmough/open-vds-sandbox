
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

public class FloatVector4 extends ByteBufferBackedObject {

    public FloatVector4() {
        this.createByteBuffer(Float.BYTES * 4);
        this.set((float)0, (float)0, (float)0, (float)0);
    }

    public FloatVector4(float x, float y, float z, float t) {
        this.createByteBuffer(Float.BYTES * 4);
        this.set(x, y, z, t);
    }

    public FloatVector4(FloatVector4 rhs) {
        this.createByteBuffer(Float.BYTES * 4);
        this.set(rhs.getX(), rhs.getY(), rhs.getZ(), rhs.getT());
    }

    public boolean equals(Object other) {
        if (other == this) return true;
        if (other == null) return false;
        if (getClass() != other.getClass()) return false;
        FloatVector4 real_other = (FloatVector4)other;
        return (this.getX() == real_other.getX() &&
                this.getY() == real_other.getY() &&
                this.getZ() == real_other.getZ() &&
                this.getT() == real_other.getT());
    }

    public void set(float x, float y, float z, float t) {
                this.getByteBufferProxy().putFloat(0 * Float.BYTES, x);
                this.getByteBufferProxy().putFloat(1 * Float.BYTES, y);
                this.getByteBufferProxy().putFloat(2 * Float.BYTES, z);
                this.getByteBufferProxy().putFloat(3 * Float.BYTES, t);
    }

    public void setX(float value) {
        this.getByteBufferProxy().putFloat(0 * Float.BYTES, value);
    }


    public void setY(float value) {
        this.getByteBufferProxy().putFloat(1 * Float.BYTES, value);
    }


    public void setZ(float value) {
        this.getByteBufferProxy().putFloat(2 * Float.BYTES, value);
    }


    public void setT(float value) {
        this.getByteBufferProxy().putFloat(3 * Float.BYTES, value);
    }

    public float getX() {
        return this.getByteBufferProxy().getFloat(0 * Float.BYTES);
    }


    public float getY() {
        return this.getByteBufferProxy().getFloat(1 * Float.BYTES);
    }


    public float getZ() {
        return this.getByteBufferProxy().getFloat(2 * Float.BYTES);
    }


    public float getT() {
        return this.getByteBufferProxy().getFloat(3 * Float.BYTES);
    }

    public String toString() {
        String value = "(";
        for (int i = 0; i < 4; ++i)
        {
            if (i > 0)
                value = value + ", ";
            value = value + this.getByteBufferProxy().getFloat(i * Float.BYTES);
        }
        value = value + ")";
        return value;
    }
}


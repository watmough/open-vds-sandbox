
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

public class FloatVector2 extends ByteBufferBackedObject {

    public FloatVector2() {
        this.createByteBuffer(Float.BYTES * 2);
        this.set((float)0, (float)0);
    }

    public FloatVector2(float x, float y) {
        this.createByteBuffer(Float.BYTES * 2);
        this.set(x, y);
    }

    public FloatVector2(FloatVector2 rhs) {
        this.createByteBuffer(Float.BYTES * 2);
        this.set(rhs.getX(), rhs.getY());
    }

    public boolean equals(Object other) {
        if (other == this) return true;
        if (other == null) return false;
        if (getClass() != other.getClass()) return false;
        FloatVector2 real_other = (FloatVector2)other;
        return (this.getX() == real_other.getX() &&
                this.getY() == real_other.getY());
    }

    public void set(float x, float y) {
                this.getByteBufferProxy().putFloat(0 * Float.BYTES, x);
                this.getByteBufferProxy().putFloat(1 * Float.BYTES, y);
    }

    public void setX(float value) {
        this.getByteBufferProxy().putFloat(0 * Float.BYTES, value);
    }


    public void setY(float value) {
        this.getByteBufferProxy().putFloat(1 * Float.BYTES, value);
    }

    public float getX() {
        return this.getByteBufferProxy().getFloat(0 * Float.BYTES);
    }


    public float getY() {
        return this.getByteBufferProxy().getFloat(1 * Float.BYTES);
    }

    public String toString() {
        String value = "(";
        for (int i = 0; i < 2; ++i)
        {
            if (i > 0)
                value = value + ", ";
            value = value + this.getByteBufferProxy().getFloat(i * Float.BYTES);
        }
        value = value + ")";
        return value;
    }
}


/*
 * Copyright 2022 The Open Group
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

public class NDPosArray extends ByteBufferBackedObject implements PODArray {

    private int size;

    public NDPosArray(int size) {
        this.createByteBuffer(6 * Float.BYTES * size);
        this.size = size;
        this.readOnly = false;
    }

    private boolean readOnly;

    public NDPosArray(ByteBuffer bytebuffer, int size, boolean readOnly) {
        this.setByteBuffer(bytebuffer, 0);
        this.size = size;
        this.readOnly = readOnly;
    }

    public ByteBuffer getByteBuffer() {
        return this.getBackingByteBuffer();
    }

    public int getSize() {
        return this.size;
    }

    public boolean isReadOnly() {
        return this.readOnly;
    }

    public void dispose() {
        this.setByteBuffer(null, 0);
    }

    public void set(int index, float pos0, float pos1, float pos2, float pos3, float pos4, float pos5) {
        if (this.readOnly) {
            throw new UnsupportedOperationException("Object is read-only");
        } else {
            this.getManagedBuffer().putFloat(0 * Float.BYTES + index * 6 * Float.BYTES, pos0);
            this.getManagedBuffer().putFloat(1 * Float.BYTES + index * 6 * Float.BYTES, pos1);
            this.getManagedBuffer().putFloat(2 * Float.BYTES + index * 6 * Float.BYTES, pos2);
            this.getManagedBuffer().putFloat(3 * Float.BYTES + index * 6 * Float.BYTES, pos3);
            this.getManagedBuffer().putFloat(4 * Float.BYTES + index * 6 * Float.BYTES, pos4);
            this.getManagedBuffer().putFloat(5 * Float.BYTES + index * 6 * Float.BYTES, pos5);
        }
    }

    public float getPos0(int index) {
        return this.getManagedBuffer().getFloat(0 * Float.BYTES + index * 6 * Float.BYTES);
    }

    public float getPos1(int index) {
        return this.getManagedBuffer().getFloat(1 * Float.BYTES + index * 6 * Float.BYTES);
    }

    public float getPos2(int index) {
        return this.getManagedBuffer().getFloat(2 * Float.BYTES + index * 6 * Float.BYTES);
    }

    public float getPos3(int index) {
        return this.getManagedBuffer().getFloat(3 * Float.BYTES + index * 6 * Float.BYTES);
    }

    public float getPos4(int index) {
        return this.getManagedBuffer().getFloat(4 * Float.BYTES + index * 6 * Float.BYTES);
    }

    public float getPos5(int index) {
        return this.getManagedBuffer().getFloat(5 * Float.BYTES + index * 6 * Float.BYTES);
    }

    public void setPos0(int index, float value) {
        if (this.readOnly) {
            throw new UnsupportedOperationException("Object is read-only");
        } else {
            this.getManagedBuffer().putFloat(0 * Float.BYTES + index * 6 * Float.BYTES, value);
        }
    }

    public void setPos1(int index, float value) {
        if (this.readOnly) {
            throw new UnsupportedOperationException("Object is read-only");
        } else {
            this.getManagedBuffer().putFloat(1 * Float.BYTES + index * 6 * Float.BYTES, value);
        }
    }

    public void setPos2(int index, float value) {
        if (this.readOnly) {
            throw new UnsupportedOperationException("Object is read-only");
        } else {
            this.getManagedBuffer().putFloat(2 * Float.BYTES + index * 6 * Float.BYTES, value);
        }
    }

    public void setPos3(int index, float value) {
        if (this.readOnly) {
            throw new UnsupportedOperationException("Object is read-only");
        } else {
            this.getManagedBuffer().putFloat(3 * Float.BYTES + index * 6 * Float.BYTES, value);
        }
    }

    public void setPos4(int index, float value) {
        if (this.readOnly) {
            throw new UnsupportedOperationException("Object is read-only");
        } else {
            this.getManagedBuffer().putFloat(4 * Float.BYTES + index * 6 * Float.BYTES, value);
        }
    }

    public void setPos5(int index, float value) {
        if (this.readOnly) {
            throw new UnsupportedOperationException("Object is read-only");
        } else {
            this.getManagedBuffer().putFloat(5 * Float.BYTES + index * 6 * Float.BYTES, value);
        }
    }

    public void set(int index, NDPos value) {
        this.set(index, value.getPos0(), value.getPos1(), value.getPos2(), value.getPos3(), value.getPos4(), value.getPos5());
    }

    public NDPos get(int index) {
        float pos0 = this.getPos0(index);
        float pos1 = this.getPos1(index);
        float pos2 = this.getPos2(index);
        float pos3 = this.getPos3(index);
        float pos4 = this.getPos4(index);
        float pos5 = this.getPos5(index);
        return new NDPos(pos0, pos1, pos2, pos3, pos4, pos5);
    }

    public String toString() {
        String value = "";
        for (int index = 0; index < this.size; ++index) {
            if (index > 0)
                value = value + ", ";
            value = value + "(";
            for (int i = 0; i < 6; ++i) {
                if (i > 0)
                    value = value + ", ";
                value = value + this.getManagedBuffer().getFloat(i * Float.BYTES + index * 6 * Float.BYTES);
            }
            value = value + ")";
        }
        return value;
    }
}

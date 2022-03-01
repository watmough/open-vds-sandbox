
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

public class VDSIJKGridDefinition extends IJKGridDefinition {

    public VDSIJKGridDefinition() {
        this.createByteBuffer(Double.BYTES * 3 * 4 + Integer.BYTES * 3);
    }

    public IntVector3 getDimensionMap() {
        return new IntVector3(this.getBackingByteBuffer(), this.getByteBufferOffset() + IJKGridDefinition.BYTES);
    }

    public void setDimensionMap(IntVector3 value) {
        value.put(this.getManagedBuffer(), this.getByteBufferOffset() + IJKGridDefinition.BYTES);
    }

    public String toString() {
        String value = "(" + super.toString() + ", DimensionMap(" + this.getDimensionMap().toString() + "))";
        return value;
    }

    public boolean equals(Object other) {
        if (other == this) return true;
        if (other == null) return false;
        if (getClass() != other.getClass()) return false;
        VDSIJKGridDefinition real_other = (VDSIJKGridDefinition)other;
        return super.equals(other) && this.getDimensionMap().equals(real_other.getDimensionMap());
    }
}

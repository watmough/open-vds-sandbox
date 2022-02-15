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

public abstract class ManagedBase {
	
	private long native_object;

    public ManagedBase(long native_object) {
        this.native_object = native_object;
    }
	
    long getNativeObject() {
        if (this.native_object == 0)
           throw new RuntimeException("Accessing disposed object");
        return this.native_object;
    }
	
	protected void onDisposing(long native_object, boolean isDisposing) {
	}

	private synchronized void dispose(boolean isDisposing) {

		if (this.native_object != 0) {
			long native_object = this.native_object;
			this.native_object = 0;
			onDisposing(native_object, isDisposing);
		}
    }

    public synchronized void dispose() {
        dispose(true);
    }

    @Override
    public void finalize() {
		dispose(false);
    }
	
	public boolean isDisposed() {
		return this.native_object == 0;
	}
}

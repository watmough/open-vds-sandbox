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
import java.util.logging.*;

public abstract class ManagedBase {

    static {
        try {
            System.loadLibrary("openvds-javacpp");
        } catch (Throwable e) {
            Logger.getLogger(ManagedBase.class.getName()).log(Level.SEVERE, "Failed to load JNI library.", e);
        }
    }

    public static <T> T requireNonNull(T obj, String message) {
        if (obj == null) {
            throw new IllegalArgumentException(message);
        }
        return obj;
    }

    public static class ObjectDisposedException extends RuntimeException {
        public ObjectDisposedException() {
            super("Accessing disposed object");
        }
        public ObjectDisposedException(String msg) {
            super("Accessing disposed object: " + msg);
        }
    }

    static void staticInit() {
    }

	private long native_object;

    private boolean is_disposed;

    public ManagedBase(long native_object) {
        this.native_object = native_object;
        this.is_disposed = false;
    }
	
    long getNativeObject() {
        if (this.is_disposed)
           throw new ObjectDisposedException();
        return this.native_object;
    }
	
	protected void onDisposing(long native_object, boolean isDisposing) {
	}

	private synchronized void dispose(boolean isDisposing) {
        this.is_disposed = true;
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
    protected void finalize() {
		dispose(false);
    }
	
	public boolean isDisposed() {
		return this.is_disposed;
	}

	public boolean isNull() { return this.native_object == 0; }
}

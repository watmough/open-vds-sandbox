package org.opengroup.openvds;

public class VdsError extends JniPointerWithoutDeletion {

    private static native long cpCreateErrorHandle();

    private static native void cpDeleteHandle(long handle);

    private static native void cpSetErrorCode(long handle, int code);

    private static native void cpSetErrorMessage(long handle, String message);

    private static native int cpGetErrorCode(long handle);

    private static native String cpGetErrorMessage(long handle);

    public VdsError() {
        super(cpCreateErrorHandle());
        setOwnHandle(true);
    }

    public VdsError(long handle) {
        super(handle);
    }

    // Called by JniPointer.release()
    @Override
    protected synchronized void deleteHandle() {
        cpDeleteHandle(_handle);
    }

    public int getErrorCode() {
        return cpGetErrorCode(_handle);
    }

    public void setErrorCode(int errorCode) {
        cpSetErrorCode(_handle, errorCode);
    }

    public String getMessage() {
        return cpGetErrorMessage(_handle);
    }

    public void setMessage(String message) {
        cpSetErrorMessage(_handle, message);
    }
}

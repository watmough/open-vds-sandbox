package org.opengroup.openvds;

/**
 * Simple class holding information on Upload/Download error
 */
public class ConnectionErrorInfo {

    private int errorCode = -1;
    private String errorMessage;
    private String objectID;

    public ConnectionErrorInfo() { }

    public ConnectionErrorInfo(int code, String msg, String id) {
        errorCode = code;
        errorMessage = msg;
        objectID = id;
    }

    public int getErrorCode() {
        return errorCode;
    }

    public void setErrorCode(int errorCode) {
        this.errorCode = errorCode;
    }

    public String getErrorMessage() {
        return errorMessage;
    }

    public void setErrorMessage(String errorMessage) {
        this.errorMessage = errorMessage;
    }

    public String getObjectID() {
        return objectID;
    }

    public void setObjectID(String objectID) {
        this.objectID = objectID;
    }
}

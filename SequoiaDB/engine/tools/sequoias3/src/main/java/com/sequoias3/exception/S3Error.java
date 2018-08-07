package com.sequoias3.exception;

public enum S3Error {

    UNKNOWN_ERROR(-1, "Unknown error"),

    // common error
    INVALID_ARGUMENT(-101, "Invalid argument"),

    MISSING_ARGUMENT(-102, "Missing argument"),

    // service error
    USER_CREATE_FAILED(-201, "user create failed"),

    // dao error
    DAO_GETCONN_ERROR(-301, "Get connection failed");

    private int errorCode;
    private String desc;

    S3Error(int errorCode, String desc) {
        this.errorCode = errorCode;
        this.desc = desc;
    }

    public int getErrorCode() {
        return errorCode;
    }

    public String getErrorDescription() {
        return desc;
    }

    public String getErrorType() {
        return name();
    }

    @Override
    public String toString() {
        return name() + "(" + this.errorCode + ")" + ":" + this.desc;
    }

    public static S3Error getS3Error(int errorCode) {
        for (S3Error value : S3Error.values()) {
            if (value.getErrorCode() == errorCode) {
                return value;
            }
        }

        return UNKNOWN_ERROR;
    }
}
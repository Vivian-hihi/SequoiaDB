package com.sequoias3.exception;

public enum S3Error {

    UNKNOWN_ERROR(-1, "Unknown error", "Unknown error"),

    // 400
    INVALID_ARGUMENT(-900, "InvalidArgument", "Invalid argument."),
    MISSING_ARGUMENT(-901, "MissingArgument", "Missing argument"),

    //403
    INVALID_ADMINISTRATOR(-701, "AccessDenied", "Not admin user."),

    //403
    INVALID_ACCESSKEYID(-702, "InvalidAccessKeyId", "Invalid accessKeyId."),
    SIGNATURE_NOT_MATCH(-703, "SignatureDoesNotMatch", "Signature does not match."),

    // service error
    USER_NOT_EXIST(-800, "NoSuchUser", "User not exist."),
    USER_CREATE_FAILED(-801, "AddUserFailed", "Create user failed."),
    USER_CREATE_NAME_INVALID(-802, "AddUserFailed", "The username is invalid."),
    USER_CREATE_ROLE_INVALID(-803, "AddUserFailed", "The role is invalid."),
    USER_CREATE_EXIST(-804, "AddUserFailed", "The username is exist."),

    USER_DELETE_FAILED(-810, "DelUserFailed", "Delete user failed."),
    USER_DELETE_LAST_ADMIN(-811, "DelUserFailed", "Init admin user cannot be delete."),

    USER_UPDATE_FAILED(-820, "UpdateUserFailed", "Update user failed."),
    USER_GET_FAILED(-830, "GetUserFailed", "Get user failed."),

    // dao error
    DAO_GETCONN_ERROR(-601, "GetDBConnectFail", "Get connection failed.");

    private int errIndex;
    private String code;
    private String message;

    S3Error(int errIndex, String code, String desc) {
        this.errIndex = errIndex;
        this.code = code;
        this.message = desc;
    }

    public int getErrIndex() {
        return errIndex;
    }

    public String getErrorMessage() {
        return message;
    }

    public String getCode() {
        return code;
    }

    public String getErrorType() {
        return name();
    }

    @Override
    public String toString() {
        return name() + "(" + this.code + ")" + ":" + this.message;
    }

    public static S3Error getS3Error(int errorIndex) {
        for (S3Error value : S3Error.values()) {
            if (value.getErrIndex() == errorIndex) {
                return value;
            }
        }

        return UNKNOWN_ERROR;
    }
}
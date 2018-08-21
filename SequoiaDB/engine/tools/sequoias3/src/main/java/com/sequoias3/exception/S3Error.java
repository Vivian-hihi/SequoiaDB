package com.sequoias3.exception;

public enum S3Error {

    UNKNOWN_ERROR(-1, "InternalError", "We encountered an internal error. Please try again."),

    // dao error
    DAO_GETCONN_ERROR(-401, "GetDBConnectFail", "Get connection failed."),
    DAO_DUPLICATE_KEY(-402, "DuplicateKey", "Duplicate key."),

    //bucket error
    BUCKET_CREATE_FAILED(-500, "CreateBucketFailed", "Create bucket failed."),
    BUCKET_DELETE_FAILED(-501, "DeleteBucketFailed", "Delete bucket failed."),
    BUCKET_GET_SERVICE_FAILED(-502, "GetServiceFailed", "Get service failed."),

    BUCKET_NOT_EXIST(-510, "NoSuchBucket", "The speciﬁed bucket does not exist."),
    BUCKET_INVALID_BUCKETNAME(-511, "InvalidBucketName", "The speciﬁed bucket Name is not valid."),
    BUCKET_ALREADY_EXIST(-512, "BucketAlreadyExists", "The requested bucket name is not available. The bucket namespace is shared by all users of the system. Please select a diﬀerent name and try again."),
    BUCKET_ALREADY_OWNEDYOU(-513, "BucketAlreadyOwnedByYou", "Your previous request to create the named bucket succeeded and you already own it."),
    BUCKET_NOT_EMPTY(-514, "BucketNotEmpty", "The bucket you tried to delete is not empty."),
    BUCKET_TOO_MANY_BUCKETS(-515, "TooManyBuckets", "You have attempted to create more buckets than allowed."),

    //object
    OBJECT_INVALID_KEY(-601, "InvalidKey","Invalid Key."),

    //authorization
    INVALID_ADMINISTRATOR(-701, "AccessDenied", "Not admin user."),
    INVALID_ACCESSKEYID(-702, "InvalidAccessKeyId", "Invalid accessKeyId."),
    SIGNATURE_NOT_MATCH(-703, "SignatureDoesNotMatch", "Signature does not match."),
    ACCESS_DENIED(-704, "AccessDenied", "Access Denied."),

    // User
    USER_CREATE_FAILED(-801, "AddUserFailed", "Create user failed."),
    USER_DELETE_FAILED(-810, "DelUserFailed", "Delete user failed."),
    USER_UPDATE_FAILED(-820, "UpdateUserFailed", "Update user failed."),
    USER_GET_FAILED(-830, "GetUserFailed", "Get user failed."),
    USER_NOT_EXIST(-800, "NoSuchUser", "User not exist."),
    USER_CREATE_NAME_INVALID(-802, "InvalidUserName", "The username is invalid."),
    USER_CREATE_ROLE_INVALID(-803, "InvalidRole", "The role is invalid."),
    USER_CREATE_EXIST(-804, "UserAlreadyExists", "The username is exist."),

    USER_DELETE_INIT_ADMIN(-811, "InitAdminCannotDelete", "Init admin user cannot be delete."),


    // 400
    INVALID_ARGUMENT(-900, "InvalidArgument", "Invalid argument.");

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
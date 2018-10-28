package com.sequoias3.exception;

public enum S3Error {

    UNKNOWN_ERROR(-1, "InternalError", "We encountered an internal error. Please try again."),

    // dao error
    DAO_GETCONN_ERROR(-401, "GetDBConnectFail", "Get connection failed."),
    DAO_DUPLICATE_KEY(-402, "DuplicateKey", "Duplicate key."),
    DAO_CS_NOT_EXIST(-403, "CSNotExist", "Collection space does not exist."),
    DAO_CL_NOT_EXIST(-404, "CLNotExist", "Collection does not exist."),
    DAO_INSERT_LOB_FAILED(-405, "InsertLobFailed", "Insert Lob Failed."),
    DAO_DB_ERROR(-406, "DBError", "DB error."),
    DAO_LOB_FNE(407, "LobIsNotFound", "Lob is not found."),
    DAO_TRANSACTION_BEGIN_ERROR(-408, "BeginTransactionFailed", "Begin transaction failed."),

    //bucket error
    BUCKET_CREATE_FAILED(-500, "CreateBucketFailed", "Create bucket failed."),
    BUCKET_DELETE_FAILED(-501, "DeleteBucketFailed", "Delete bucket failed."),
    BUCKET_GET_SERVICE_FAILED(-502, "GetServiceFailed", "Get service failed."),
    BUCKET_VERSIONING_SET_FAILED(-503, "PutBucketVersioningFailed", "Put bucket versioning failed."),
    BUCKET_VERSIONING_GET_FAILED(-504, "GetBucketVersioningFailed", "Get bucket versioning failed."),

    BUCKET_NOT_EXIST(-510, "NoSuchBucket", "The speciﬁed bucket does not exist."),
    BUCKET_INVALID_BUCKETNAME(-511, "InvalidBucketName", "The speciﬁed bucket Name is not valid."),
    BUCKET_ALREADY_EXIST(-512, "BucketAlreadyExists", "The requested bucket name is not available. The bucket namespace is shared by all users of the system. Please select a different name and try again."),
    BUCKET_ALREADY_OWNED_BY_YOU(-513, "BucketAlreadyOwnedByYou", "Your previous request to create the named bucket succeeded and you already own it."),
    BUCKET_NOT_EMPTY(-514, "BucketNotEmpty", "The bucket you tried to delete is not empty."),
    BUCKET_TOO_MANY_BUCKETS(-515, "TooManyBuckets", "You have attempted to create more buckets than allowed."),
    BUCKET_INVALID_VERSIONING_STATUS(-516, "InvalidVersioningStatus", "The versioning status is invalid."),

    //object
    OBJECT_WRITE_fAILED(-600, "WriteObjectFailed", "Put object failed."),
    OBJECT_PUT_fAILED(-601, "PutObjectFailed", "Put object failed."),
    OBJECT_GET_FAILED(-602, "GetObjectFailed", "Get object failed"),
    OBJECT_DELETE_FAILED(-603, "DeleteObjectFailed", "Delete object failed."),
    OBJECT_LIST_FAILED(-604, "ListObjectsFailed", "List objects failed."),
    OBJECT_LIST_VERSIONS_FAILED(-605, "ListVersionsFailed", "List versions failed."),

    OBJECT_INVALID_KEY(-611, "InvalidKey", "Invalid Key."),
    OBJECT_KEY_TOO_LONG(-612, "KeyTooLongError", "Your key is too long."),
    OBJECT_METADATA_TOO_LARGE(-613, "MetadataTooLarge", "Your metadata headers exceed the maximum allowed metadata size."),
    OBJECT_NO_SUCH_KEY(-614, "NoSuchKey", "The specified key does not exist."),
    OBJECT_BAD_DIGEST(-615, "BadDigest", "The Content-MD5 you speciﬁed did not match what we received."),
    OBJECT_INVALID_ENCODING_TYPE(-616, "InvalidArgument", "Invalid Encoding Method specified in Request"),
    OBJECT_INVALID_TOKEN(-617, "InvalidArgument", "The continuation token provided is incorrect."),
    OBJECT_IS_IN_USE(-618, "ObjectIsInUse", "The object is in use."),
    OBJECT_RANGE_INVALID(-619, "RangeNotSatisfiable", "Requested range not satisfiable."),
    OBJECT_INVALID_DIGEST(-620, "InvalidDigest", " The Content-MD5 you speciﬁed is not valid."),
    OBJECT_NO_SUCH_VERSION(-621, "NoSuchVersion", "The specified version does not exist."),

    OBJECT_IF_MODIFIED_SINCE_FAILED(-621, "NotModiﬁed", "If-Modified-Since not match"),
    OBJECT_IF_UNMODIFIED_SINCE_FAILED(-621, "PreconditionFailed ", "If-Unmodified-Since not match"),
    OBJECT_IF_MATCH_FAILED(-621, "PreconditionFailed", "If-Match not match"),
    OBJECT_IF_NONE_MATCH_FAILED(-621, "NotModiﬁed", "If-None-Match not match"),
    OBJECT_INVALID_TIME(-622, "InvalidArgument", "Time is invalid"),

    //authorization
    INVALID_ADMINISTRATOR(-701, "AccessDenied", "Not admin user."),
    INVALID_ACCESSKEYID(-702, "InvalidAccessKeyId", "Invalid accessKeyId."),
    SIGNATURE_NOT_MATCH(-703, "SignatureDoesNotMatch", "Signature does not match."),
    ACCESS_DENIED(-704, "AccessDenied", "Access Denied."),
    NO_CREDENTIALS(-705, "CredentialsNotSupported", "This request does not support credentials."),

    // User
    USER_NOT_EXIST(-800, "NoSuchUser", "User not exist."),
    USER_CREATE_FAILED(-801, "AddUserFailed", "Create user failed."),
    USER_CREATE_NAME_INVALID(-802, "InvalidUserName", "The username is invalid."),
    USER_CREATE_ROLE_INVALID(-803, "InvalidRole", "The role is invalid."),
    USER_CREATE_EXIST(-804, "UserAlreadyExists", "The username is exist."),
    USER_DELETE_FAILED(-810, "DelUserFailed", "Delete user failed."),
    USER_DELETE_INIT_ADMIN(-811, "InitAdminCannotDelete", "Init admin user cannot be delete."),
    USER_DELETE_CLEAN_FAILED(-812, "CleanResourceFailed", "Clean resource failed, please try again."),
    USER_DELETE_RELEASE_RESOURCE(-813, "BucketMustRelease", "Please delete your bucket before delete user, or delete user force."),
    USER_UPDATE_FAILED(-820, "UpdateUserFailed", "Update user failed."),
    USER_GET_FAILED(-830, "GetUserFailed", "Get user failed."),

    INVALID_ARGUMENT(-900, "InvalidArgument", "Invalid argument."),
    METHOD_NOT_ALLOWED(-901, "MethodNotAllowed", "The speciﬁed method is not allowed against this resource.");

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
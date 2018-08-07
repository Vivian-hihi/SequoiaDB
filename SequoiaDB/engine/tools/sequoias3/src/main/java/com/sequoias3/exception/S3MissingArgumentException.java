package com.sequoias3.exception;

public class S3MissingArgumentException extends S3ServerException {
    private static final long serialVersionUID = -5238230425202542393L;

    public S3MissingArgumentException(String message) {
        super(S3Error.MISSING_ARGUMENT, message);
    }

    public S3MissingArgumentException(String message, Throwable e) {
        super(S3Error.MISSING_ARGUMENT, message, e);
    }
}
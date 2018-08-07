package com.sequoias3.exception;

public class S3InvalidArgumentException extends S3ServerException {
    private static final long serialVersionUID = -5238230425202542393L;

    public S3InvalidArgumentException(String message) {
        super(S3Error.INVALID_ARGUMENT, message);
    }

    public S3InvalidArgumentException(String message, Throwable e) {
        super(S3Error.INVALID_ARGUMENT, message, e);
    }
}

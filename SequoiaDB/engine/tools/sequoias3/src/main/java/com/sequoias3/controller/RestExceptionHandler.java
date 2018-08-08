package com.sequoias3.controller;

import com.sequoias3.core.Error;
import com.sequoias3.exception.S3ServerException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.ControllerAdvice;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.ResponseBody;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

@ControllerAdvice
public class RestExceptionHandler {
    private static final Logger logger = LoggerFactory.getLogger(RestExceptionHandler.class);
    //TODO: write sequoiadbs3's reponse
    private static final String ERROR_ATTRIBUTE = "X-S3-ERROR";

    @ExceptionHandler(S3ServerException.class)
    @ResponseBody
    public ResponseEntity s3ExceptionHandler(S3ServerException e, HttpServletRequest request,
                                             HttpServletResponse response) {
        String msg = String.format("request=%s, errcode=%s, message=%s", request.getRequestURI(), e.getError()
                .getCode(), e.getMessage());
        logger.error(msg, e);

        HttpStatus status;
        switch (e.getError()) {
            case INVALID_ARGUMENT:
            case MISSING_ARGUMENT:
            case USER_CREATE_NAME_INVALID:
            case USER_CREATE_ROLE_INVALID:
                status = HttpStatus.BAD_REQUEST;
                break;
            case INVALID_ACCESSKEYID:
            case INVALID_ADMINISTRATOR:
            case USER_DELETE_LAST_ADMIN:
                status = HttpStatus.FORBIDDEN;
                break;
            case USER_NOT_EXIST:
                status = HttpStatus.NOT_FOUND;
                break;
            case USER_CREATE_EXIST:
                status = HttpStatus.CONFLICT;
                break;
            default:
                status = HttpStatus.INTERNAL_SERVER_ERROR;
        }

        Error exceptionBody = new Error(e, request.getRequestURI());
        return ResponseEntity.status(status).body(exceptionBody);
    }

    @ExceptionHandler(Exception.class)
    public ResponseEntity unexpectedExceptionHandler(Exception e, HttpServletRequest request,
                                                     HttpServletResponse response) throws Exception {
        String msg = String.format("request=%s", request.getRequestURI());
        logger.error(msg, e);

        HttpStatus status = HttpStatus.INTERNAL_SERVER_ERROR;

        Error exceptionBody = new Error(e, request.getRequestURI());
        if ("HEAD".equalsIgnoreCase(request.getMethod())) {
            String error = exceptionBody.toString();
            response.setHeader(ERROR_ATTRIBUTE, error);
            return ResponseEntity.status(status).build();
        } else {
            return ResponseEntity.status(status).body(exceptionBody);
        }
    }
}
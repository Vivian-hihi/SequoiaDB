package com.sequoias3.controller;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.ControllerAdvice;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.ResponseBody;

import com.sequoias3.exception.S3ServerException;

@ControllerAdvice
public class RestExceptionHandler {
    private static final Logger logger = LoggerFactory.getLogger(RestExceptionHandler.class);
    //TODO: write sequoiadbs3's reponse
    private static final String ERROR_ATTRIBUTE = "X-SCM-ERROR";

    private static class ExceptionBody {

        private long timestamp;
        private int status;
        private String error;
        private String exception;
        private String message;
        private String path;

        ExceptionBody(S3ServerException e, String path) {
            this.timestamp = System.currentTimeMillis();
            this.status = e.getError().getErrorCode();
            this.error = e.getError().getErrorDescription();
            this.exception = e.getClass().getName();
            this.message = e.getMessage();
            this.path = path;
        }

        ExceptionBody(Exception e, String path) {
            this.timestamp = System.currentTimeMillis();
            this.status = HttpStatus.INTERNAL_SERVER_ERROR.value();
            this.error = "INTERNAL_SERVER_ERROR";
            this.exception = e.getClass().getName();
            this.message = e.getMessage();
            this.path = path;
        }

        public long getTimestamp() {
            return timestamp;
        }

        public int getStatus() {
            return status;
        }

        public String getError() {
            return error;
        }

        public String getException() {
            return exception;
        }

        public String getMessage() {
            return message;
        }

        public String getPath() {
            return path;
        }

        @Override
        public String toString() {
            return String
                    .format("{\"timestamp\":%d,\"status\":%d,\"error\":\"%s\",\"exception\":\"%s\",\"message\":\"%s\",\"path\":\"%s\"}",
                            timestamp, status, error, exception, message, path);
        }
    }

    @ExceptionHandler(S3ServerException.class)
    @ResponseBody
    public ResponseEntity s3ExceptionHandler(S3ServerException e, HttpServletRequest request,
            HttpServletResponse response) {
        String msg = String.format("request=%s, errcode=%d", request.getRequestURI(), e.getError()
                .getErrorCode());
        logger.error(msg, e);

        HttpStatus status;
        switch (e.getError()) {
            default:
                status = HttpStatus.INTERNAL_SERVER_ERROR;
        }

        ExceptionBody exceptionBody = new ExceptionBody(e, request.getRequestURI());
        if ("HEAD".equalsIgnoreCase(request.getMethod())) {
            String error = exceptionBody.toString();
            response.setHeader(ERROR_ATTRIBUTE, error);
            return ResponseEntity.status(status).build();
        }
        else {
            return ResponseEntity.status(status).body(exceptionBody);
        }
    }

    @ExceptionHandler(Exception.class)
    public ResponseEntity unexpectedExceptionHandler(Exception e, HttpServletRequest request,
            HttpServletResponse response) throws Exception {
        String msg = String.format("request=%s", request.getRequestURI());
        logger.error(msg, e);

        HttpStatus status = HttpStatus.INTERNAL_SERVER_ERROR;

        ExceptionBody exceptionBody = new ExceptionBody(e, request.getRequestURI());
        if ("HEAD".equalsIgnoreCase(request.getMethod())) {
            String error = exceptionBody.toString();
            response.setHeader(ERROR_ATTRIBUTE, error);
            return ResponseEntity.status(status).build();
        }
        else {
            return ResponseEntity.status(status).body(exceptionBody);
        }
    }
}
package com.sequoias3.controller;

import com.sequoias3.exception.S3Error;
import com.sequoias3.service.AuthorizationService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;
import org.springframework.http.MediaType;

import com.sequoias3.common.RestParamDefine;
import com.sequoias3.exception.S3ServerException;
import com.sequoias3.service.UserService;

import javax.servlet.http.HttpServletRequest;

@RestController
public class UserController {
    private static final Logger logger = LoggerFactory.getLogger(UserController.class);

    @Autowired
    UserService userService;

    @Autowired
    AuthorizationService authorizationService;

    @PostMapping(value="/users/{username}",produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity createUser(@RequestParam(value=RestParamDefine.ROLE,required = false) String role,
                                  @RequestHeader(RestParamDefine.AUTHORIZATION) String authorization,
                                  HttpServletRequest httpServletRequest)throws S3ServerException{
        String adminName = authorizationService.getNameByAuthorization(authorization);

        if (authorizationService.isAdminUser(adminName) != true) {
            logger.info("Not an admin user:adminName={}", adminName);
            throw new S3ServerException(S3Error.INVALID_ADMINISTRATOR, "not an admin user");
        }

        int index = httpServletRequest.getRequestURI().indexOf("/",1);
        String userName = httpServletRequest.getRequestURI().substring(index+1);

        logger.info("creating user:adminName={},user={},role={}", adminName, userName, role);
        return ResponseEntity.ok()
                .body(userService.createUser(userName, role));
    }

    @PutMapping(value="/users/{username}", produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity updateUser(@RequestHeader(RestParamDefine.AUTHORIZATION) String authorization,
                                 HttpServletRequest httpServletRequest)
            throws S3ServerException {
        String adminName = authorizationService.getNameByAuthorization(authorization);

        if (authorizationService.isAdminUser(adminName) != true) {
            throw new S3ServerException(S3Error.INVALID_ADMINISTRATOR, "not an admin user");
        }
        int index = httpServletRequest.getRequestURI().indexOf("/",1);
        String userName = httpServletRequest.getRequestURI().substring(index+1);

        logger.info("creating user:admin={},user={}", adminName, userName);
        return ResponseEntity.ok()
                .body(userService.updateUser(userName));
    }

    @GetMapping(value="/users/{username}", produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity getUser(@RequestHeader(RestParamDefine.AUTHORIZATION) String authorization,
                                  HttpServletRequest httpServletRequest)
            throws S3ServerException{
        String adminName = authorizationService.getNameByAuthorization(authorization);

        if (authorizationService.isAdminUser(adminName) != true) {
            throw new S3ServerException(S3Error.INVALID_ADMINISTRATOR, "not an admin user");
        }

        int index = httpServletRequest.getRequestURI().indexOf("/",1);
        String userName = httpServletRequest.getRequestURI().substring(index+1);

        logger.info("get user:admin={},user={}", adminName, userName);
        return ResponseEntity.ok()
                .header("x-amz-id-2"," JuKZqmXuiwFeDQxhD7M8KtsKobSzWA1QEjLbTMTagkKdBX2z7Il1jGhDeJ3j6s80 ")
                .header("x-amz-requestid","32FE2CEB32F5EE25")
                .body(userService.getUser(userName));
    }

    @DeleteMapping(value="/users/{username}", produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity deleteUser(@RequestHeader(RestParamDefine.AUTHORIZATION) String authorization,
                           HttpServletRequest httpServletRequest) throws S3ServerException{
        String adminName = authorizationService.getNameByAuthorization(authorization);

        if (authorizationService.isAdminUser(adminName) != true) {
            throw new S3ServerException(S3Error.INVALID_ADMINISTRATOR, "not an admin user");
        }

        int index = httpServletRequest.getRequestURI().indexOf("/",1);
        String userName = httpServletRequest.getRequestURI().substring(index+1);

        logger.info("delete user:admin={},user={}", adminName, userName);
        userService.deleteUser(userName);
        return ResponseEntity.ok().build();
    }
}

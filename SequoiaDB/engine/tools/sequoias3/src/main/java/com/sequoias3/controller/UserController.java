package com.sequoias3.controller;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestHeader;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import com.sequoias3.common.RestParamDefine;
import com.sequoias3.core.UserAuthKey;
import com.sequoias3.exception.S3InvalidArgumentException;
import com.sequoias3.exception.S3ServerException;
import com.sequoias3.service.UserService;

@RestController
public class UserController {
    private static final Logger logger = LoggerFactory.getLogger(UserController.class);

    @Autowired
    UserService userService;

    @PostMapping("/users/{username}")
    public UserAuthKey createUser(@RequestParam(RestParamDefine.ROLE) String role,
            @PathVariable(RestParamDefine.USER_NAME) String userName,
            @RequestHeader(RestParamDefine.AUTHORIZATION) String authorization)
            throws S3ServerException {
        String[] result = authorization.split("/");
        if (result == null || result.length != 2) {
            // TODO: remove authorization's password?
            throw new S3InvalidArgumentException("authorization is invalid:authorization="
                    + authorization);
        }

        logger.info("creating user:admin={},user={},role={}", result[0], userName, role);
        return userService.createUser(result[0], result[1], userName, role);
    }
}

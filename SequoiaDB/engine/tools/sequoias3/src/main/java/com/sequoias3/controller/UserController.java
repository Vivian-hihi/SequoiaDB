package com.sequoias3.controller;

import com.sequoias3.common.RestParamDefine;
import com.sequoias3.core.User;
import com.sequoias3.exception.S3Error;
import com.sequoias3.exception.S3ServerException;
import com.sequoias3.service.UserService;
import com.sequoias3.utils.RestUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

@RestController
public class UserController {
    private static final Logger logger = LoggerFactory.getLogger(UserController.class);

    @Autowired
    UserService userService;

    @Autowired
    RestUtils restUtils;

    @PostMapping(value = "/users/{username}", produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity createUser(@RequestParam(value = RestParamDefine.ROLE, required = false) String role,
                                     @RequestHeader(RestParamDefine.AUTHORIZATION) String authorization,
                                     @PathVariable("username") String username)
            throws S3ServerException {
        User adminUser = restUtils.getOperatorByAuthorization(authorization);

        if (!adminUser.getRole().equals(RestParamDefine.ROLE_ADMIN)) {
            throw new S3ServerException(S3Error.INVALID_ADMINISTRATOR, "not an admin user.adminUser=" + adminUser.getUserName() + ",role=" + adminUser.getRole());
        }

        logger.info("creating user:adminName={},user={},role={}", adminUser.getUserName(), username, role);
        return ResponseEntity.ok()
                .header("x-amz-id-2", " JuKZqmXuiwFeDQxhD7M8KtsKobSzWA1QEjLbTMTagkKdBX2z7Il1jGhDeJ3j6s80 ")
                .header("x-amz-requestid", "32FE2CEB32F5EE25")
                .body(userService.createUser(username, role));
    }

    @PutMapping(value = "/users/{username}", produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity updateUser(@RequestHeader(RestParamDefine.AUTHORIZATION) String authorization,
                                     @PathVariable("username") String username)
            throws S3ServerException {
        User adminUser = restUtils.getOperatorByAuthorization(authorization);

        if (!adminUser.getRole().equals(RestParamDefine.ROLE_ADMIN)) {
            throw new S3ServerException(S3Error.INVALID_ADMINISTRATOR, "not an admin user.adminUser=" + adminUser.getUserName() + ",role=" + adminUser.getRole());
        }

        logger.info("creating user:admin={},user={}", adminUser.getUserName(), username);
        return ResponseEntity.ok()
                .header("x-amz-id-2", " JuKZqmXuiwFeDQxhD7M8KtsKobSzWA1QEjLbTMTagkKdBX2z7Il1jGhDeJ3j6s80 ")
                .header("x-amz-requestid", "32FE2CEB32F5EE25")
                .body(userService.updateUser(username));
    }

    @GetMapping(value = "/users/{username}", produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity getUser(@RequestHeader(RestParamDefine.AUTHORIZATION) String authorization,
                                  @PathVariable("username") String username)
            throws S3ServerException {
        User adminUser = restUtils.getOperatorByAuthorization(authorization);

        if (!adminUser.getRole().equals(RestParamDefine.ROLE_ADMIN)) {
            throw new S3ServerException(S3Error.INVALID_ADMINISTRATOR, "not an admin user.adminUser=" + adminUser.getUserName() + ",role=" + adminUser.getRole());
        }

        logger.info("get user:admin={},user={}", adminUser.getUserName(), username);
        return ResponseEntity.ok()
                .header("x-amz-id-2", " JuKZqmXuiwFeDQxhD7M8KtsKobSzWA1QEjLbTMTagkKdBX2z7Il1jGhDeJ3j6s80 ")
                .header("x-amz-requestid", "32FE2CEB32F5EE25")
                .body(userService.getUser(username));
    }

    @DeleteMapping(value = "/users/{username}", produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity deleteUser(@RequestHeader(RestParamDefine.AUTHORIZATION) String authorization,
                                     @PathVariable("username") String username)
            throws S3ServerException {
        User adminUser = restUtils.getOperatorByAuthorization(authorization);

        if (!adminUser.getRole().equals(RestParamDefine.ROLE_ADMIN)) {
            throw new S3ServerException(S3Error.INVALID_ADMINISTRATOR, "not an admin user.adminUser=" + adminUser.getUserName() + ",role=" + adminUser.getRole());
        }

        logger.info("delete user:admin={},user={}", adminUser.getUserName(), username);
        userService.deleteUser(username);
        return ResponseEntity.ok().build();
    }
}

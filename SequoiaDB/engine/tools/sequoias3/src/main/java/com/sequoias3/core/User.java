package com.sequoias3.core;

public class User {
    public static final String JSON_KEY_USERNAME = "UserName";
    public static final String JSON_KEY_USERID = "UserID";
    public static final String JSON_KEY_PASSWORD = "Password";
    public static final String JSON_KEY_ROLE = "Role";
    public static final String JSON_KEY_ACCESS_KEY_ID = "AccessKeyID";
    public static final String JSON_KEY_SECRET_ACCESS_KEY = "SecretAccessKey";
    private String userName;
    private String userId;
    private String password;
    private String role;
    private String accessKeyID;
    private String secretAccessKey;

    public String getUserName() {
        return userName;
    }

    public void setUserName(String username) {
        this.userName = username;
    }

    public String getUserId() {
        return userId;
    }

    public void setUserId(String userId) {
        this.userId = userId;
    }

    public String getPassword() {
        return password;
    }

    public void setPassword(String password) {
        this.password = password;
    }

    public String getRole() {
        return role;
    }

    public void setRole(String role) {
        this.role = role;
    }

    public String getAccessKeyID() {
        return accessKeyID;
    }

    public void setAccessKeyID(String accessKeyID) {
        this.accessKeyID = accessKeyID;
    }

    public String getSecretAccessKey() {
        return secretAccessKey;
    }

    public void setSecretAccessKey(String secretAccessKey) {
        this.secretAccessKey = secretAccessKey;
    }
}

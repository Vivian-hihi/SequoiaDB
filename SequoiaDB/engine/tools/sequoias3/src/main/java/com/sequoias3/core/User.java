package com.sequoias3.core;

public class User {
    public static final String JSON_KEY_USERNAME = "Name";
    public static final String JSON_KEY_USERID = "ID";
    public static final String JSON_KEY_ROLE = "Role";
    public static final String JSON_KEY_ACCESS_KEY_ID = "AccessKeyID";
    public static final String JSON_KEY_SECRET_ACCESS_KEY = "SecretAccessKey";
    private String userName;
    private int userId;
    private String role;
    private String accessKeyID;
    private String secretAccessKey;

    public String getUserName() { return userName; }

    public void setUserName(String username) { this.userName = username; }

    public int getUserId() { return userId; }

    public void setUserId(int userId) { this.userId = userId; }

    public String getRole() { return role; }

    public void setRole(String role) { this.role = role; }

    public String getAccessKeyID() { return accessKeyID; }

    public void setAccessKeyID(String accessKeyID) { this.accessKeyID = accessKeyID; }

    public String getSecretAccessKey() { return secretAccessKey; }

    public void setSecretAccessKey(String secretAccessKey) { this.secretAccessKey = secretAccessKey; }
}

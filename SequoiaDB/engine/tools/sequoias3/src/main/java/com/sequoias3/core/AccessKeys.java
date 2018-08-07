package com.sequoias3.core;


public class AccessKeys {
    public static final String JSON_ACCESSKEY_ID = "AccessKeyID";
    public static final String JSON_SECRET_ASSCESS_KEY = "SecretAccessKey";

    private String accessKeyID;
    private String secretAccessKey;

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

package com.sequoias3.core.serial;

import java.io.IOException;

import com.fasterxml.jackson.core.JsonGenerator;
import com.fasterxml.jackson.databind.SerializerProvider;
import com.fasterxml.jackson.databind.ser.std.StdSerializer;
import com.sequoias3.core.UserAuthKey;

public class UserAuthKeySerializer extends StdSerializer<UserAuthKey> {

    private static final long serialVersionUID = 5087465916766420390L;

    public UserAuthKeySerializer() {
        super(UserAuthKey.class);
    }

    @Override
    public void serialize(UserAuthKey user, JsonGenerator gen, SerializerProvider provider)
            throws IOException {
        gen.writeStartObject();
        gen.writeStringField(UserAuthKey.JSON_ACCESSKEY_ID, user.getAccessKeyID());
        gen.writeStringField(UserAuthKey.JSON_ACCESSKEYS, user.getAccessKeys());
        gen.writeStringField(UserAuthKey.JSON_SECRET_ASSCESS_KEY, user.getSecretAccessKey());
        gen.writeEndObject();
    }
}

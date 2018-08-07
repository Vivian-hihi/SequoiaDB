package com.sequoias3.core.serial;

import java.io.IOException;

import com.fasterxml.jackson.core.JsonGenerator;
import com.fasterxml.jackson.databind.SerializerProvider;
import com.fasterxml.jackson.databind.ser.std.StdSerializer;
import com.sequoias3.core.AccessKeys;

public class UserAuthKeySerializer extends StdSerializer<AccessKeys> {

    private static final long serialVersionUID = 5087465916766420390L;

    public UserAuthKeySerializer() {
        super(AccessKeys.class);
    }

    @Override
    public void serialize(AccessKeys user, JsonGenerator gen, SerializerProvider provider)
            throws IOException {
        gen.writeStartObject();
        gen.writeStringField(AccessKeys.JSON_ACCESSKEY_ID, user.getAccessKeyID());
        gen.writeStringField(AccessKeys.JSON_SECRET_ASSCESS_KEY, user.getSecretAccessKey());
        gen.writeEndObject();
    }
}

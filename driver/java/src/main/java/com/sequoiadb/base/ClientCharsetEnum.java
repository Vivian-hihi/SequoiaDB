/*
 * Copyright 2023 SequoiaDB Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the License for the specific language governing permissions and limitations under
 * the License.
 */

package com.sequoiadb.base;

/**
 * The client charset type, support UTF-8 and GB18030
 */
public enum ClientCharsetEnum {
    /**
     * UTF-8 charset
     */
    UTF_8("UTF-8"),

    /**
     * GB18030 charset
     */
    GB18030("GB18030");

    private final String name;

    ClientCharsetEnum(final String name) {
        this.name = name;
    }

    /**
     * Gets the charset name.
     *
     * @return the charset name
     */
    public String getName() {
        return name;
    }
}

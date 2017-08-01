/*
 * Copyright 2017 SequoiaDB Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

package com.sequoiadb.base;

import org.bson.BSONObject;

import java.util.HashMap;
import java.util.Map;

/**
 * @class DBQuery
 * @brief Database operation rules.
 */
public class DBQuery {
    private BSONObject matcher;
    private BSONObject selector;
    private BSONObject orderBy;
    private BSONObject hint;
    private BSONObject modifier;
    private Long skipRowsCount;
    private Long returnRowsCount;
    private int flag;

    /**
     * @memberof FLG_QUERY_STRINGOUT 0x00000001
     * @brief Normally, query return bson object,
     * when this flag is added, query return binary data stream
     */
    public static final int FLG_QUERY_STRINGOUT = 0x00000001;

    /**
     * @memberof FLG_QUERY_FORCE_HINT 0x00000080
     * @brief Force to use specified hint to query,
     * if database have no index assigned by the hint, fail to query.
     */
    public static final int FLG_QUERY_FORCE_HINT = 0x00000080;

    /**
     * @memberof FLG_QUERY_PARALLED 0x00000100
     * @brief Enable parallel sub query, each sub query will finish scanning diffent part of the data.
     */
    public static final int FLG_QUERY_PARALLED = 0x00000100;

    /**
     * @memberof FLG_QUERY_WITH_RETURNDATA 0x00000200
     * @brief In general, query won't return data until cursor gets from database, when add this flag, return data in query response, it will be more high-performance.
     */
    public static final int FLG_QUERY_WITH_RETURNDATA = 0x00000200;

    /**
     * @memberof FLG_QUERY_EXPLAIN 0x00000400
     * @brief Query explain.
     */
    static final int FLG_QUERY_EXPLAIN = 0x00000400;

    /**
     * @memberof FLG_QUERY_MODIFY 0x00001000
     * @brief Query and modify.
     */
    static final int FLG_QUERY_MODIFY = 0x00001000;

    /**
     * @memberof FLG_QUERY_PREPARE_MORE 0x00004000
     * @brief Enable prepare more data when query.
     */
    public static final int FLG_QUERY_PREPARE_MORE = 0x00004000;

    /** @memberof FLG_QUERY_KEEP_SHARDINGKEY_IN_UPDATE 0x00008000
      * @brief The sharding key in update rule is not filtered,
      *        when executing queryAndUpdate.
      */
    public static final int FLG_QUERY_KEEP_SHARDINGKEY_IN_UPDATE = 0x00008000;

    final static Map<Integer, Integer> flagsMap = new HashMap<Integer, Integer>();

    static {
        flagsMap.put(FLG_QUERY_STRINGOUT, FLG_QUERY_STRINGOUT);
        flagsMap.put(FLG_QUERY_FORCE_HINT, FLG_QUERY_FORCE_HINT);
        flagsMap.put(FLG_QUERY_PARALLED, FLG_QUERY_PARALLED);
        flagsMap.put(FLG_QUERY_WITH_RETURNDATA, FLG_QUERY_WITH_RETURNDATA);
        flagsMap.put(FLG_QUERY_PREPARE_MORE, FLG_QUERY_PREPARE_MORE);
        flagsMap.put(FLG_QUERY_KEEP_SHARDINGKEY_IN_UPDATE, FLG_QUERY_KEEP_SHARDINGKEY_IN_UPDATE);
    }

    public DBQuery() {
        matcher = null;
        selector = null;
        orderBy = null;
        hint = null;
        modifier = null;
        skipRowsCount = 0L;
        returnRowsCount = -1L;
        flag = 0;
    }

    /**
     * @return The modified rule BSONObject
     * @fn BSONObject getModifier()
     * @brief Get modified rule
     */
    public BSONObject getModifier() {
        return modifier;
    }

    /**
     * @param Modifier The modified rule BSONObject
     * @fn void setModifier(BSONObject modifier)
     * @brief Set modified rule
     */
    public void setModifier(BSONObject modifier) {
        this.modifier = modifier;
    }

    /**
     * @return The selective rule BSONObject
     * @fn BSONObject getSelector()
     * @brief Get selective rule
     */
    public BSONObject getSelector() {
        return selector;
    }

    /**
     * @param Selector The selective rule BSONObject
     * @fn void setSelector(BSONObject selector)
     * @brief Set selective rule
     */
    public void setSelector(BSONObject selector) {
        this.selector = selector;
    }

    /**
     * @return The matching rule BSONObject
     * @fn BSONObject getMatcher()
     * @brief Get matching rule
     */
    public BSONObject getMatcher() {
        return matcher;
    }

    /**
     * @param Matcher The matching rule BSONObject
     * @fn void setMatcher(BSONObject matcher)
     * @brief Set matching rule
     */
    public void setMatcher(BSONObject matcher) {
        this.matcher = matcher;
    }

    /**
     * @return The ordered rule BSONObject
     * @fn BSONObject getOrderBy()
     * @brief Get ordered rule
     */
    public BSONObject getOrderBy() {
        return orderBy;
    }

    /**
     * @param OrderBy The ordered rule BSONObject
     * @fn void setOrderBy(BSONObject orderBy)
     * @brief Set ordered rule
     */
    public void setOrderBy(BSONObject orderBy) {
        this.orderBy = orderBy;
    }

    /**
     * @return The sepecified access plan BSONObject
     * @fn BSONObject getHint()
     * @brief Get sepecified access plan
     */
    public BSONObject getHint() {
        return hint;
    }

    /**
     * @param Hint The sepecified access plan BSONObject
     * @fn void setHint(BSONObject hint)
     * @brief Set sepecified access plan
     */
    public void setHint(BSONObject hint) {
        this.hint = hint;
    }

    /**
     * @return The count of BSONObjects to skip
     * @fn Long getSkipRowsCount()
     * @brief Get the count of BSONObjects to skip
     */
    public Long getSkipRowsCount() {
        return skipRowsCount;
    }

    /**
     * @param SkipRowsCount The count of BSONObjects to skip
     * @fn void setSkipRowsCount(Long skipRowsCount)
     * @brief Set the count of BSONObjects to skip
     */
    public void setSkipRowsCount(Long skipRowsCount) {
        this.skipRowsCount = skipRowsCount;
    }

    /**
     * @return The count of BSONObjects to return
     * @fn Long getReturnRowsCount()
     * @brief Get the count of BSONObjects to return
     */
    public Long getReturnRowsCount() {
        return returnRowsCount;
    }

    /**
     * @param ReturnRowsCount The count of BSONObjects to return
     * @fn void setReturnRowsCount(Long returnRowsCount)
     * @brief Set the count of BSONObjects to return
     */
    public void setReturnRowsCount(Long returnRowsCount) {
        this.returnRowsCount = returnRowsCount;
    }

    /**
     * @return The query flag
     * @fn int getFlag()
     * @brief Get the query
     * @see com.sequoiadb.base.DBCollection.query
     */
    public int getFlag() {
        return flag;
    }

    /**
     * @param The query flag as below:
     *            DBQuery.FLG_QUERY_STRINGOUT
     *            DBQuery.FLG_QUERY_FORCE_HINT
     *            DBQuery.LG_QUERY_PARALLED
     *            DBQuery.FLG_QUERY_WITH_RETURNDATA
     * @fn void setFlag(int flag)
     * @brief Set the query flag
     * @see com.sequoiadb.base.DBCollection.query
     */
    public void setFlag(int flag) {
        this.flag = flag;
    }

    private static int _regulate(final int newFlags, final int flag) {
        int retFlags = newFlags;
        Integer tmpFlag = flagsMap.get((Integer) flag);
        if (tmpFlag == null)
            return retFlags;
        if (tmpFlag != flag) {
            retFlags &= ~flag;
            retFlags |= tmpFlag;
        }
        return retFlags;
    }

    static int regulateFlag(final int flag) {
        int newFlags = flag;
        if ((flag & FLG_QUERY_STRINGOUT) != 0) {
            newFlags = _regulate(newFlags, FLG_QUERY_STRINGOUT);
        }
        if ((flag & FLG_QUERY_FORCE_HINT) != 0) {
            newFlags = _regulate(newFlags, FLG_QUERY_FORCE_HINT);
        }
        if ((flag & FLG_QUERY_PARALLED) != 0) {
            newFlags = _regulate(newFlags, FLG_QUERY_PARALLED);
        }
        if ((flag & FLG_QUERY_WITH_RETURNDATA) != 0) {
            newFlags = _regulate(newFlags, FLG_QUERY_WITH_RETURNDATA);
        }
        if ((flag & FLG_QUERY_KEEP_SHARDINGKEY_IN_UPDATE) != 0) {
            newFlags = _regulate(newFlags, FLG_QUERY_KEEP_SHARDINGKEY_IN_UPDATE);
        }
        return newFlags;
    }
}

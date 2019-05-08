package com.sequoiadb.threadexecutor;

import java.util.ArrayList;
import java.util.List;

public abstract class ResultAnalyzer {
    List<ResultStore> resultList = new ArrayList<>();

    public ResultAnalyzer add(ResultStore collector) {
        resultList.add(collector);
        return this;
    }

    public List<ResultStore> getResultList() {
        return resultList;
    }

    public abstract void analyze() throws Exception;
}

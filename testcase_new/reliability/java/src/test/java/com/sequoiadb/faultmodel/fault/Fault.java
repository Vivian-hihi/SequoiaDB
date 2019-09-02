package com.sequoiadb.faultmodel.fault;

public abstract class Fault {

    private String name;

    public Fault(String name) {
        this.name = name;
    }

    public String getName() {
        return name;
    }

    public abstract void make() throws Exception;

    public abstract boolean checkMake() throws Exception;

    public abstract void restore() throws Exception;

    public abstract boolean checkRestore() throws Exception;

    public abstract void init() throws Exception;

    public abstract void fini() throws Exception;

}

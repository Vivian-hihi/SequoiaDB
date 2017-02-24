package com.sequoiadb.exception;

public class CommException extends RuntimeException {
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	private int errorCode = -9999; // -9999表示这个异常没有错误码

	@Override
	public String getMessage() {
		return super.getMessage();
	}

	public CommException(String errorMsg) {
		super(errorMsg);
	}

	public CommException(String errorMsg, int errorCode) {
		super("error code:"+errorCode+",error message:"+errorMsg);
		this.errorCode = errorCode; 
	}
	
	public CommException(Throwable e) {
		super(e);
	}

	public int getErrorCode() {
		return errorCode;
	}

	
}

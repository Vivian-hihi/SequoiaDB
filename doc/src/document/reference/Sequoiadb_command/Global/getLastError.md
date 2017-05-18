##名称##

getLastError - 获取当前操作的错误码。

##语法##

**getLastError()**

##类别##

Global

##描述##

获取当前操作的错误码。

##参数##

无

##返回值##

返回 Int32 类型的值。其中0表示无错误，非0表示错误。

##版本##

v2.6及以上版本。

##示例##

1. 获取当前操作的错误码

  ```lang-javascript
  > db = new Sdb()
  (nofile):0 uncaught exception: -15
  > getLastError()
  -15
  ```

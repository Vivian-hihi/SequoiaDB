##名称##

showClass - （内部使用）显示已经注册到 spt 模块框架中的 javascript 类。

##语法##

**showClass([className])**

##类别##

Global

##描述##

显示已经注册到 spt 模块框架中的 javascript 类。

注意：

* 该接口为内部调试使用的接口。

##参数##

* `className` ( *String*， *Optional* )

	若指定具体的类，该接口将显示该类所包含的方法。

##返回值##

无。

##错误##

无。

##版本##

v2.6及以上版本。

##示例##

1. 显示已经注册到 spt 模块框架中的 javascript 类。

	```lang-javascript
	> showClass()
	All classes:
   		BSONArray
   		BSONObj
   		Cmd
   		File
   		FileContent
   		Hash
   		Oma
   		Remote
   		Sdbtool
   		Ssh
   		System
 	```
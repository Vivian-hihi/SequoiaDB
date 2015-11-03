协调节点（coord）和数据节点（data）对外提供 REST 接口访问，同时支持 HTTP 和 HTTPS 协议。

##通用请求头##

                   说明                                   例子
  ---------------- -------------------------------------- -------------------------------------------------
  Content-Type     请求内容的类型                         application/x-www-form-urlencoded;charset=UTF-8
  Content-Length   请求内容的长度                         54
  Host             主机名（协调节点或数据节点的服务地址）   192.168.1.214:11814

<pre class="prettyprint lang-diy">
POST / HTTP/1.0
Content-Type: application/x-www-form-urlencoded;charset=UTF-8
Content-Length: 54
Host: 192.168.1.214:11814</pre>

##通用响应头##

                   说明             例子
  ---------------- ---------------- -----------
  Content-Type     响应内容的类型   text/html
  Content-Length   响应内容的长度   54

<pre class="prettyprint lang-diy">
HTTP/1.1 200 Ok
Content-Length: 35
Content-Type: text/html</pre>

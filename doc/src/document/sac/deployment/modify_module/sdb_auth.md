
- 为 SequoiaDB 服务创建鉴权。不同的服务创建鉴权的方式不一样。

   演示创建 SequoiaDB 鉴权，用户名 root，密码 123。为了保证安全，生产环境请不要使用简单密码。

      ```lang-javascript
      $ cd /opt/sequoiadb/bin
      $ ./sdb

      > db = new Sdb( "localhost", 11810 )
      localhost:11810
      Takes 0.101142s.
      > db.createUsr( "root", "123" )
      Takes 0.394340s.
      ```

- 在 SAC 设置服务鉴权

   1. 在 **部署** 页面，创建鉴权前。

      ![设置鉴权](sac/deployment/modify_module/sdb_auth_1.jpg)

   2. 创建鉴权后，服务报错：Authority is forbidden，错误码 -179。

      ![设置鉴权](sac/deployment/modify_module/sdb_auth_2.jpg)

   3. 点击服务的 **鉴权**，在窗口输入 **用户名** 和 **密码**。

      ![设置鉴权](sac/deployment/modify_module/sdb_auth_3.jpg)

   4. SAC 可以访问服务。

      ![设置鉴权](sac/deployment/modify_module/sdb_auth_4.jpg)
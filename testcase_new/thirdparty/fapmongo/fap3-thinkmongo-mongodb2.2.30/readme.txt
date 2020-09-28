配置文件相对路径
nodejs-12.14.1/demo/src/config/adapter.js

修改访问地址：
exports.model = {
  type: 'mongo',
  common: {
      logConnect: isDev,
      logger: msg => think.logger.info(msg)
  },
  mongo: {
      host: 'localhost',
      database: 'cs_think_mongo',  // 自己创建的数据库名字
      port: 11817,
      user: '',
      password: ''
  }
};


测试用例相对路径：
nodejs-12.14.1/demo/src/controller/index.js

执行测试用例：
启动npm：npm start
web页面加载：http://192.168.20.55:8360/


查看当前mongodb版本：
vi nodejs-12.14.1/demo/package-lock.json

// think-mongo驱动mongodb的版本

    "think-mongo": {
      ......
      "requires": {
        ......
        "mongodb": "^2.2.30",
        ......

// mongoose驱动mongodb的版本

    "mongoose": {
      ......
      "requires": {
        ......
        "mongodb": "3.1.13",
        ......

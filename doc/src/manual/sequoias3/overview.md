SequoiaS3 系统实现通过 AWS S3 接口访问 SequoiaDB 的能力。

SequoiaS3 将 S3 接口中的区域、桶和对象映射为 SequoiaDB 中的集合空间，集合，记录和Lob，实现桶的增、删、查，对象的增、删、查，对象的版本管理，以及分段上传的能力，支持从 Amazon S3 或其他实现 S3 接口的存储服务平滑迁移到 SequoiaDB数据库。
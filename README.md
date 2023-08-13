<<<<<<< HEAD
# kvstore
一个内存数据库
=======
### 命令
SET key value
GET key
APPAND key value//追加字符串
LEN key //返回字符串长度
DELETE key
EXIST key //是否存在key

ASET key value[values...]
AGET key
ACOUNT key
ADELETE key [values...] //返回删除value个数
AEXIST key value //判断value是否存在于key中

LPUSH key value[values...]
RPUSH key value[values...]
LGET key
LCOUNT key
LDELETE key [values...] //返回删除value个数
LEXIST key value //判断value是否存在于key中

RSET key field value [field value ...]
RGET key field
RCOUNT key //一个key的所有value个数
RDELETE key [fields...] //返回删除value个数
REXIST key field

SSET key value[values...]
SGET key
SCOUNT key
SDELETE key [values..] //返回成功删除value个数
SEXIST key value

quit //客户端退出

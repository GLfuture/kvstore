<!--
 * @Description: 
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-09-29 05:40:03
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-11 09:10:39
-->
<<<<<<< HEAD
# kvstore
一个内存数据库
### 依赖
spdlog
rapaidjson(已内置)
gcc 版本支持C++20 (8.0以上) 

=======
### 协议头
version:1.0 //4byte
length //4byte
=======


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
LPOP key [values...] //返回删除value个数
RPOP key [values...]
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

BEG

END //事务

CLEAN.CACHE //清除回滚缓存
CLEAN.AOF //清除aof文件(本次的aof文件不会直接删除，会被保存到backup.kv，并删除原backup.kv，会在第二次调用该命令彻底删除)


quit //退出客户端


======
### 线程安全
采用读写锁以提高性能，较之前的版本性能得到大幅提高(肉眼可见的执行速度变快)并且store和store中的每一个key都对应一个锁，而不是直接对store进行锁死，对性能有很大提升，比如需要对store结构进行变更且需要对其中某个key所对应的数据结构进行更改，这时结构更改完毕后立即释放store写锁，并对key写锁进行上锁，其他线程此时可以访问该store的其他key而不需要一直等待该操作全部完成


======
### 持久化
1.定时将各数据结构写入磁盘
2.有一个文件保存每次执行的命令，并在执行完毕后删除该条命令，作用是防止宕机未执行的命令(暂不考虑，代价太大，每一次都要读写文件)
3.有一个文件保留不超过file_max_size的命令，以便HISTORY n查看前n次命令
4.有一个文件存储事务

======
### 异常情况
1.解决客户端断开却还有命令没有处理的问题

2.解决死锁问题

3.解决Affairs在BEG时未创建的问题

4.对TCP粘包进行处理(规定协议，协议头中包含长度信息)

5.解决string遇\0截断错误

6.BackUp浅拷贝问题

7.Rollback也需要记录到aof文件，同时需要在每次beg之前back.store的内容记录到文件中
========
### 后续优化
可以将解析命令放在客户端处理完毕由协议带给服务器，减少数据库压力
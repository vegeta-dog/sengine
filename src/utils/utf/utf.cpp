#include "utf.h"
#include <iostream>
#include <string.h>
#include <bits/stdc++.h>

/**
 * @brief 截取含有utf字符的字符串 左闭右开
 *
 * @param begin 起始位置 (起始位置是0)
 * @param end 结束位置  (结尾位置是length)
 * @return std::string 结果串
 */
std::string libUTF::substr_utf(const std::string &data, int begin, int end)
{
    // 传入utf8字符的第一个字节,可以知道这个字符有多少个byte
    auto char_size = [](const unsigned int& bit) -> unsigned int {  
        return bit >= 0x80 ? 3 : 1;
    };
    const char *chs = data.c_str();
    int strl = strlen(chs), l = 0, r = 0;
    for (int i = 0; l < strl && i < begin; i++)
        l += char_size(chs[l]);
    for (int i = 0; r < strl && i < end; i++) 
        r += (r == strl) ? 1 : char_size(chs[r]);
    return data.substr(l, r - l);
}


/**
 * @brief 截取含有utf字符的字符串
 *
 * @param begin 起始位置 (起始位置是0)
 * @param len 长度  
 * @return std::string 结果串
 */
std::string libUTF::substr_utf_bylen(const std::string &data, int begin, int len)
{
    return substr_utf(data, begin, begin + len);
}


// 测试代码:
// int main()
// {
//     // std::string str = "but你好阿我日聂玛";
//     std::string str = "原文httpwwwruanyifengcomblog201407databaseimplementationhtml有应用软件之中数据库可能是最复杂的MySQL的手册有3000多页PostgreSQL的手册有2000多页Oracle的手册更是比它们相加还要厚但是自己写一个最简单的数据库做起来并不难Reddit上面有一个帖子只用了几百个字就把原理讲清楚了下面是我根据这个帖子整理的内容一数据以文本形式保存第一步就是将所要保存的数据写入文本文件这个文本文件就是你的数据库为了方便读取数据必须分成记录每一条记录的长度规定为等长比如假定每条记录的长度是800字节那么第5条记录的开始位置就在3200字节大多数时候我们不知道某一条记录在第几个位置只知道主键primarykey的值这时为了读取数据可以一条条比对记录但是这样做效率太低实际应用中数据库往往采用B树Btree格式储存数据二什么是B树要理解B树必须从二叉查找树Binarysearchtree讲起二叉查找树是一种查找效率非常高的数据结构它有两个特点1最个节点最多只有两个子树2左子树都为小于父节点的值右子树都为大于父节点的值3在n个节点中找到目标值一般只需要logn次比较二叉查找树的结构不适合数据库因为它的查找效率与层数相关越处在下层的数据就需要越多次比较极端情况下n个数据需要n次比较才能找到目标值对于数据库来说每进入一层就要从硬盘读取一次数据这非常致命因为硬盘的读取时间远远大于数据处理时间数据库读取硬盘越少越好B树是对二叉查找树的改进它的设计思想是将相关数据尽量集中在一起以便一次读取多个数据减少硬盘操作次数B树的特点也有三个1一个节点可以容纳多个值比如上图中最多的一个节点容纳了4个值2除非数据已经填满否则不会增加新的层也就是说B树追求层越少越好3子节点中的值与父节点中的值有严格的大小对应关系一般来说如果父节点有a个值那么就有a1个子节点比如上图中父节点有两个值7和16就对应三个子节点第一个子节点都是小于7的值最后一个子节点都是大于16的值中间的子节点就是7和16之间的值这种数据结构非常有利于减少读取硬盘的次数假定一个节点可以容纳100个值那么3层的B树可以容纳100万个数据如果换成二叉查找树则需要20层假定操作系统一次读取一个节点并且根节点保留在内存中那么B树在100万个数据中查找目标值只需要读取两次硬盘三索引数据库以B树格式储存只解决了按照主键查找数据的问题如果想查找其他字段就需要建立索引index所谓索引就是以某个字段为关键字的B树文件假定有一张雇员表包含了员工号主键和姓名两个字段可以对姓名建立索引文件该文件以B树格式对姓名进行储存每个姓名后面是其在数据库中的位置即第几条记录查找姓名的时候先从索引中找到对应第几条记录然后再从主表中读取原始记录这种索引查找方法叫做索引顺序存取方法IndexedSequentialAccessMethod缩写为ISAM它已经有多种实现比如CISAM库和DISAM库只要使用这些代码库就能自己写一个最简单的数据库四高级功能部署了最基本的数据存取包括索引以后还可以实现一些高级功能1SQL语言是数据库通用操作语言所以需要一个SQL解析器将SQL命令解析为对应的ISAM操作2数据库连接join是指数据库的两张表通过外键建立连接关系你需要对这种操作进行优化3数据库交易transaction是指批量进行一系列数据库操作只要有一步不成功整个操作都不成功所以需要有一个操作日志以便失败时对操作进行回滚4备份机制保存数据库的副本5远程操作使得用户可以在不同的机器上通过TCPIP协议操作数据库";
//     // std::string str = "hththt1238912聂玛比a1斯草泥马";
//     // char c[4] = "草";
//     // char c2[4] = "斯";
//     // std::cout << std::bitset<8>((unsigned int)c[0]) << std::endl;
//     // std::cout << std::bitset<8>((unsigned int)'t') << std::endl;
//     std::cout << libUTF::substr_utf(str, 0, 110) << std::endl;
// }

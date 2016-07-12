
# Redbase 开发文档

----------------------------

##一. 总体框架
 
#### 1.1 概述

RedBase是一个轻量级的关系型数据库, 框架的设计思路主要根据斯坦福大学的[Database System Implementation(CS346)](https://web.stanford.edu/class/cs346/2015/)课程提供的Redbase数据库框架来设计. 
主要包含5个模块:
 - PageFile(PF)
 - RecordManager(RM)
 - IndexManager(IX)
 - SystemManager(SM)
 - QueryManager(QM)
 在具体的实现中, 为了遵循模块化程序设计的原则, 将各个模块按照不同的功能分成多个类来实现, 具体实现细节将在后面介绍

#### 1.2 基本功能
 - 支持的数据类型: INT, FLOAT, STRING(需要指定大小). 
 - 数据库的建立和删除，以及数据库的统计信息查询
 - 表的建立和删除，以及表的统计信息查询
 - 索引的建立和删除
 - 使用与MySQL语法相似的SQL语句对表或数据库中的数据执行增、删、改、查操作

        
<br/>
<br/>
<br/>
<br/>
<br/>

#### 1.3 层次结构
![enter image description here](https://lh3.googleusercontent.com/Vu26zhUf1W6pQ2rithKpciFHnEMdxJzsts7CMOP0geyF2kdAf6CeBU2hr5qj7yXdR7BSkCn6=s0 "绘图1.jpg")

#### 1.4 设计语言和运行环境
开发环境：Visual Studio 2015
编程语言：C++（主要使用C++11推荐语法）


----------

<br/>
<br/>

## 二. 各模块的具体功能
#### 2.1 Page File Manager(PF)
PF模块是系统的最底层模块, 主要向高层次的结构(Index Manager和Record Manager)提供以页为基本单元的文件IO操作. 在该模块中, 主要实现了创建, 删除, 打开和关闭文件四个操作. 
对于单个文件(PageFile类)的操作, 必须先从PF模块打开一个PageFile实例为操作对象, 以页(Page)为最小访问单元. PageFile提供了从文件中获取一个新的Page, 获取一个特定页号的Page, 强制更新(Force)文件中特定页号的Page等方法. 
实际上, 由于文件系统IO速度比较慢, 为了提高获取数据的速度, PM模块还需要对每个文件维护一个Buffer(BufferManager类). 每个BufferManager实例管理一个PageFile实例以及一个Buffer(用哈希表结构存储在内存中), 上层模块只能通过BufferManager来间接地访问PageFile获取所需Page. 

#### 2.2 Record Manager(RM)
RM模块是管理记录文件的模块, 主要面向Query Manager和System Manager两个模块. RM模块只提供了对记录文件的创建, 删除, 打开和关闭四个操作. 
在RedBase的实现中, 每个表中有若干条记录, 一个表中的所有记录必须存在同一个记录文件(RecordFile)中. 对单个记录文件的操作, 必须通过RecordFile实例进行操作. RecordFile提供了对该文件中记录的增删改查四个操作.

#### 2.3 Index Manager(IX)
IX模块是管理索引的模块, 主要提供了对某个表的属性进行创建, 删除索引, 并对已创建的索引的打开, 关闭四个操作. 关于索引的具体实现, RedBase的索引与大多数数据库引擎类似, 使用B+树作为索引的数据结构, 每个B+树的结点存储在一个Page中, 以达到较高的索引效率. 
其他模块对于单个索引的数据插入, 删除, 查询, 必须先通过Index Manager获取一个IndexHandle实例, 并通过这个IndexHandle实例进行操作.

#### 2.4 System Manager(SM)
SM模块直接面向Command Parser，提供一些可能会影响整个数据库系统的操作，包括：
 - 创建、删除一个数据库/关系表/索引等（DDL）
 - 维护系统中各个数据库和表的Catalog
 - 从文件中导入数据
 - 设置数据库或表的参数
 - 输出关系表

#### 2.5 Query Manager(QM)
QM模块与SM模块类似，主要向Command Parser提供了用于执行特定的SQL操作。当前的实现支持的SQL操作有Select（包括多表联查），Insert，Delete，Update四种操作。操作的执行主要先通过System Manager获取查询表的结构, 分析后通过调用Record Manager的方法查询并解析查询的结果, 返回给Command Parser.


----------


## 三. 重要接口

### 2.1 PF模块
#### PageManager类

`RETCODE CreateFile (const char * fileName);       // Create a new file`
创建一个新的文件(索引的结点或是记录文件)

`RETCODE DestroyFile (const char * fileName);       // Destroy a file` 
删除一个文件

`RETCODE OpenFile (const char * fileName, PageFilePtr & fileHandle);		// Open a file` 
打开一个文件并获取该文件的PageFile实例

`RETCODE CloseFile (PageFilePtr &fileHandle);				// Close a file` 
关闭一个文件并释放PageFile实例

#### PageFile类(只能由BufferManager调用)
`RETCODE GetThisPage (PageNum pageNum, PagePtr &pageHandle) ;` 
获取该文件中特定一页, 返回Page实例

`RETCODE AllocatePage (PagePtr &pageHandle);` 
分配一个新的页, 返回Page实例

`RETCODE DisposePage (PageNum pageNum);` 
释放(不再使用)特定的页

`RETCODE ForcePage (PageNum page, const PagePtr & pageHande);` 
强制把一个页写入文件中的特定页中

#### BufferManager类(由其他类调用, 间接操作PageFile)

`RETCODE GetPage (PageNum page, PagePtr & pBuffer);	` 
获取一个特定的页, 返回Page实例

`RETCODE MarkDirty (PageNum page);` 
把一个页标记为Dirty(修改过)

`RETCODE LockPage (PageNum page);` 
把一个页锁定在Buffer中

`RETCODE UnlockPage (PageNum page);` 
把一个已锁定的页释放

`RETCODE ForcePage (PageNum page);` 
强制把一个页写入文件

`RETCODE FlushPages ( );` 
把Buffer中的所有页写入文件

`RETCODE AllocatePage (PagePtr & page);	`	
分配一个新的页, 返回Page实例

`RETCODE DisposePage (PageNum page);` 
释放特定的页

### 2.2 RM模块
#### RecordFile类
`RETCODE Open (const BufferManagerPtr & ptr );`
打开一个记录文件

`RETCODE InsertRec (const char *pData, RecordIdentifier &rid);       `
插入记录

`RETCODE DeleteRec (const RecordIdentifier&rid);  `
删除记录

`RETCODE UpdateRec (const Record &rec);       `
修改一条记录

`RETCODE GetRec (const RecordIdentifier &rid, Record &rec) const;`
获取一条记录

`RETCODE ForcePages (PageNum pageNum) const; `
将缓冲区的页重新写回硬盘中

#### RecordFileManager类

`RETCODE CreateFile (const char *fileName, size_t recordSize);`
创建文件
	
`RETCODE DestroyFile (const char *fileName);`
删除文件

`RETCODE OpenFile (const char *fileName, RecordFilePtr &fileHandle);`
打开文件

`RETCODE CloseFile (RecordFilePtr &fileHandle);`
关闭文件

### 2.3 IX模块
IndexManager 类
`RETCODE CreateIndex (const char *fileName, AttrType attrType, int attrLength);`
创建一个新的索引
`RETCODE DestroyIndex (const char *fileName);`
删除一个索引
`RETCODE OpenIndex (const char *fileName, IndexHandlePtr &indexHandle);`
打开数据文件中的索引
`RETCODE CloseIndex (IndexHandlePtr &indexHandle);`
关闭索引

IndexHandle类
`RETCODE InsertEntry (void *pData, const RecordIdentifier & rid);`
插入索引（B+树算法）
`RETCODE DeleteEntry (void *pData, const RecordIdentifier & rid);`
删除索引
`RETCODE ForcePages ( );`
拷贝整棵B+树到磁盘


### 2.4 SM模块
#### SystemManager 类

`RETCODE CreateDb (const char * dbName, PageFileManagerPtr & pfMgr);`
创建一个新的数据库

`RETCODE OpenDb (const char *dbName); `
打开一个数据库

`RETCODE CloseDb ( );  `
关闭当前打开的数据库

`RETCODE CreateTable (const char *relName, Create relation, int attrCount, AttrInfo *attributes);`
(在当前打开的数据库中)创建表

`RETCODE DropTable (const char *relName);`
删除表

`RETCODE CreateIndex (const char *relName, const char *attrName);`
对某个表的一个属性建立索引

`RETCODE DropIndex (const char *relName, Destroy index, const char *attrName);`
删除索引

#### 2.5 Query Manager(QM)

	RETCODE Select (
	int           nSelAttrs,        //  查询的属性个数
	const RelAttr selAttrs[],       // 查询的属性
	int           nRelations,       // 查询的关系表个数
	const char * const relations[], // 查询的关系表
	int           nConditions,      //  查询的条件个数
	const Condition conditions[]);  // 查询的具体条件
根据传入的参数执行查询操作。

	RETCODE Insert (const char  *relName,           // 将要插入的关系表
	int         nValues,            // 插入的属性值个数
	const Value values[]);          // 插入的值
对一个特定的表执行插入操作。

	RETCODE Delete (const char *relName,   // 将要删除记录的关系表
	int        nConditions,         // 删除的条件个数
	const Condition conditions[]);  // 删除的具体条件
对一个特定的表以特定的条件删除记录

	RETCODE Update (const char *relName, // 将要更新的关系
	const RelAttr &updAttr,         // 更新的属性
	const int bIsValue,             // 标记用来赋值的是常量(rhsValue)还是其他记录的值(rhsRelAttr)
	const RelAttr &rhsRelAttr,      // 用来赋值的记录值
	const Value &rhsValue,          // 用来赋值的常量
	int   nConditions,              // 赋值的条件个数
	const Condition conditions[]);  // 赋值的具体条件
更新一个表中满足条件的记录


----------


## 四. 使用语法

#### Table 操作
	create table relName(attrName1 Type1, attrName2 Type2, ..., attrNameN TypeN);
	drop table relName;

#### Index 操作
	create index relName(attrName);	
	drop index relName(attrName);

#### Print 操作
	print relName;

#### Select
<code class="hljs sql"><span class="hljs-operator"><span class="hljs-keyword">Select</span> </span><i><span class="hljs-operator">A</span><sub><span class="hljs-operator">1</span></sub></i><span class="hljs-operator">, </span><i><span class="hljs-operator">A</span><sub><span class="hljs-operator">2</span></sub></i><span class="hljs-operator">, </span><span class="text-muted"><span class="hljs-operator">…</span></span><span class="hljs-operator">, </span><i><span class="hljs-operator">A</span><sub><span class="hljs-operator">m</span></sub></i><span class="hljs-operator">
<span class="hljs-keyword">From</span> </span><i><span class="hljs-operator">R</span><sub><span class="hljs-operator">1</span></sub></i><span class="hljs-operator">, </span><i><span class="hljs-operator">R</span><sub><span class="hljs-operator">2</span></sub></i><span class="hljs-operator">, </span><span class="text-muted"><span class="hljs-operator">…</span></span><span class="hljs-operator">, </span><i><span class="hljs-operator">R</span><sub><span class="hljs-operator">n</span></sub></i><span class="hljs-operator">
</span><span class="text-muted"><span class="hljs-operator">[</span></span><span class="hljs-operator"><span class="hljs-keyword">Where</span> </span><i><span class="hljs-operator">A</span><sub><span class="hljs-operator">1</span></sub><span class="hljs-operator">’</span></i><span class="hljs-operator"> </span><i><span class="hljs-operator">comp</span><sub><span class="hljs-operator">1</span></sub></i><span class="hljs-operator"> </span><i><span class="hljs-operator">AV</span><sub><span class="hljs-operator">1</span></sub></i><span class="hljs-operator"> <span class="hljs-keyword">And</span> </span><i><span class="hljs-operator">A</span><sub><span class="hljs-operator">2</span></sub><span class="hljs-operator">’</span></i><span class="hljs-operator"> </span><i><span class="hljs-operator">comp</span><sub><span class="hljs-operator">2</span></sub></i><span class="hljs-operator"> </span><i><span class="hljs-operator">AV</span><sub><span class="hljs-operator">2</span></sub></i><span class="hljs-operator"> <span class="hljs-keyword">And</span> </span><span class="text-muted"><span class="hljs-operator">…</span></span><span class="hljs-operator"> <span class="hljs-keyword">And</span> </span><i><span class="hljs-operator">A</span><sub><span class="hljs-operator">k</span></sub><span class="hljs-operator">’</span></i><span class="hljs-operator"> </span><i><span class="hljs-operator">comp</span><sub><span class="hljs-operator">k</span></sub></i><span class="hljs-operator"> </span><i><span class="hljs-operator">AV</span><sub><span class="hljs-operator">k</span></sub></i><span class="text-muted"><span class="hljs-operator">]</span></span><span class="hljs-operator">;</span>
</code>
#### Insert
<code class="hljs sql"><span class="hljs-operator"><span class="hljs-keyword">Insert</span> <span class="hljs-keyword">Into</span> </span><i><span class="hljs-operator">relName</span></i><span class="hljs-operator"> <span class="hljs-keyword">Values</span> (</span><i><span class="hljs-operator">V</span><sub><span class="hljs-operator">1</span></sub></i><span class="hljs-operator">, </span><i><span class="hljs-operator">V</span><sub><span class="hljs-operator">2</span></sub></i><span class="hljs-operator">, </span><span class="text-muted"><span class="hljs-operator">…</span></span><span class="hljs-operator">, </span><i><span class="hljs-operator">V</span><sub><span class="hljs-operator">n</span></sub></i><span class="hljs-operator">);</span>
</code>

#### Update
<code class="hljs sql"><span class="hljs-operator"><span class="hljs-keyword">Update</span> </span><i><span class="hljs-operator">relName</span></i><span class="hljs-operator">
<span class="hljs-keyword">Set</span> </span><i><span class="hljs-operator">attrName</span></i><span class="hljs-operator"> = </span><i><span class="hljs-operator">AV</span></i><span class="hljs-operator">
</span><span class="text-muted"><span class="hljs-operator">[</span></span><span class="hljs-operator"><span class="hljs-keyword">Where</span> </span><i><span class="hljs-operator">A</span><sub><span class="hljs-operator">1</span></sub></i><span class="hljs-operator"> </span><i><span class="hljs-operator">comp</span><sub><span class="hljs-operator">1</span></sub></i><span class="hljs-operator"> </span><i><span class="hljs-operator">AV</span><sub><span class="hljs-operator">1</span></sub></i><span class="hljs-operator"> <span class="hljs-keyword">And</span> </span><i><span class="hljs-operator">A</span><sub><span class="hljs-operator">2</span></sub></i><span class="hljs-operator"> </span><i><span class="hljs-operator">comp</span><sub><span class="hljs-operator">2</span></sub></i><span class="hljs-operator"> </span><i><span class="hljs-operator">AV</span><sub><span class="hljs-operator">2</span></sub></i><span class="hljs-operator"> <span class="hljs-keyword">And</span> </span><span class="text-muted"><span class="hljs-operator">…</span></span><span class="hljs-operator"> <span class="hljs-keyword">And</span> </span><i><span class="hljs-operator">A</span><sub><span class="hljs-operator">k</span></sub></i><span class="hljs-operator"> </span><i><span class="hljs-operator">comp</span><sub><span class="hljs-operator">k</span></sub></i><span class="hljs-operator"> </span><i><span class="hljs-operator">AV</span><sub><span class="hljs-operator">k</span></sub></i><span class="text-muted"><span class="hljs-operator">]</span></span><span class="hljs-operator">;</span>
</code>

#### Delete
<code class="hljs sql"><span class="hljs-operator"><span class="hljs-keyword">Delete</span> <span class="hljs-keyword">From</span> </span><i><span class="hljs-operator">relName</span></i><span class="hljs-operator">
</span><span class="text-muted"><span class="hljs-operator">[</span></span><span class="hljs-operator"><span class="hljs-keyword">Where</span> </span><i><span class="hljs-operator">A</span><sub><span class="hljs-operator">1</span></sub></i><span class="hljs-operator"> </span><i><span class="hljs-operator">comp</span><sub><span class="hljs-operator">1</span></sub></i><span class="hljs-operator"> </span><i><span class="hljs-operator">AV</span><sub><span class="hljs-operator">1</span></sub></i><span class="hljs-operator"> <span class="hljs-keyword">And</span> </span><i><span class="hljs-operator">A</span><sub><span class="hljs-operator">2</span></sub></i><span class="hljs-operator"> </span><i><span class="hljs-operator">comp</span><sub><span class="hljs-operator">2</span></sub></i><span class="hljs-operator"> </span><i><span class="hljs-operator">AV</span><sub><span class="hljs-operator">2</span></sub></i><span class="hljs-operator"> <span class="hljs-keyword">And</span> </span><span class="text-muted"><span class="hljs-operator">…</span></span><span class="hljs-operator"> <span class="hljs-keyword">And</span> </span><i><span class="hljs-operator">A</span><sub><span class="hljs-operator">k</span></sub></i><span class="hljs-operator"> </span><i><span class="hljs-operator">comp</span><sub><span class="hljs-operator">k</span></sub></i><span class="hljs-operator"> </span><i><span class="hljs-operator">AV</span><sub><span class="hljs-operator">k</span></sub></i><span class="text-muted"><span class="hljs-operator">]</span></span><span class="hljs-operator">;</span>
</code>


----------


## 五. 实际测试

创建一张students表, 其中有三个属性, id和age均为int型(i), name为char(20)类型.
![enter image description here](https://lh3.googleusercontent.com/JXV18def9WQguACou2EkU7Eq3LGxdjOFVL0otaVMLlsBL4swjcMMN0gu2hTPRVHtxmgI9oCg=s0 "create table.PNG")

输出表的信息
![enter image description here](https://lh3.googleusercontent.com/Ub9JJoWKqJrR-KL2l1b-mnQ-LJtmHEhvtn0vy3y_4ABWYXMP0MjwN9L-E8OPUaY5HM8kE-Hv=s0 "printtable.PNG")

插入第一条记录
![enter image description here](https://lh3.googleusercontent.com/yXfDfDIS7Yt2k4CQMJ3rPodmjv-htCogKGsGgASI6teAIE50QPFlKs7Gp2ls7DVdpQcKXUNj=s0 "insert.PNG")


插入第二条记录
![enter image description here](https://lh3.googleusercontent.com/-jAZ9uaxJX3zoncIwBwTOzhRKmU2vdJ4mEp-BoLSH3oEfA-hJeFrK6BC6Iszn67xR0EBEIij=s0 "insert2.PNG")

查询表中的所有记录并输出
![enter image description here](https://lh3.googleusercontent.com/z0D7rKANadiIHqFvgHhc88aho-rzvu3MKfn-G-X6dBBac_RyHA1sUMy2axMgEBq1U9VHDWWu=s0 "_selectstar.PNG")

条件查询
![enter image description here](https://lh3.googleusercontent.com/NZEW5eVdY7DHWoKFh5axXeAOrwzdc1p72utG_-ClfL9tdPl-DePrDR1C4xwqxZeM5tKYOtLr=s0 "_select2.PNG")

更新记录
![enter image description here](https://lh3.googleusercontent.com/T-gRAKA6vTCS-FaOKXO7wv8k9RG8LC_gyp9AFmTJpOujR_jXV2u_6vPMjfW-nY8eb76mV-pA=s0 "update.PNG")

删除记录
![enter image description here](https://lh3.googleusercontent.com/d2gTGsuES7OfzTf5XoOBPztPaexe1Eg4SzTOSgRf-Xp1yRlcXjfZmy676EmHlKiRTPgcW-An=s0 "delete.PNG")

<br/>
<br/>
检测删除结果
![enter image description here](https://lh3.googleusercontent.com/FzLgK3ocpoG7jxgMuBX78oeuoCfbS867jv8zH1RZZSNoNLVekmznJJnj0MQ_On6uHqJJULQd=s0 "_select3.PNG")

创建索引
![enter image description here](https://lh3.googleusercontent.com/cdGRafWDe1mogIkgC0LFZxNb28QDfT4z8spL-1R12PU0lF1mN4KfikX27B8dvfaimmt13Cq1=s0 "createindex.PNG")


----------

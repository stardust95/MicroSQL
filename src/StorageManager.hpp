#pragma once

/*
	1. 聚集存储, 即按主键的顺序存放,
	2. 每个文件是一个数据库, 文件内部分为若干页(上限为PageNum的大小, 64位)

*/

class StorageManager {
public:
	StorageManager ( );
	~StorageManager ( );

private:

};

StorageManager::StorageManager ( ) {
}

StorageManager::~StorageManager ( ) {
}

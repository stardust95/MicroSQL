#pragma once

/*
	1. IndexHandle用于操作(查询, 插入, 删除)一个索引, 与大多数数据库采用的索引相同, 当前其内部实现也为B+树
	2. 每个结点的大小为Utils::PAGESIZE, 存放的Key
	3. 一个文件当做一段连续的内存, 指向子节点的指针就是在文件中的偏移
	4. 叶节点和内部结点都统一用一种struct来存
*/

#include "Utils.hpp"
#include "RecordIdentifier.hpp"
#include "BpTreeNode.hpp"

struct IndexHeader {				// the information of every index
	AttrType attrType;
	
	PageNum rootPage;

	size_t attrLength;

	size_t numMaxKeys;			// the order of tree

	size_t numPages;

	size_t height;



};

class IndexHandle {
	friend class IndexManager;

public:
	using Comparator = BpTreeNode::Comparator;

	//IndexHandle ( );
	IndexHandle ();
	~IndexHandle ( );

	RETCODE InsertEntry (void *pData, const RecordIdentifier & rid);  // Insert new index entry ( b+tree algorithm)

	RETCODE DeleteEntry (void *pData, const RecordIdentifier & rid);  // Delete index entry
	
	RETCODE ForcePages ( );                             // Copy all pages (the whole b+tree) to disk

	RETCODE ReadHeader ( );

	BpTreeNodePtr FetchNode (PageNum page) const;

	BpTreeNodePtr FetchNode (const RecordIdentifier &) const;

	BpTreeNodePtr FindLeaf (void * pData) ;
	BpTreeNodePtr FindLargestLeaf ( ) ;

private:
	/*
	The file must be opened before any other operation
	*/
	RETCODE Open (BufferManagerPtr buf);

	bool IsValid ( ) const;

	RETCODE GetThisPage (PageNum, PagePtr &);
	RETCODE GetNewPage (PagePtr  &);

	RETCODE SetHeight (size_t h);

	/*
		Get Info
	*/

	AttrType attrType ( ) const;

	size_t attrLen ( ) const;

	size_t numMaxKeys ( ) const;		// the order of tree

	size_t numPages ( ) const;

	size_t height ( ) const;

/* B+Tree Members */
	BpTreeNodePtr root;
	VoidPtr largestKey;

/*	IndexHandle Members */

	IndexHeader header;

	BufferManagerPtr bufMgr;
	
	bool headerModified;

	bool isOpenHandle;

	std::vector<BpTreeNodePtr> path;
	
	std::vector<size_t> pathPage;

};

using IndexHandlePtr = shared_ptr<IndexHandle>;
//
//IndexHandle::IndexHandle ( ) {
//	largestKey = root = nullptr;
//	
//	header.height = 0;
//
//}

inline IndexHandle::IndexHandle () {
	headerModified = false;
	isOpenHandle = false;
	root = nullptr;

}

IndexHandle::~IndexHandle ( ) {

}

/*
The file must be opened before any other operation
*/
inline RETCODE IndexHandle::Open (BufferManagerPtr buf) {
	PagePtr pagePtr;				// for root page
	DataPtr pData;
	RETCODE result;

	if ( buf == nullptr )
		return RETCODE::INVALIDOPEN;

	bufMgr = buf;

	ReadHeader ( );
	SetHeight (header.height);

	isOpenHandle = true;
	headerModified = false;
	
	bool hasLeaf = false;
	if ( height() == 0 ) {		// is empty tree (without root)
		if ( result = this->GetNewPage (pagePtr) ) {
			Utils::PrintRetcode (result);
			return result;
		}
		pagePtr->GetPageNum (header.rootPage);
		//header.height = 1;
		SetHeight (1);
	} else {
		hasLeaf = true;
		this->GetThisPage (header.rootPage, pagePtr);
	}
	// lock root page
	bufMgr->GetPage (header.rootPage, pagePtr);

	root = make_shared<BpTreeNode> (header.attrType, header.attrLength, pagePtr);

	path[0] = root;
	headerModified = true;
	largestKey = VoidPtr ( reinterpret_cast<void*>(new char[attrLen()]() ) );
	if ( hasLeaf ) {
		BpTreeNodePtr largestLeaf = FindLargestLeaf ( );
		largestLeaf->CopyKeyTo (largestLeaf->GetNumKeys ( ) - 1, largestKey.get());
	}
	return result;
}

inline RETCODE IndexHandle::InsertEntry (void * pData, const RecordIdentifier & rid) {
	
	if ( pData == nullptr )
		return BADKEY;

	RETCODE result;
	BpTreeNodePtr node = FindLeaf (pData);			// find the leaf node to insert
	BpTreeNodePtr newNode = nullptr;
	size_t level = height ( ) - 1;						// from bottom up
	bool newLargest = false;
	void * prevKey = nullptr;

	assert (node != nullptr);

	size_t fitPos = node->FindKey (pData, rid);
	if ( fitPos != RETCODE::KEYNOTFOUND )
		return RETCODE::ENTRYEXISTS;

	if ( node->GetNumKeys ( ) == 0 || node->comp (pData, largestKey.get()) > 0 ){		// if is new key is the largest key
		newLargest = true;
		prevKey = largestKey.get ( );
	}

	if ( (result = node->Insert (pData, rid)) && result != RETCODE::NODEKEYSFULL ) {
		Utils::PrintRetcode (result);
		return result;
	}

	// if the inserting entry has the largest key
	if ( newLargest ) {
		for ( size_t i = 0; i < height ( ); i++ ) {
			size_t pos = path[i]->FindKey (prevKey);
			if ( pos != RETCODE::KEYNOTFOUND )			// here the condition must be true (i.e. the prevKey(currently largest) should be found found)
				path[i]->SetKey (pos, pData);
		}

		memcpy_s (largestKey.get ( ), attrLen ( ), pData, attrLen ( ));
	}

	// no space in node -> overflow nonroot
	void * failedKey = pData;
	RecordIdentifier failedRid = rid;
	while ( result = RETCODE::NODEKEYSFULL ) {			// if the insert node is full

		VoidPtr oldLargest = shared_ptr<void> ( reinterpret_cast<char*>(new char[attrLen()]()) );

		if ( node->LargestKey ( ) == nullptr ) {		// numkeys == 0
			oldLargest = nullptr;
		} else {
			node->CopyKeyTo (node->GetNumKeys ( ) - 1, oldLargest.get());
		}

		PagePtr pagePtr;

		if ( result = this->GetNewPage (pagePtr) ) {
			Utils::PrintRetcode (result);
			return result;
		}

		newNode = make_shared<BpTreeNode> (attrType ( ), attrLen ( ), pagePtr);
		
		if ( result = node->Split (*newNode.get ( )) ) {
			Utils::PrintRetcode (result);
			return result;
		}

		BpTreeNodePtr curRight = FetchNode (newNode->GetRight ( ));
		if ( curRight != nullptr ) {
			curRight->SetLeft (newNode->GetPageNum());
			curRight = nullptr;
		}

		BpTreeNodePtr nodeInsertedInto = nullptr;

		// put the new entry into one of the 2 now.
		// In the comparison,
		// > takes care of normal cases
		// = is a hack for dups - this results in affinity for preserving
		// RID ordering for children - more balanced tree when all keys are the same.

		if ( node->comp (pData, node->LargestKey ( )) >= 0 ) {
			newNode->Insert (failedKey, failedRid);
			nodeInsertedInto = newNode;
		} else {
			node->Insert (failedKey, failedRid);
			nodeInsertedInto = node;
		}
		// go up to parent level and repeat
		if( level < 0 )		// if root
			break;
		size_t posAtParent = pathPage[level];
		BpTreeNodePtr parent = path[level];

		// update old key - keep same addr
		parent->Delete (posAtParent);

		result = parent->Insert (node->LargestKey ( ), node->GetPageRid( ));
		// this result should always be good - we removed first before
		// inserting to prevent overflow.

		// insert new key
		result = parent->Insert (newNode->LargestKey ( ), newNode->GetPageRid ( ));

		// iterate for parent node and split if required
		node = parent;
		failedKey = node->LargestKey ( );
		failedRid = node->GetPageRid ( );

		newNode = nullptr;
	}

	if ( level >= 0 ) {
		// insertion done
		return RETCODE::COMPLETE;
	} else {
		// root split happened
		if ( result = bufMgr->UnlockPage (header.rootPage) ) {
			Utils::PrintRetcode (result);
			return result;
		}
		// make new root node
		PagePtr pagePtr;
		if ( result = GetNewPage (pagePtr) ) {
			Utils::PrintRetcode (result);
			return result;
		}

		root = make_shared<BpTreeNode> (attrType ( ), attrLen ( ), pagePtr);

		root->Insert (node->LargestKey ( ), node->GetPageRid ( ));
		root->Insert (newNode->LargestKey ( ), newNode->GetPageRid ( ));

		// lock root page
		header.rootPage = root->GetPageNum ( );
		PagePtr rootPagePtr;
		if ( result = GetThisPage (header.rootPage, rootPagePtr) ) {
			Utils::PrintRetcode (result);
			return result;
		}

		

	}


	/*MarkDirty and UnLock*/
	//bufMgr->MarkDirty()
	headerModified = true;
	header.numPages++;
	return RETCODE::COMPLETE;
}

inline RETCODE IndexHandle::ForcePages ( ) {

	PagePtr headerPage;
	RETCODE result;
	
	if ( result = bufMgr->GetPage (0, headerPage) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	memcpy_s (headerPage->GetData ( ).get ( ), sizeof (IndexHeader), reinterpret_cast< void* >( &header ), sizeof (IndexHeader));

	if ( result = bufMgr->FlushPages ( ) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	return result;
}

inline RETCODE IndexHandle::ReadHeader ( ) {

	PagePtr pagePtr;
	DataPtr pData;
	RETCODE result;

	if ( result = bufMgr->GetPage (0, pagePtr) ) {
		Utils::PrintRetcode (result);
		return result;
	}
	
	if ( result = pagePtr->GetData (pData) ) {
		Utils::PrintRetcode (result);
		return result;
	}
	
	memcpy_s (reinterpret_cast< void* >( &header ), sizeof (IndexHeader), pData.get ( ), sizeof (IndexHeader));

	return RETCODE::COMPLETE;
}

inline BpTreeNodePtr IndexHandle::FetchNode (PageNum page) const {
	return FetchNode(RecordIdentifier{page, 0});
}

inline BpTreeNodePtr IndexHandle::FetchNode (const RecordIdentifier & rid) const {
	
	PageNum page;
	PagePtr pagePtr;
	RETCODE result;

	if ( result = rid.GetPageNum (page) ) {
		Utils::PrintRetcode (result);
		return nullptr;
	}

	if ( result = bufMgr->GetPage (page, pagePtr) ) {
		Utils::PrintRetcode (result);
		return nullptr;
	}

	return make_shared<BpTreeNode> (BpTreeNode (attrType ( ), attrLen ( ), pagePtr));

}

inline BpTreeNodePtr IndexHandle::FindLargestLeaf ( ) {
	PageNum page;
	PagePtr pagePtr;
	RETCODE result;

	if ( root == nullptr )
		return nullptr;
	if ( height ( ) == 1 ) {
		path[0] = root;
		return root;
	}

	for ( size_t i = 1; i < height ( ); i++ ) {
		auto rid = path[i - 1]->GetRid (path[i - 1]->GetNumKeys ( ) - 1);
		if ( rid.GetPageNum (page) == RETCODE::COMPLETE && page == Utils::UNKNOWNPAGENUM ) {
			return nullptr;
		}
		// start with a new page 
		if ( path[i] != nullptr ) {
			bufMgr->UnlockPage (path[i]->GetPageNum());
			path[i] = nullptr;
			//pathPage[i] = 0;
		} 		
		path[i] = FetchNode (rid);

		// make the buffer lock page
		if ( result = bufMgr->GetPage (page, pagePtr) ) {
			Utils::PrintRetcode (result);
			return nullptr;
		}

		pathPage[i - 1] = 0;
	}

	return path[height()-1];
}

inline AttrType IndexHandle::attrType ( ) const {
	return header.attrType;
}

inline size_t IndexHandle::attrLen( )const {
	return header.attrLength;
}

inline size_t IndexHandle::numMaxKeys ( ) const {
	return header.numMaxKeys;
}

inline size_t IndexHandle::numPages ( ) const {
	return header.numPages;
}

inline size_t IndexHandle::height ( ) const {
	return header.height;
}



inline BpTreeNodePtr IndexHandle::FindLeaf (void * pData) {
	if ( root == nullptr )
		return nullptr;

	if ( height() == 1 ) {
		path[0] = root;
		return root;
	}

	PageNum page;
	RETCODE result;
	PagePtr pagePtr;

	//path[0] = root;
	//pathPage[0] = roo

	for ( size_t i = 1; i < height ( ); i++ ) {
		RecordIdentifier r = path[i - 1]->GetRid (pData);
		size_t pos = path[i - 1]->FindKeyPosFit (pData);

		if ( result = r.GetPageNum (page) ) {
			Utils::PrintRetcode (result);
			return nullptr;
		}
		
		if ( page == Utils::UNKNOWNPAGENUM ) {
			void * p = path[i - 1]->LargestKey ( );
			r = path[i - 1]->GetRid (p);
			pos = path[i - 1]->FindKey (p);
		}

		// if start with a new page
		if ( i < path.size ( ) ) {
			if ( result = bufMgr->UnlockPage (path[i]->GetPageNum ( )) ) {
				Utils::PrintRetcode (result);
				return nullptr;
			}
			path[i] = nullptr;
		}

		path[i] = FetchNode (page);
		
		// lock page
		PagePtr tmp;
		if ( result = bufMgr->GetPage (page, tmp) ) {
			Utils::PrintRetcode (result);
			return nullptr;
		}
		pathPage[i - 1] = pos;
	}

	return path[height()-1];
}

inline RETCODE IndexHandle::GetThisPage (PageNum page, PagePtr & pagePtr) {
	RETCODE result;

	if ( result = bufMgr->GetPage (page, pagePtr) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	if ( result = bufMgr->MarkDirty (page) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	//bufMgr->UnlockPage (page);

	return RETCODE::COMPLETE;
}

inline RETCODE IndexHandle::GetNewPage (PagePtr & page) {
	RETCODE result;

	if ( result = bufMgr->AllocatePage (page) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	header.numPages++;
	headerModified = true;

	return RETCODE::COMPLETE;
}

/*
	Update height and allocate space for path and pathPages

*/
inline RETCODE IndexHandle::SetHeight (size_t h) {
	path.clear ( );
	pathPage.clear ( );

	header.height = h;
	
	path.push_back (root);

	/*
		Initialize path with size height() and pathPage with size height()-1
	*/
	for ( size_t i = 1; i < height ( ); i++ ) {
		path.push_back (nullptr);
		pathPage.push_back (Utils::UNKNOWNPOS);
	}

	return RETCODE::COMPLETE;
}

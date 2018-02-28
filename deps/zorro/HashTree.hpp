#ifndef __HASHTREE_HPP__
#define __HASHTREE_HPP__

/* $Id: HashTree.hpp,v 1.3 2014/02/28 08:55:04 skv Exp $
Author: Konstantin Stupnik konstantin.stupnik@gmail.com
*/

#ifdef __SunOS_5_9
#include <inttypes.h>
#else
#include <stdint.h>
#endif
#include <memory.h>
#include <string>
#include <stdexcept>


struct HTDefaultHashFunc{
  uint32_t operator()(const std::string& a_str)
  {
    unsigned char* ptr=(unsigned char*)a_str.c_str();
    uint32_t rv=0x7654321**ptr;
    while(*ptr)
    {
      rv+=37*rv+*ptr;
      ptr++;
    }
    return rv;
  }
  uint32_t operator()(uint32_t a_val)
  {
    return a_val;
  }
  uint32_t operator()(uint64_t a_val)
  {
    return (uint32_t)a_val;
  }
};

template <class K,class V>
struct KeyVal{
   const K m_key;
   V m_value;
   KeyVal(){}
   KeyVal(const K& a_key,const V& a_value):m_key(a_key),m_value(a_value){}
   const K& key()const
   {
      return m_key;
   }
   V& value()
   {
      return m_value;
   }
   const V& value()const
   {
      return m_value;
   }
};

template <class NodeType,size_t POOL_PAGE_SIZE=256>
struct HTPooledAllocator{
   struct PoolPage{
      PoolPage():m_current(m_nodes),m_nextPage(0){}
      NodeType m_nodes[POOL_PAGE_SIZE];
      NodeType* m_current;
      PoolPage* m_nextPage;
   };
   PoolPage m_firstPage;
   PoolPage* m_currentPage;
   NodeType* m_freeNodes;
   HTPooledAllocator():m_currentPage(&m_firstPage),m_freeNodes(0){}
   ~HTPooledAllocator()
   {
      PoolPage* ptr=m_firstPage.m_nextPage;
      PoolPage* optr;
      while(ptr)
      {
         optr=ptr;
         ptr=ptr->m_nextPage;
         delete optr;
      }
   }
   NodeType* newNode()
   {
      if(m_freeNodes)
      {
         NodeType* rv=m_freeNodes;
         m_freeNodes=m_freeNodes->m_next;
         return rv;
      }
      if(m_currentPage->m_current==m_currentPage->m_nodes+POOL_PAGE_SIZE)
      {
         m_currentPage=m_currentPage->m_nextPage=new PoolPage;
      }
      return m_currentPage->m_current++;
   }
   void freeNode(NodeType* a_ptr)
   {
      a_ptr->m_next=m_freeNodes;
      m_freeNodes=a_ptr;
   }
};

template <class NodeType,size_t N>
class HTHeapAllocator{
public:
  NodeType* newNode()
  {
    return new NodeType;
  }
  void freeNode(NodeType* a_ptr)
  {
    delete a_ptr;
  }
};


template <class K,class V,typename HF=HTDefaultHashFunc,size_t PoolSz=256,template <class NT,size_t SZ=PoolSz> class NodesAllocator=HTPooledAllocator>
class HashTree{
protected:
   struct Node;
   struct DataNode;
   struct DataArrayNode;
public:

   typedef KeyVal<K,V> value_type;

   explicit HashTree(const HF& a_hashFunc=HF()):
       m_root(0),m_count(0),m_hashFunc(a_hashFunc),m_begin((DataNode*)&m_end),m_npool(0),m_dpool(0),m_apool(0),m_externalAllocators(false)
   {
      m_end.m_prev=0;
      m_end.m_next=0;
   }

   typedef NodesAllocator<Node> NodeAlloc;
   typedef NodesAllocator<DataNode> DataNodeAlloc;
   typedef NodesAllocator<DataArrayNode> ArrayNodeAlloc;

   void assignAllocators(NodeAlloc* argNA,DataNodeAlloc* argDA,ArrayNodeAlloc* argAN)
   {
     m_npool=argNA;
     m_dpool=argDA;
     m_apool=argAN;
     m_externalAllocators=true;
   }

   HashTree(const HashTree& a_other):m_root(0),m_count(0),m_hashFunc(a_other.m_hashFunc),m_begin(&m_end),
       m_npool(0),m_dpool(0),m_apool(0),m_externalAllocators(false)
   {
      for(const_iterator it=a_other.begin(),end=a_other.end();it!=end;++it)
      {
         insert(it->m_key,it->m_value);
      }
   }

   HashTree& operator=(const HashTree& a_other)
   {
      if(this==&a_other)
      {
         return *this;
      }
      clear();
      for(const_iterator it=a_other.begin(),end=a_other.end();it!=end;++it)
      {
         insert(it->m_key,it->m_value);
      }
      return *this;
   }

   template <size_t DefSz2,template <class NT2,size_t SZ2=DefSz2> class NodesAllocator2>
   HashTree& operator=(const HashTree<K,V,HF,DefSz2,NodesAllocator2>& a_other)
   {
      clear();
      for(typename HashTree<K,V,HF,DefSz2,NodesAllocator2>::const_iterator it=a_other.begin(),end=a_other.end();it!=end;++it)
      {
         insert(it->m_key,it->m_value);
      }
      return *this;
   }

   bool operator==(const HashTree& a_other)const
   {
      if(this==&a_other)
      {
         return true;
      }
      const_iterator it1=begin(),end1=end(),it2=a_other.begin(),end2=a_other.end();
      for(;;++it1,++it2)
      {
         if(it1==end1)
         {
            if(it2==end2)
            {
               return true;
            }
            return false;
         }
         if(it2==end2)
         {
            return false;
         }
         if(!(it1->m_key==it2->m_key) || !(it1->m_value==it2->m_value))
         {
            return false;
         }
      }
      return false;
   }
   template <size_t DefSz2,template <class NT2,size_t SZ2=DefSz2> class NodesAllocator2>
   bool operator==(const HashTree<K,V,HF,DefSz2,NodesAllocator2>& a_other)const
   {
      const_iterator it1=begin(),end1=end();
      typename HashTree<K,V,HF,DefSz2,NodesAllocator2>::const_iterator it2=a_other.begin(),end2=a_other.end();
      for(;;++it1,++it2)
      {
         if(it1==end1)
         {
            if(it2==end2)
            {
               return true;
            }
            return false;
         }
         if(it2==end2)
         {
            return false;
         }
         if(!(it1->m_key==it2->m_key) || !(it1->m_value==it2->m_value))
         {
            return false;
         }
      }
      return false;
   }

   bool operator!=(const HashTree& a_other)const
   {
      return !operator==(a_other);
   }

   template <size_t DefSz2,template <class NT2,size_t SZ2=DefSz2> class NodesAllocator2>
   bool operator!=(const HashTree<K,V,HF,DefSz2,NodesAllocator2>& a_other)const
   {
     return !operator==(a_other);
   }


   ~HashTree()
   {
      clear(true);
   }

   class iterator{
   public:
      iterator():m_node(0){}
      iterator operator++(int)
      {
         m_node=m_node->m_next;
         return *this;
      }

      iterator operator++()
      {
         DataNode* rv=m_node;
         m_node=m_node->m_next;
         return iterator(rv);
      }

      iterator operator--(int)
      {
         m_node=m_node->m_prev;
         return *this;
      }

      iterator operator--()
      {
         DataNode* rv=m_node;
         m_node=m_node->m_prev;
         return iterator(rv);
      }

      iterator& operator=(const iterator& a_other)
      {
         m_node=a_other.m_node;
         return *this;
      }

      bool operator==(const iterator& a_other)const
      {
         return m_node==a_other.m_node;
      }

      bool operator!=(const iterator& a_other)const
      {
         return m_node!=a_other.m_node;
      }

      value_type& operator*()
      {
         return m_node->keyval();
      }
      value_type* operator->()
      {
         return &m_node->keyval();
      }
      const value_type& operator*()const
      {
         return m_node->keyval();
      }
      const value_type* operator->()const
      {
         return *m_node->keyval();
      }
   protected:
      iterator(DataNode* a_node):m_node(a_node){}
      friend class HashTree;
      typename HashTree::DataNode* m_node;
      friend class const_iterator;
   };

   class const_iterator{
   public:
      const_iterator():m_node(0){}
      const_iterator(const iterator& a_other):m_node(a_other.m_node){}
      const_iterator operator++(int)
      {
         m_node=m_node->m_next;
         return *this;
      }

      const_iterator operator++()
      {
         const DataNode* rv=m_node;
         m_node=m_node->m_next;
         return const_iterator(rv);
      }

      const_iterator operator--(int)const
      {
         m_node=m_node->m_prev;
         return *this;
      }

      const_iterator operator--()
      {
         const DataNode* rv=m_node;
         m_node=m_node->m_prev;
         return const_iterator(rv);
      }

      const_iterator& operator=(const const_iterator& a_other)
      {
         m_node=a_other.m_node;
         return *this;
      }

      bool operator==(const const_iterator& a_other)const
      {
         return m_node==a_other.m_node;
      }

      bool operator!=(const const_iterator& a_other)const
      {
         return m_node!=a_other.m_node;
      }

      const value_type& operator*()const
      {
         return m_node->keyval();
      }
      const value_type* operator->()const
      {
         return &m_node->keyval();
      }
   protected:
      const_iterator(const DataNode* a_node):m_node(a_node){}
      friend class HashTree;
      const typename HashTree::DataNode* m_node;
   };


   iterator begin()
   {
      return iterator(m_begin);
   }

   iterator end()
   {
      return iterator((DataNode*)&m_end);
   }

   const_iterator begin()const
   {
      return const_iterator(m_begin);
   }

   const_iterator end()const
   {
      return const_iterator((const DataNode*)&m_end);
   }

   iterator insert(const K& a_key,const V& a_value)
   {
      return insert(a_key,a_value,true);
   }

   V& operator[](const K& a_key)
   {
      iterator it=insert(a_key,V(),false);
      return it->m_value;
   }

   const V& operator[](const K& a_key)const
   {
      iterator it=find(a_key);
      if(it==end())
      {
        throw std::runtime_error("const HashTree::operator[] on non existent key");
      }
      return it->m_value;
   }

   iterator insert(const value_type& a_keyval)
   {
      return insert(a_keyval.m_key,a_keyval.m_value);
   }

   template <class InputIterator>
   void insert(InputIterator it,InputIterator end)
   {
     for(;it!=end;++it)
     {
       insert(*it);
     }
   }

   iterator find(const K& a_key)
   {
      if(!m_root)
      {
         return end();
      }
      uint32_t hashCode=m_hashFunc(a_key);
      Node* ptr=m_root;
      while(ptr)
      {
         if(ptr->m_nodeType==ntInnerNode)
         {
           ptr=ptr->m_ch[hashCode&3];
           hashCode>>=2;
           continue;
         }
         if(ptr->m_nodeType==ntValuesArray)
         {
            DataArrayNode& va=ptr->asArray();
            for(size_t i=0;i<va.m_count;i++)
            {
               if(va.m_array[i]->keyval().m_key==a_key)
               {
                  return iterator(va.m_array[i]);
               }
            }
            return end();
         }
         if(ptr->asData().keyval().m_key==a_key)
         {
            return iterator(ptr->asDataPtr());
         }
         return end();
      }
      return end();
   }

   const_iterator find(const K& a_key)const
   {
      if(!m_root)
      {
         return end();
      }
      uint32_t hashCode=m_hashFunc(a_key);
      Node* ptr=m_root;
      while(ptr)
      {
         if(ptr->m_nodeType==ntInnerNode)
         {
           ptr=ptr->m_ch[hashCode&3];
           hashCode>>=2;
           continue;
         }
         if(ptr->m_nodeType==ntValuesArray)
         {
            DataArrayNode& va=ptr->asArray();
            for(size_t i=0;i<va.m_count;i++)
            {
               if(va.m_array[i]->keyval().m_key==a_key)
               {
                  return const_iterator(va.m_array[i]);
               }
            }
            return end();
         }
         if(ptr->asData().keyval().m_key==a_key)
         {
            return const_iterator(ptr->asDataPtr());
         }
         return end();
      }
      return end();
   }


   bool find(const K& a_key,V& a_valueRef)
   {
      iterator it=find(a_key);
      if(it!=end())
      {
         a_valueRef=it->m_value;
         return true;
      }
      return false;
   }

   void erase(const K& a_key)
   {
      if(!m_root)
      {
         return;
      }
      uint32_t hashCode=m_hashFunc(a_key);
      Node* ptr=m_root;
      Node* path[16];
      unsigned char pathIdx[16];
      size_t pathCount=0;
      while(ptr)
      {
         unsigned char idx=hashCode&3;
         if(ptr->m_nodeType==ntInnerNode)
         {
            path[pathCount]=ptr;
            pathIdx[pathCount++]=idx;
            ptr=ptr->m_ch[idx];
            hashCode>>=2;
            continue;
         }
         if(ptr->m_nodeType==ntValuesArray)
         {
            DataArrayNode& va=ptr->asArray();
            bool found=false;
            for(size_t i=0;i<va.m_count;i++)
            {
               if(va.m_array[i]->keyval().m_key==a_key)
               {
                  found=true;
                  DataNode* vptr=va.m_array[i];
                  if(vptr->m_prev)
                  {
                     vptr->m_prev->m_next=vptr->m_next;
                     vptr->m_next->m_prev=vptr->m_prev;
                  }else
                  {
                     m_begin=vptr->m_next;
                     m_begin->m_prev=0;
                  }
                  vptr->destroyKeyVal();
                  m_dpool->freeNode(vptr);
                  if(va.m_count>1)
                  {
                     DataNode** newValues=new DataNode*[va.m_count-1];
                     if(i>0)
                     {
                        memcpy(newValues,va.m_array,i*sizeof(DataNode*));
                     }
                     if(i<va.m_count-1)
                     {
                        memcpy(newValues+i,va.m_array+i+1,(va.m_count-i-1)*sizeof(DataNode*));
                     }
                     delete [] va.m_array;
                     va.m_array=newValues;
                     va.m_count--;
                     m_count--;
                     return;
                  }else
                  {
                     delete [] va.m_array;
                     m_apool->freeNode(ptr->asArrayPtr());
                     m_count--;
                     break;
                  }
               }
            }
            if(!found)
            {
               return;
            }
         }else
         {
            DataNode* dptr=ptr->asDataPtr();
            if(!(dptr->keyval().m_key==a_key))
            {
               return;
            }
            if(dptr->m_prev)
            {
               dptr->m_prev->m_next=dptr->m_next;
               dptr->m_next->m_prev=dptr->m_prev;
            }else
            {
               m_begin=dptr->m_next;
               m_begin->m_prev=0;
            }
            dptr->destroyKeyVal();
            m_dpool->freeNode(dptr);
            m_count--;
         }

         while(pathCount>0)
         {
            ptr=path[pathCount-1];
            idx=pathIdx[pathCount-1];
            ptr->m_ch[idx]=0;
            if(ptr->m_ch[0] || ptr->m_ch[1] || ptr->m_ch[2] || ptr->m_ch[3])
            {
               return;
            }
            m_npool->freeNode(ptr);
            if(ptr==m_root)
            {
               m_root=0;
            }
            pathCount--;
         }
         return;
      }
   }

   void erase(iterator a_it)
   {
      erase(a_it.m_node->keyval().m_key);
   }

   void erase(const_iterator a_it)
   {
      erase(a_it.m_node->keyval().m_key);
   }

   size_t size()const
   {
      return m_count;
   }

   bool empty()const
   {
      return !m_count;
   }

   void clear(bool a_freeMemPool=false)
   {
      if(m_root)
      {
         recClear(m_root);
      }
      m_root=0;
      m_count=0;
      m_begin=(DataNode*)&m_end;
      m_end.m_prev=0;
      if(a_freeMemPool && m_npool && !m_externalAllocators)
      {
         delete m_npool;
         delete m_dpool;
         delete m_apool;
         m_npool=0;
         m_dpool=0;
         m_apool=0;
      }
   }

   void swap(HashTree& a_other)
   {
      Node* tmpRoot=m_root;
      m_root=a_other.m_root;
      a_other.m_root=tmpRoot;
      size_t tmpCount=m_count;
      m_count=a_other.m_count;
      a_other.m_count=tmpCount;
      DataNode* tmpBegin=m_begin;
      m_begin=a_other.m_begin;
      a_other.m_begin=tmpBegin;

      NodesAllocator<Node>* tmpNPool=m_npool;
      m_npool=a_other.m_npool;
      a_other.m_npool=tmpNPool;

      NodesAllocator<DataNode>* tmpDPool=m_dpool;
      m_dpool=a_other.m_dpool;
      a_other.m_dpool=tmpDPool;

      NodesAllocator<DataArrayNode>* tmpAPool=m_apool;
      m_apool=a_other.m_apool;
      a_other.m_apool=tmpAPool;

      bool tmpExt=m_externalAllocators;
      m_externalAllocators=a_other.m_externalAllocators;
      a_other.m_externalAllocators=tmpExt;


      if(m_end.m_prev)
      {
         m_end.m_prev->m_next=(DataNode*)&a_other.m_end;
      }
      if(a_other.m_end.m_prev)
      {
        a_other.m_end.m_prev->m_next=(DataNode*)&m_end;
      }
      DataNode* tmpEnd=m_end.m_prev;
      m_end.m_prev=a_other.m_end.m_prev;
      a_other.m_end.m_prev=tmpEnd;
   }

protected:

   iterator insert(const K& a_key,const V& a_value,bool overwrite)
   {
      uint32_t hashCode=m_hashFunc(a_key);
      size_t bitIndex=0;
      if(!m_root)
      {
         if(!m_npool)
         {
            m_npool=new NodesAllocator<Node>;
            m_dpool=new NodesAllocator<DataNode>;
            m_apool=new NodesAllocator<DataArrayNode>;
         }
         m_root=m_npool->newNode();
         m_root->m_nodeType=ntInnerNode;
         m_root->m_ch[3]=m_root->m_ch[2]=m_root->m_ch[1]=m_root->m_ch[0]=0;
      }
      Node* ptr=m_root;
      Node* parent=0;
      unsigned char parentIdx=0;
      unsigned char idx;
      for(;;)
      {
         idx=hashCode&3;
         if(ptr->m_nodeType==ntInnerNode)
         {
           if(ptr->m_ch[idx])
           {
             parent=ptr;
             parentIdx=idx;
             ptr=ptr->m_ch[idx];
             hashCode>>=2;
             bitIndex+=2;
             continue;
           }else
           {
             DataNode& n=*(DataNode*)(ptr->m_ch[idx]=(Node*)m_dpool->newNode());
             n.constructKeyVal(value_type(a_key,a_value));
             n.m_next=(DataNode*)&m_end;
             n.m_prev=m_end.m_prev;
             if(m_end.m_prev)
             {
               m_end.m_prev->m_next=&n;
             }else
             {
               m_begin=&n;
             }
             m_end.m_prev=&n;
             m_count++;
             return iterator(&n);
           }
         }

         if(ptr->m_nodeType==ntValuesArray)
         {
            DataArrayNode& va=ptr->asArray();
            for(size_t i=0;i<va.m_count;i++)
            {
               if(va.m_array[i]->keyval().m_key==a_key)
               {
                  if(overwrite)
                  {
                     va.m_array[i]->keyval().m_value=a_value;
                  }
                  return iterator(va.m_array[i]);
               }
            }
            DataNode** newArr=new DataNode*[va.m_count+1];
            memcpy(newArr,va.m_array,sizeof(DataNode*)*va.m_count);
            delete [] va.m_array;
            va.m_array=newArr;
            DataNode& n=*m_dpool->newNode();
            n.constructKeyVal(value_type(a_key,a_value));
            n.m_next=(DataNode*)&m_end;
            n.m_prev=m_end.m_prev;
            if(m_end.m_prev)
            {
               m_end.m_prev->m_next=&n;
            }else
            {
               m_begin=&n;
            }
            m_end.m_prev=&n;
            va.m_array[va.m_count++]=&n;
            m_count++;
            return iterator(&n);
         }else
         {
            DataNode* dptr=ptr->asDataPtr();
            if(dptr->keyval().m_key==a_key)
            {
               if(overwrite)
               {
                  dptr->keyval().m_value=a_value;
               }
               return iterator(dptr);
            }
            if(bitIndex!=30)
            {
               uint32_t oldHashCode=m_hashFunc(dptr->keyval().m_key);
               oldHashCode>>=bitIndex;
               do
               {
                  Node* newNode=parent->m_ch[parentIdx]=m_npool->newNode();
                  newNode->m_nodeType=ntInnerNode;
                  newNode->m_ch[3]=newNode->m_ch[2]=newNode->m_ch[1]=newNode->m_ch[0]=0;
                  if((oldHashCode&3)!=idx)
                  {
                     newNode->m_ch[oldHashCode&3]=ptr;
                     ptr=newNode;
                     DataNode& n=*(DataNode*)(ptr->m_ch[idx]=(Node*)(dptr=m_dpool->newNode()));
                     n.constructKeyVal(value_type(a_key,a_value));
                     n.m_next=(DataNode*)&m_end;
                     n.m_prev=m_end.m_prev;
                     if(m_end.m_prev)
                     {
                        m_end.m_prev->m_next=dptr;
                     }else
                     {
                        m_begin=dptr;
                     }
                     m_end.m_prev=dptr;
                     m_count++;
                     return iterator(dptr);
                  }
                  parent=newNode;
                  parentIdx=idx;
                  hashCode>>=2;
                  oldHashCode>>=2;
                  bitIndex+=2;
                  idx=hashCode&3;
               }while(bitIndex<30);
            }

            DataArrayNode* newNode=(DataArrayNode*)(parent->m_ch[parentIdx]=(Node*)m_apool->newNode());
            newNode->m_nodeType=ntValuesArray;
            newNode->m_count=2;
            newNode->m_array=new DataNode*[2];
            newNode->m_array[0]=dptr;
            DataNode& n=*(newNode->m_array[1]=m_dpool->newNode());
            n.constructKeyVal(value_type(a_key,a_value));
            n.m_next=(DataNode*)&m_end;
            n.m_prev=m_end.m_prev;
            if(m_end.m_prev)
            {
               m_end.m_prev->m_next=&n;
            }else
            {
               m_begin=&n;
            }
            m_end.m_prev=&n;
            m_count++;
            return iterator(&n);
         }
      }
      return end();
   }

   friend class iterator;
   friend class const_iterator;

   static const intptr_t ntInnerNode=intptr_t(-1);//0xffffffff,
   static const intptr_t ntValuesArray=intptr_t(-2);//0xfffffffe

   struct DataNode;
   struct DataNodeBase{
     DataNode* m_prev;
     DataNode* m_next;
   };
   struct DataNode:DataNodeBase{
      char m_keyValBuf[sizeof(KeyVal<K,V>)];
      void constructKeyVal(const KeyVal<K,V>& a_kv)
      {
        new(m_keyValBuf)KeyVal<K,V>(a_kv);
      }
      void destroyKeyVal()
      {
        keyval().~KeyVal<K,V>();
      }
      KeyVal<K,V>& keyval()
      {
        return *((KeyVal<K,V>*)(m_keyValBuf));
      }
      const KeyVal<K,V>& keyval()const
      {
        return *((const KeyVal<K,V>*)(m_keyValBuf));
      }
   };

   struct DataArrayNode{
      intptr_t m_nodeType;
      union{
        DataNode** m_array;
        DataArrayNode* m_next;
      };
      size_t m_count;
   };


   struct Node{
      intptr_t m_nodeType;
      union{
         Node* m_next;
         Node* m_ch[4];
      };
      DataNode& asData()
      {
        return *(DataNode*)this;
      }
      DataNode* asDataPtr()
      {
        return (DataNode*)this;
      }
      DataArrayNode& asArray()
      {
        return *(DataArrayNode*)this;
      }
      DataArrayNode* asArrayPtr()
      {
        return (DataArrayNode*)this;
      }
   };

   Node* m_root;
   size_t m_count;
   HF m_hashFunc;
   DataNodeBase m_end;
   DataNode* m_begin;

   NodesAllocator<Node>* m_npool;
   NodesAllocator<DataNode>* m_dpool;
   NodesAllocator<DataArrayNode>* m_apool;
   bool m_externalAllocators;

   void recClear(Node* a_ptr)
   {
      if(a_ptr->m_nodeType==ntInnerNode)
      {
         if(a_ptr->m_ch[0])recClear(a_ptr->m_ch[0]);
         if(a_ptr->m_ch[1])recClear(a_ptr->m_ch[1]);
         if(a_ptr->m_ch[2])recClear(a_ptr->m_ch[2]);
         if(a_ptr->m_ch[3])recClear(a_ptr->m_ch[3]);
         m_npool->freeNode(a_ptr);
         return;
      }else if(a_ptr->m_nodeType==ntValuesArray)
      {
         DataArrayNode& va=a_ptr->asArray();
         for(size_t i=0;i<va.m_count;i++)
         {
            va.m_array[i]->destroyKeyVal();
            m_dpool->freeNode(va.m_array[i]);
         }
         delete [] va.m_array;
         m_apool->freeNode(&va);
         return;
      }else
      {
         a_ptr->asData().destroyKeyVal();
         m_dpool->freeNode(a_ptr->asDataPtr());
      }
   }
};

template <class K,class V,typename HF,size_t PoolSz,template <class NT,size_t SZ> class NodesAllocator>
const intptr_t HashTree<K,V,HF,PoolSz,NodesAllocator>::ntInnerNode;
template <class K,class V,typename HF,size_t PoolSz,template <class NT,size_t SZ> class NodesAllocator>
const intptr_t HashTree<K,V,HF,PoolSz,NodesAllocator>::ntValuesArray;


#endif

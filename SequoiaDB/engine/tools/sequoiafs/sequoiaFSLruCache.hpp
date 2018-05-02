#ifndef __SEQUOIAFSLRUCACHE_HPP__
#define __SEQUOIAFSLRUCACHE_HPP__

#include "sequoiaFS.hpp"
#include <iostream>
#include <map>

namespace sequoiafs
{
    class Node {
      public:
      int key;
      struct dirMetaNode node;
      Node *prev, *next;
      Node(int k, const struct dirMetaNode* v): key(k), prev(NULL), next(NULL) 
      {
        node.name= v->name;
        node.mode= v->mode;
        node.uid = v->uid;
        node.gid = v->gid;
        node.pid= v->pid;
        node.nLink = v->nLink;
        node.id= v->id;
        node.size= v->size;
        node.atime= v->atime;
        node.ctime= v->ctime;
        node.mtime= v->mtime;
        node.symLink= v->symLink;
      }
    };


    class DoublyLinkedList 
    {
        Node *front, *rear;
        
        bool isEmpty() 
        {
          return rear == NULL;
        }

        public:
        DoublyLinkedList(): front(NULL), rear(NULL){}
        Node* getRearDir() 
        {
            return rear;
        }
        void removeRearDir();
        void moveDirToHead(Node *Dir); 
        Node* addDirToHead(int key,const struct dirMetaNode* value);   
    };

    class LRUCache
    {
        int capacity, size;
        DoublyLinkedList *dirList;
        map<int, Node*> DirMap;
        
        pthread_mutex_t mutex;
        pthread_mutexattr_t attr;

        public:
        LRUCache(int capacity) 
        {
            this->capacity = capacity;
            size = 0;
            dirList = new DoublyLinkedList();
            DirMap = map<int, Node*>();
        }

        ~LRUCache() 
        {
            map<int, Node*>::iterator i1;
            for(i1=DirMap.begin();i1!=DirMap.end();i1++) 
            {
                delete i1->second;
            }
            delete dirList;
        }
        int initMutex();
        void put(int key,const struct dirMetaNode * value);
        struct dirMetaNode * get(int key) ;
    };

}

#endif

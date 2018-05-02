#include "sequoiaFSLruCache.hpp"

using namespace sequoiafs;
Node* DoublyLinkedList::addDirToHead(int key, const struct dirMetaNode * value) 
{
    Node *Dir = new Node(key, value);
    if(!front && !rear)
    {
        front = rear = Dir;
    }
    else 
    {
        Dir->next = front;
        front->prev = Dir;
        front = Dir;
    }
    return Dir;
}

void DoublyLinkedList::moveDirToHead(Node *Dir) 
{
    if(Dir==front) 
    {
        return;
    }
    if(Dir == rear) 
    {
        rear = rear->prev;
        rear->next = NULL;
    }
    else 
    {
        Dir->prev->next = Dir->next;
        Dir->next->prev = Dir->prev;
    }

    Dir->next = front;
    Dir->prev = NULL;
    front->prev = Dir;
    front = Dir;
}

void DoublyLinkedList::removeRearDir() 
{
    if(isEmpty()) 
    {
        return;
    }
    if(front == rear) 
    {
        delete rear;
        front = rear = NULL;
    }
    else 
    {
        Node *temp = rear;
        rear = rear->prev;
        rear->next = NULL;
        delete temp;
    }
}  


struct dirMetaNode * LRUCache::get(int key) 
{
    if(DirMap.find(key)==DirMap.end()) 
    {
        return NULL;
    }
    struct dirMetaNode *node = &DirMap[key]->node;

    // move the page to front
    dirList->moveDirToHead(DirMap[key]);
    return node;
}

int LRUCache::initMutex()
{
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    if(0!=pthread_mutex_init(&mutex, &attr))
    {
        printf("Init pthread mutex failed.\n");
        return -1;
    }
    
    return 0;
}

void LRUCache::put(int key, const struct dirMetaNode *value) 
{
    if(DirMap.find(key)!=DirMap.end()) 
    {     
        DirMap[key]->node.name = value->name;
        DirMap[key]->node.mode = value->mode;
        DirMap[key]->node.uid = value->uid;     
        DirMap[key]->node.gid = value->gid;
        DirMap[key]->node.pid = value->pid;
        DirMap[key]->node.id = value->id;
        DirMap[key]->node.nLink = value->nLink;
        DirMap[key]->node.size = value->size;    
        DirMap[key]->node.ctime = value->ctime;
        DirMap[key]->node.atime = value->atime;
        DirMap[key]->node.mtime = value->mtime; 
        DirMap[key]->node.symLink = value->symLink;
        dirList->moveDirToHead(DirMap[key]);
        return;
    }

    if(size == capacity) 
    {
        // remove rear page
        int k = dirList->getRearDir()->key;
        DirMap.erase(k);
        dirList->removeRearDir();
        size--;
    }

    // add new page to head to Queue
    Node *Dir = dirList->addDirToHead(key, value);
    size++;
    DirMap[key] = Dir;
}



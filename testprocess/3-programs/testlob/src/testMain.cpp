#include <iostream>
#include "mongo/client/dbclient.h"

using namespace mongo;
using namespace std;

void run()
{
   DBClientConnection c ;
   c.connect("localhost:27017") ;
   string dbName = "shardtest" ;
   string filename = "hadoop" ;
   string outfile = "out.mongo" ;
   string infile = "hadoop-2.2.0.tar.gz" ;
   string remoteName = "hadoop-2.2.0" ;
   GridFS* gridfs = new GridFS(c, dbName) ;

   //list file info
  auto_ptr<DBClientCursor> cursor = gridfs->list( ) ;

   while(cursor->more())
   {
      BSONObj p = cursor->next() ;
      cout << p << endl ;
   }

   //put file to mongo
   BSONObj put = gridfs->storeFile(infile, remoteName, "tar.gz") ;
   cout << "put file:" << put << endl ;
   
/* 
   //get file from mongo
   GridFile gfile = gridfs->findFile(filename) ;
   cout << "FileName: " << gfile.getFilename() << endl ;
   cout << "ChunkSize: " << gfile.getChunkSize() << endl ;
   cout << "ContentLength: " << gfile.getContentLength() << endl ;
   cout << "ContentType: " << gfile.getContentType() << endl ;
   cout << "UploadDate: " << gfile.getUploadDate() << endl ;
   cout << "MD5: " << gfile.getMD5() << endl ;

   cout << "begin to write to " << outfile << endl ;
   gridfs_offset tmp = gfile.write(outfile) ;
   cout << "write offset:" << tmp << endl ;
   */
 
/*
   auto_ptr<DBClientCursor> cursor = c.query("shardtest.people", BSONObj()) ;
   while(cursor->more())
   {
      BSONObj p = cursor->next() ;
      cout << p << endl ;
   }
 */
}

int main()
{
   try{
      run() ;
      cout << "connected ok!" << endl ;
   }catch ( DBException &e ) {
      cout << "caught " << e.what() << endl ;
   }

   return 0 ;
      
}


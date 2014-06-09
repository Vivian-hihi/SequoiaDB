#include "../../client/jstobs.h"
#include "../../client/bson/bson.h"

void getBSONRaw( const CHAR *js, CHAR **raw )
{
   bson bs ;
   if (jsonToBson( &bs, js ) )
   {
      bs.ownmem = FALSE ;
      *raw = bs.data ;
   }
}

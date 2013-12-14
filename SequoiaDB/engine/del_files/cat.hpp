#ifndef CAT_HPP__
#define CAT_HPP__
#include "core.hpp"
namespace engine
{
#define CAT_COLLECTION_CATALOG_CSNAME "SYSCATALOG"
#define CAT_COLLECTION_CATALOG_COLLECTION \
CAT_COLLECTION_CATALOG_CSNAME ".COLLECTION"
#define CAT_COLLECTION_CATALOG_NETWORK \
CAT_COLLECTION_CATALOG_CSNAME ".NETWORK"
// heart beat every 3 seconds
#define CAT_HEARTBEAT_RATE 3
// node is considered offline after missing 3 heartbeat
#define CAT_HEARTBEAT_WINDOW 3
// node timeout time is rate * window
#define CAT_HEARTBEAT_TIMEOUT \
   CAT_HEARTBEAT_RATE * CAT_HEARTBEAT_WINDOW
}

#endif

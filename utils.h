#ifndef _UTILS_H_
#define _UTILS_H_

#include "map.h"

typedef int chessId;

#define RETURN_NULL_ON_NULL(arg)  \
  if (NULL == arg) {              \
    return NULL;                  \
  }

#define RETURN_RESULT_ON_NULL(arg)  \
  if (NULL == arg) {                \
    return CHESS_NULL_ARGUMENT;     \
  }

#define RETURN_ZERO_ON_NULL(arg)  \
  if (arg == 0) {                \
    return 0;     \
  }

// both argument are NULL -> they are "same" 
// one of them is NULL -> the other is greater
#define COMPARE_NOT_NULL(first, second) \
    if(NULL == first) {                \
    if (NULL == second) {               \
      return 0;                         \
    } else {                            \
      return -1;                        \
    }                                   \
  } else {                              \
    if (NULL == second) {               \
      return 1;                         \
    }                                   \
  }

#define MATCHNODE_FOREACH(list) \
      for(;list;matchNodeNext(list))

#define IF_MAP_PUT(map, key, data)                          \
  if (MAP_OUT_OF_MEMORY == mapPut(map,                      \
                                  (MapKeyElement)key,       \
                                  (MapDataElement)data))    

#define MAP_GET(map, key, type)         \
   (type)mapGet(map,(MapKeyElement)key)      
/**
 * Ids are integers. Copies the key to a new integer.
 * 
 * @param element Key to be copied
 * @return New copy of element
 */
MapKeyElement copyId(MapKeyElement element);

/**
 * Frees ID keys (integers)
 * 
 * @param element key to be freed
 */
void freeId(MapKeyElement element);

/**
 * Confirms that provided tournament's or player's ID are in compliance with
 * the specification: > 0
 * 
 * @param id ID to validate
 * @return true ID is valid
 * @return false ID is not valid
 */
bool validateId(chessId id);

/**
 * Validates that the provided location string is in compliance 
 * with the location demands:
 *    Not empty
 *    Starts with a capital letter
 *    All following characters are small letters and spaces
 * 
 * @param location the location string
 * @return true location is valid
 * @return false location is invalid
 */
bool validateLocation(const char *location);

/**
 * Compares between IDs. The bigger ID is consider "greater"
 * 
 * @param element1 first ID to compare
 * @param element2 second ID to compare
 * @return >0 first ID is greater; 
 *         <0 second ID is greater;
 *         0 IDs are identical
 */
int idCompare(MapKeyElement element1, MapKeyElement element2);

#endif /* _UTILS_H_ */
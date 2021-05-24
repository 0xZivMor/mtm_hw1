#include "utils.h"
#include "map.h"
#include <string.h>
#include <stdlib.h>

struct node_t {
  void *value;
  struct node_t *next;
};

MapKeyElement copyId(MapKeyElement element)
{
  ChessId *copy = (ChessId*)malloc(sizeof(ChessId));
  RETURN_NULL_ON_NULL(copy)

  *copy = *((ChessId *)element);
  return (MapKeyElement)copy;
}

void freeId(MapKeyElement element)
{
  if (NULL == element) {
    return;
  }

  free(element);
}

bool validateId(ChessId id)
{
  if (id <= 0) {
    return false;
  }
  return true;
}

bool validateLocation(const char *location)
{
  if (NULL == location) {
    return false;
  }
  
  // Not empty
  if (!strlen(location)) {
    return false;
  }
  const char *ptr = location;

  // checking location begins with capital letter
  if(*ptr < 'A' || *ptr > 'Z') {
    return false;
  }

  ptr++;
  while(*ptr) {
    if((*ptr < 'a' || *ptr > 'z') && *ptr != ' '){
      return false;
    }
    ptr++;
  }

  return true;
}

int idCompare(MapKeyElement element1, MapKeyElement element2)
{
  return (int)*((ChessId *)element1) - (int)*((ChessId *)element2);
}
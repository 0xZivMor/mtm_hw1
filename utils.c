#include "utils.h"
#include "map.h"
#include <string.h>
#include <stdlib.h>

struct node_t {
  void *value;
  struct node_t *next;
};

MapKeyElement copyInt(MapKeyElement element)
{
  int *copy = (int*)malloc(sizeof(int));
  RETURN_NULL_ON_NULL(copy)

  *copy = *((int *)element);
  return (MapKeyElement)copy;
}

void freeInt(MapKeyElement element)
{
  if (NULL == element) {
    return;
  }

  free(element);
}

bool validateId(int id)
{
  if (id <= 0) {
    return false;
  }
  return true;
}

bool validateLocation(const char *location)
{
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
  return *((int *)element1) - *((int *)element2);
}
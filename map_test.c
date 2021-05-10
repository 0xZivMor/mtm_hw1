#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "map.h"

int compareKeys(MapKeyElement first, MapKeyElement second);
MapDataElement copyData(MapDataElement src);
MapKeyElement copyKey(MapKeyElement src);
void freeData(MapDataElement data);
void freeKey(MapKeyElement key);

int compareKeys(MapKeyElement first, MapKeyElement second)
{
  return *(int *)first - *(int *)second;
}

MapDataElement copyData(MapDataElement src)
{
  MapDataElement new_data = (MapDataElement)malloc(strlen((char *)src) + 1);
  strncpy(new_data, src, strlen(src) + 1);
  return new_data;
}
MapKeyElement copyKey(MapKeyElement src)
{
  MapKeyElement new_key = (MapDataElement)malloc(sizeof(int));
  *(int *)new_key = * (int *)src;
  return new_key;
}

void freeData(MapDataElement data)
{
  free(data);
}
void freeKey(MapKeyElement key)
{
  free(key);
}

int main()
{
  Map map = mapCreate(copyData, copyKey, freeData, freeKey, compareKeys);

  char c = 'a', *data;
  int *key; 
  for (int i = 26; i; i--, c++) {
    key = (int *)malloc(sizeof(int));
    *key = i;
    data = (char *)malloc(sizeof(char));
    *data = c;
    mapPut(map, (MapKeyElement)key, (MapDataElement)data);
  }

  MAP_FOREACH(int *, key, map) {
    printf("%c ", *(char *)mapGet(map, key));

  }
  printf("\n");
  return 0;
}
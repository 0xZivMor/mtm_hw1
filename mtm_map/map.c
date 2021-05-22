#include <stdlib.h>
#ifndef NDEBUG
#include <assert.h>
#endif  // NDEBUG
#include "map.h"

typedef struct node_t {
  MapKeyElement key;
  MapDataElement data;
  struct node_t *next;
  struct node_t *previous;
} *Node;

struct Map_t {
  copyMapDataElements copyData;
  copyMapKeyElements copyKey;
  freeMapDataElements freeData;
  freeMapKeyElements freeKey;
  compareMapKeyElements compare;

  Node top;
  Node current;
  int keys_count;
};


/**
 * Allocates a new dictionary node.
 * 
 * @param key - key element of new node. Assuming not NULL.
 * @param data - data element of new node. Assuming not NULL.
 * @param next - pointer to next node in the list, may be NULL
 * @param previous - pointer to previous node in the list, may be NULL.
 * @return
 *    Node populated with the arguments values on success
 *    NULL on memory allocation error
 */
static Node newNode(MapKeyElement key, 
                    MapDataElement data, 
                    Node next,
                    Node previous);

/**
 * Search a node in the map with the provided key. All arguments are
 * assumed to be valid.
 * 
 * @param map - map to be searched
 * @param key - key to be found
 * @return
 *    Node if key was found in map, otherwise NULL.
 */
static Node mapFind(Map map, MapKeyElement key);

Map mapCreate(copyMapDataElements copy_data_elements_method,
              copyMapKeyElements copy_key_elements_method,
              freeMapDataElements free_data_elements_method,
              freeMapKeyElements free_key_element_method,
              compareMapKeyElements compare_key_elements_method)
{
  if ((NULL == copy_data_elements_method) ||
      (NULL == copy_key_elements_method) ||
      (NULL == free_data_elements_method) ||
      (NULL == free_key_element_method) ||
      (NULL == compare_key_elements_method)) {
    return NULL;  
  }

  Map map = (Map)malloc(sizeof(*map));

  if (NULL == map) {
    return NULL;
  }

  map->top = NULL;
  map->current = NULL;
  map->keys_count = 0;

  map->copyData = copy_data_elements_method;
  map->copyKey = copy_key_elements_method;
  map->freeData = free_data_elements_method;
  map->freeKey = free_key_element_method;
  map->compare = compare_key_elements_method;

  return map;
}

void mapDestroy(Map map)
{
  if (NULL == map) {
    return;
  }

  mapClear(map);
  free(map);
}

Map mapCopy(Map map)
{
  if (NULL == map) {
    return NULL;
}

  Map copy = mapCreate(map->copyData, map->copyKey, map->freeData, 
                       map->freeKey, map->compare);
  
  if (NULL == copy) {
    return NULL;
  }

  MapKeyElement key = mapGetFirst(map);
  MapDataElement data;

  while (NULL != key) {
    data = mapGet(map, key);
    if (MAP_SUCCESS != mapPut(copy, key, data)) {
      break;
    }
    key = mapGetNext(map);
  }

  // Had issues copying data to the new copy
  if (NULL != key) {
    mapDestroy(copy);
    return NULL;
  }

  return copy;
}

int mapGetSize(Map map)
{
  if (NULL == map) {
    return -1;
  }

  return map->keys_count;
}

bool mapContains(Map map, MapKeyElement element)
{
  if ((NULL == map) || (NULL == element)) {
    return false;
  }

  if (NULL != mapFind(map, element)) {
    return true;
  }

  return false;
}

MapResult mapPut(Map map, MapKeyElement keyElement, MapDataElement dataElement)
{
  if ((NULL == map) || (NULL == keyElement) || (NULL == dataElement)) {
    return MAP_NULL_ARGUMENT;
  }

  MapDataElement data_copy = map->copyData(dataElement);
  MapKeyElement key_copy = map->copyKey(keyElement);
  
  assert(NULL != data_copy);
  assert(NULL != key_copy);

  if ((NULL == data_copy) || (NULL == key_copy)) {
    return MAP_OUT_OF_MEMORY;
  }

  Node current_node;

  // if key is already found, we need to update it
  current_node = mapFind(map, key_copy);
  if (NULL != current_node) {
    MapDataElement old_data = current_node->data;
    current_node->data = data_copy;
    map->freeData(old_data);
    return MAP_SUCCESS;
  }

  current_node = map->top;
  Node previous = NULL;

  while ((NULL != current_node) && 
         (map->compare(key_copy, current_node->key)) > 0) {
    previous = current_node;
    current_node = current_node->next;
  }

  Node new_node = newNode(key_copy, data_copy, current_node, previous);     
  if (NULL == new_node) {                         
    map->freeData(data_copy);                      
    map->freeKey(key_copy);                       
    return MAP_OUT_OF_MEMORY;         
  }

  if (NULL != previous) {
    previous->next = new_node;
  }
  if (NULL != current_node) {
    current_node->previous = new_node;
  }

  map->keys_count++;

  // mapping was empty, put the new pair at the top
  if (NULL == map->top) {
    map->top = new_node;
  }

  return MAP_SUCCESS;
}

MapDataElement mapGet(Map map, MapKeyElement keyElement)
{
  if ((NULL == map) || (NULL == keyElement)) {
    return NULL;
  }

  Node requested = mapFind(map, keyElement);

  if (NULL == requested) {
    return NULL;
  }

  return requested->data;
}

MapResult mapRemove(Map map, MapKeyElement keyElement)
{
  if ((NULL == map) || (NULL == keyElement)) {
    return MAP_NULL_ARGUMENT;
  }

  Node requested = mapFind(map, keyElement);
  if (NULL == requested) {
    return MAP_ITEM_DOES_NOT_EXIST;
  }

  Node next_node = requested->next, prev_node = requested->previous;
  next_node->previous = prev_node;
  prev_node->next = next_node;
  map->keys_count--;

  map->freeData(requested->data);
  map->freeKey(requested->key);
  
  free(requested);
  return MAP_SUCCESS;
}

MapKeyElement mapGetFirst(Map map)
{
  // bad map provided or mapping is empty
  if (NULL == map || NULL == map->top) {
    return NULL;
  }

  // next value to be enumerated is the second
  map->current = map->top->next; 

  return map->copyKey(map->top->key);
}

MapKeyElement mapGetNext(Map map)
{
  // bad map or iterator has reached the end of the map
  if ((NULL == map) || (NULL == map->current)) {
    return NULL;
  }

  MapKeyElement key = map->current->key;
  map->current = map->current->next;

  return map->copyKey(key);
}

MapResult mapClear(Map map)
{
  if (NULL == map) {
    return MAP_NULL_ARGUMENT;
  }

  Node current = map->top, next;

  // destroy all nodes, keys and data elements
  while (NULL != current) {
    map->freeKey(current->key);
    map->freeData(current->data);
    next = current->next;
    free(current);
    current = next;
  }

  map->keys_count = 0;
  map->top = NULL;
  map->current = NULL;
  return MAP_SUCCESS;
}

static Node mapFind(Map map, MapKeyElement key)
{
  Node current_node = map->top;

  while (NULL != current_node) {
    if (0 == map->compare(current_node->key, key)) {
      return current_node;
    }
    current_node = current_node->next;
  }

  return NULL;
}

static Node newNode(MapKeyElement key, 
                    MapDataElement data, 
                    Node next,
                    Node previous)
{
  Node new_node = (Node)malloc(sizeof(struct node_t));

  if (NULL == new_node) {
    return NULL;
  }

  new_node->key = key;
  new_node->data = data;
  new_node->next = next;
  new_node->previous = previous;

  return new_node;
}
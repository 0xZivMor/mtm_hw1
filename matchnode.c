#include "matchnode.h"
#include "match.h"
#include "utils.h"
#include <stdlib.h>

struct match_node_t
{
  Match match;
  struct match_node_t *next;
};

matchNode matchNodeCreate(Match match, matchNode next)
{
  matchNode new_match_node = (matchNode)malloc(sizeof(*new_match_node));
  RETURN_NULL_ON_NULL(new_match_node)
  
  new_match_node->match = match;
  new_match_node->next = next;
  return new_match_node;
}

matchNode matchNodeAdd(matchNode previous, Match match)
{
  if(previous == NULL || match == NULL)
  {
    return NULL;
  }
  matchNode new_match_node = (matchNode)malloc(sizeof(*new_match_node));
  RETURN_NULL_ON_NULL(new_match_node);

  new_match_node->match = match;
  previous->next = new_match_node;
  return new_match_node;
}

matchNode matchNodeNext(matchNode node)
{
  RETURN_NULL_ON_NULL(node)
  return node->next;
}

int matchNodeTotalTime(matchNode list)
{
  int total_time = 0;
  
  FOREACH_MATCH(list, current) {
    Match current_match = matchNodeGetMatch(current);
    total_time += matchGetDuration(current_match);
  }

  return total_time;
}

int matchNodeGetSize(matchNode list)
{
  int counter = 0;
  
  FOREACH_MATCH(list, current) {
    // don't count dummy nodes
    if (NULL != matchNodeGetMatch(current)) {
      counter++;
    }
  }

  return counter;
}

Match matchNodeGetMatch(matchNode node)
{
  RETURN_NULL_ON_NULL(node)
  return node->match;
}

void matchNodeRemove(matchNode *list, Match match)
{
  if (NULL == list || NULL == *list || NULL == match) {
    return;
  }

  matchNode previous = NULL;
  matchNode head = *list;

  // going over the list with 2 pointers; list and previous. 
  // list is always one ahead of previous
  while(head) {
    Match current = matchNodeGetMatch(head);
    
    if(!matchCompare(current, match)) {  // found a match
      
      if (NULL == previous) {  // match was the first node
        *list = matchNodeNext(head);
      } else {
        previous->next = matchNodeNext(head); // changing order
      }
      matchNodeDestroy(head, false);
      return;
    }
    
    previous = head;
    head = matchNodeNext(head);
  }
}

void matchNodeDestroy(matchNode node, bool destory_match)
{
  if (NULL == node) {
    return;
  }
  
  if (destory_match) {
    matchDestroy(matchNodeGetMatch(node));
  }

  free(node);
  return;
}

void matchNodeDestroyList(matchNode head, bool destory_match)
{
  matchNode current = head;
  while(current != NULL) {
    matchNode toDestroy = current;
    current = matchNodeNext(current);
    matchNodeDestroy(toDestroy, destory_match);
  }
}

void matchNodeConcat(matchNode dest, matchNode addition)
{
  if (NULL == dest) {
    return;
  }

  dest->next = addition;
}

void matchNodeRemoveTournamentFromList(matchNode list, int tournament_id, bool destory_match)
{
  matchNode current = list, previous = NULL, to_destroy;
  Match match;
  while (NULL != current) {
    match = matchNodeGetMatch(current);
    
    // match is from the tournament to be removed
    if (tournament_id == matchGetTournament(match)) {
      if (NULL != previous) {
        previous->next = current->next;
      }
      // previous remains the same
      to_destroy = current;
      current = current->next;
      matchNodeDestroy(to_destroy, destory_match);
    } else {
      previous = current;
      current = current->next;
    }
  }
}

MapDataElement matchNodeCopy(MapDataElement original_list)
{
  RETURN_NULL_ON_NULL(original_list)
  matchNode original = (matchNode) original_list, new_list = NULL;

  FOREACH_MATCH(original, current) {
    Match match = matchNodeGetMatch(current);
    matchNode new_node = matchNodeCreate(match, new_list);

    if (NULL == new_node) { // memory error
      matchNodeDestroyList(new_list, false);
      return NULL;
    }
    new_list = new_node;
  }

  return (MapDataElement) new_list;
}

void matchNodeDestroyMap(MapDataElement element)
{
  if (NULL == element) {
    return;
  }
  matchNodeDestroyList((matchNode)element, false);
}

bool matchNodeInList(matchNode head, Match match)
{
  FOREACH_MATCH(head, current) {
    Match current_match = matchNodeGetMatch(current);

    if (!matchCompare(match, current_match)) {
      return true;
    }
  }
  return false;
}


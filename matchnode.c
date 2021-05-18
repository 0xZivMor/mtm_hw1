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
  RETURN_NULL_ON_NULL(match);

  matchNode new_match_node = (matchNode) malloc(sizeof(*new_match_node));
  RETURN_NULL_ON_NULL(new_match_node)
  
  new_match_node->match = match;
  new_match_node->next = next;
  return new_match_node;
}

matchNode matchNodeNext(matchNode node)
{
  RETURN_NULL_ON_NULL(node)
  return node->next;
}

int matchNodeGetSize(matchNode list)
{
  int counter = 0;
  
  FOREACH_MATCH(list) {
    counter++;
  }

  return counter;
}

Match matchNodeGetMatch(matchNode node)
{
  RETURN_NULL_ON_NULL(node)
  return node->match;
}

void matchNodeRemove(matchNode list, Match match)
{
  if(list == NULL || match == NULL) {
    return;
  }

  // going over the list with 2 pointers; list and previous. 
  // list is always one ahead of previous
  Match current;
  matchNode previous = NULL;
  while(list) {
    current = matchNodeGetMatch(list);
    
    if(!matchCompare(current, match)) {
      previous->next = list->next; // changing order
      matchNodeDestroy(list, false);
      return;
    }
    
    previous = list;
    list = matchNodeNext(list);
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
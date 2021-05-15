#include "matchnode.h"
#include "match.h"

struct match_node_t
{
  Match match;
  matchNode next;
};

matchNode newMatchNode(Match match, matchNode next)
{
  if(match == NULL)
  {
    return NULL;
  }
  matchNode new_match_node = (matchNode) malloc(sizeof(struct matchNode));
  if(new_match_node == NULL)
  {
    return NULL;
  }
  new_match_node->match = match;
  new_match_node->next = next;
  return new_match_node;
}

matchNode nextMatchNode(matchNode node)
{
  if(node == NULL)
  {
    return NULL;
  }
  return node->next;
}

int getSize(matchNode list)
{
  matchNode ptr = list;
  int counter = 0;
  while(ptr)
  {
    counter++;
    ptr = nextMatchNode(ptr);
  }
  return counter;
}

Match getMatchFromMatchNode(matchNode node)
{
  if(node == NULL)
  {
    return NULL;
  }
  return node->match;
}

void matchNodeRemove(matchNode list, Match match)
{
  if(list == NULL || match == NULL)
  {
    return;
  }
  matchNode ptr = list;
  Match current  = getMatchFromMatchNode(ptr);
  //checking if match is the first on the list
  if(matchCompare(current, match) == 0)
  {
    list = list->next;
    return;
  }
  //going over the list with 2 pointers; current and previous. 
  //ptr is always one ahead of previous
  matchNode previous = ptr;
  ptr = nextMatchNode(ptr);
  while(ptr)
  {
    current  = getMatchFromMatchNode(ptr);
    if(matchCompare(current, match) == 0)
    {
      previous->next = ptr->next; //changing order
      ptr->next = NULL;
      return;
    }
    previous = ptr;
    ptr = nextMatchNode(ptr);
  }
}

void matchNodeDestroy(matchNode node, bool destory_match)
{
  if(node == NULL)
  {
    return NULL;
  }
  Match toDestroy = node->match;
  node->next = NULL;
  free(node);
  if(destory_match)
  {
    matchDestroy(toDestroy);
  }
  return;
}
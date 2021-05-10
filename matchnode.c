#include "matchnode.h"
#include "match.h"

struct match_node_t
{
  Match match;
  matchNode next;
};

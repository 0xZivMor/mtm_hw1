#include "tournament.h"
#include "player.h"
#include "match.h"

struct match_t {
  Player first;
  Player second;
  Player winner;
  Tournament tournament;
  int duration;
};
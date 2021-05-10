#include "tournament.h"
#include "matchnode.h"
#include "player.h"
#include "map.h"

struct tournament_t {
  int id;
  matchNode matches;
  Map players;
  char *location;
  int max_matches_per_player;
  int players_count;
  bool finished;
  Player winner;
};
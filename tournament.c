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
static MapKeyElement copyId(MapKeyElement element);
static void freeId(MapKeyElement element);
static bool isLocationValid(const char *location)
{
  char* ptr = location;
  if(*ptr < 'A' || *ptr > 'Z') //checking location begins with capital letter
  {
    return false;
  }
  ptr++;
  while(*ptr)
  {
    if((*ptr < 'a' || *ptr > 'z') && *ptr != ' ')
    {
      return false;
    }
    ptr++;
  }
  return true;
}

Tournament tournamentCreate(int id, const char *location, int max_games_per_player)
{
  //checking for incorrect parameters
  if(id < 0 || max_games_per_player <= 0 || !isLocationValid(location))
  {
    return NULL;
  }
  
  Tournament tournament = (Tournament) malloc(sizeof(*tournament));
  if(tournament == NULL)
  {
    return NULL;
  }
  tournament->id = id;
  tournament->matches = NULL;
  tournament->players =  mapCreate(playerCopy, 
                             copyId, 
                             playerDestroy, 
                             freeId, 
                             playerCompare);
  if(tournament->players == NULL)
  {
    return NULL;
  }
  tournament->players_count = 0;
  tournament->location = location;
  tournament->max_matches_per_player = max_games_per_player;
  tournament->finished = false;
  tournament->winner = NULL;
}

ChessResult tournamentAddMatch(Tournament tournament, Match match)
{
  if(tournament == NULL || match == NULL)
  {
    return CHESS_NULL_ARGUMENT;
  }
  if(tournament->finished == true)
  {
    return CHESS_TOURNAMENT_ENDED;
  }
  if(mapContains(tournament->matches, match)) // match already in the tournament
  {
    return CHESS_GAME_ALREADY_EXISTS;
  }
  //creating list of matches played in the tournament by the players in @param match
  matchNode player1_matches;
  matchNode player2_matches;
  ChessResult first = tournamentGetMatchesByPlayer(tournament, matchGetFIrst(match), player1_matches);
  ChessResult second = tournamentGetMatchesByPlayer(tournament, matchGetFIrst(match), player1_matches);
  if(getSize(player1_matches) > tournament->max_matches_per_player || getSize(player2_matches) > tournament->max_matches_per_player)
  //if the number of matches played by one (or both) of them is too much, abbort
  {
    matchNodeDestroy(player1_matches);
    matchNodeDestroy(player2_matches);
    return CHESS_INVALID_MAX_GAMES;
  }
  //now adding the match to list of matches
  if(newMatchNode(match, tournament->matches) == NULL)
  {
    matchNodeDestroy(player1_matches);
    matchNodeDestroy(player2_matches);
    return CHESS_OUT_OF_MEMORY;
  }
  //now dealing with the players
  Player first_player = matchGetFirst(match);
  Player second_player = matchGetSecond(match);
  if(first == CHESS_PLAYER_NOT_EXIST) //this is the first game in the tournament played by first_player
  {
    //adding the player to players map
    first = mapPut(tournament->players, playerGetId(first_player), first_player);
    if(first == CHESS_OUT_OF_MEMORY)
    {
      matchNodeDestroy(player1_matches);
      matchNodeDestroy(player2_matches);
      tournamentRemoveMatch(tournament, match);
      return CHESS_OUT_OF_MEMORY;
    }
    first = CHESS_PLAYER_NOT_EXIST; //flag to signify we added the first_player just now for the 1st time
  }
  //same with second_player
  if(second == CHESS_PLAYER_NOT_EXIST)
  {
    //adding 2nd player to players map
    second = mapPut(tournament->players, playerGetId(second_player), second_player);
    if(second == CHESS_OUT_OF_MEMORY)
    {
      matchNodeDestroy(player1_matches);
      matchNodeDestroy(player2_matches);
      tournamentRemoveMatch(tournament, match);
      if(first == CHESS_PLAYER_NOT_EXIST) //first_player was added now for the 1st time so he as well needs to be removed
      {
        tournamentRemovePlayer(tournament, playerGetId(first_player));
      }
    }
  }
  matchNodeDestroy(player1_matches);
  matchNodeDestroy(player2_matches);
  return CHESS_SUCCESS;
}


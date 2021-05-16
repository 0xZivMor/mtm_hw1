#include "tournament.h"
#include "matchnode.h"
#include "player.h"
#include "map.h"
#include "string.h"

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
  int length = strlen(*location);
  char* new_loc = (char*) malloc(sizeof(char) * length);
  tournament->location = new_loc;
  tournament->max_matches_per_player = max_games_per_player;
  tournament->finished = false;
  tournament->winner = NULL;
  if(new_loc == NULL)
  {
    tournamentDestroy(tournament);
    return NULL;
  }
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
    matchNodeDestroy(player1_matches, false);
    matchNodeDestroy(player2_matches, false);
    return CHESS_INVALID_MAX_GAMES;
  }
  //now adding the match to list of matches
  if(newMatchNode(match, tournament->matches) == NULL)
  {
    matchNodeDestroy(player1_matches, false);
    matchNodeDestroy(player2_matches, false);
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
      matchNodeDestroy(player1_matches, false);
      matchNodeDestroy(player2_matches, false);
      tournamentRemoveMatch(tournament, match);
      return CHESS_OUT_OF_MEMORY;
    }
    tournament->players_count ++;
    first = CHESS_PLAYER_NOT_EXIST; //flag to signify we added the first_player just now for the 1st time
  }
  //same with second_player
  if(second == CHESS_PLAYER_NOT_EXIST)
  {
    //adding 2nd player to players map
    second = mapPut(tournament->players, playerGetId(second_player), second_player);
    if(second == CHESS_OUT_OF_MEMORY)
    {
      matchNodeDestroy(player1_matches, false);
      matchNodeDestroy(player2_matches, false);
      tournamentRemoveMatch(tournament, match);
      if(first == CHESS_PLAYER_NOT_EXIST) //first_player was added now for the 1st time so he as well needs to be removed
      {
        tournamentRemovePlayer(tournament, playerGetId(first_player));
      }
      return CHESS_OUT_OF_MEMORY;
    }
   tournament->players_count ++;
  }  
  matchNodeDestroy(player1_matches, false);
  matchNodeDestroy(player2_matches, false);
  return CHESS_SUCCESS;
}

ChessResult tournamentRemovePlayer(Tournament tournament, int player_id)
{

}

ChessResult tournamentRemoveMatch(Tournament tournament, Match match);

ChessResult tournamentEnd(Tournament tournament)
{
  if(tournament == NULL)
  {
    return CHESS_NULL_ARGUMENT;
  }
  Player current;
  Player temp;
  int max_result = -1;
  int curr_result = 0;
  matchNode games_played;
  matchNode ptr;
  ChessResult status;
  //going over the players. for each player creating a matchnode list of his games and calculating result
  MAP_FOREACH(Player,current, tournament->players)
  {
    tournamentGetMatchesByPlayer(tournament, current->id, games_played);
    ptr = games_played;
    while(ptr) //going over all games played by current
    {
      status = matchGetWinner(getMatchFromMatchNode(ptr),temp);
      if(status == CHESS_SUCCESS)
      {
        if(playerCompare(current,temp) == 0 ) //if the winner of the match is current
        {
          curr_result += 2;
        }
        else if(temp == NULL) //there was a draw
        {
          curr_result == 1;
        }
      }
      ptr = nextMatchNode(ptr);
    }
    if(curr_result > max_result) //replace winner if we got a better result
    {
      max_result = curr_result;
      tournament->winner = current;
    }
    else if(curr_result == max_result) //if results are the same
    {
      if(playerGetId(tournament->winner) > playerGetId(current)) //chose the one with "lower" id
      {
        tournament->winner = current;
      }
    }
  }
  tournament->finished = true; //updating status
  return CHESS_SUCCESS;
}

ChessResult tournamentGetMatchesByPlayer(Tournament tournament, 
                                         int player_id, 
                                         matchNode *list);

void tournamentDestroy(Tournament tournament)
{
  if(tournament == NULL)
  {
    return;
  }
  matchNodeDestroy(tournament->matches, true);
  mapDestroy(tournament->players);
  playerDestroy(tournament->winner, false);
  free(tournament);
}

int tournamentCompare(Tournament tournament1, Tournament tournament2)
{
  return tournament1->id - tournament2->id;
}

bool tournamentIsEnded(Tournament tournament)
{
  if(tournament == NULL)
  {
    return NULL;
  }
  return tournament->finished;
}

Tournament tournamentCopy(Tournament original)
{
  if(original == NULL)
  {
    return NULL;
  }
  Tournament new_tournament;
  new_tournament = tournamentCreate(original->id, original->location, original->max_matches_per_player);
  if(new_tournament == NULL)
  {
    return NULL;
  }
  new_tournament->finished = original->finished;
  new_tournament->players_count = original->players_count;
  new_tournament->winner = original->winner;
  new_tournament->matches = original->matches;
  new_tournament->players = original->players;
  return new_tournament;
}

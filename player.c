#include "player.h"
#include "matchnode.h"

//Need to go over create, destroy and copy

struct player_t {
  int id;
  matchNode matches;
};

Player playerCreate(int id)
{
  if(id <= 0)
  {
    return NULL;
  }
  Player player = (Player) malloc(sizeof(struct Player));
  if(player == NULL || matches == NULL)
  {
    return NULL;
  }
  player->id = id; 
  player->matches = NULL;
  return player;
}

int playerGetId(Player player)
{
  if(player == NULL)
  {
    return 0;
  }
  return player->id;
}

matchNode playerGetMatches(Player player)
{
  if(player == NULL)
  {
    return NULL;
  }
  return player->matches;
}

ChessResult playerAddMatch(Player player, Match match)
{
  if(player == NULL || match == NULL)
  {
    return CHESS_NULL_ARGUMENT;
  }
  if(!newMatchNode(match, player->matches)) //trying to add a new MatchNode to matches' list
  {
    return CHESS_OUT_OF_MEMORY;
  }
  return CHESS_SUCCESS;
}

ChessResult playerRemoveMatch(Player player, Match match)
{
  if(player == NULL || match == NULL)
  {
    return CHESS_NULL_ARGUMENT;
  }
  matchNodeRemove(player->matches, match);
  return CHESS_SUCCESS;
}

double playerGetScore(Player player)
{
  if(player == NULL)
  {
    return 0;
  }
  double score = 0;
  MatchNode ptr = player->matches;
  Match toCheck;
  Player* temp;
  while(ptr)
  {
    toCheck = getMatchFromMatchNode(ptr);
    temp = matchGetWinner(toCheck); 
    if(temp == &player) //this player won
    {
      score += 6;
    }
    else if(temp == NULL) //draw
    {
      score += 2;
    }
    else{ //the player lost
      score += -10;
    }
    ptr = nextMatchNode(ptr);
  }

  int number_of_games = getSize(player->matches);
  return score/ number_of_games; 
}

double playerCompare(Player player1, Player player2)
{
  //NO CHECK FOR NULL ARGUMENT  
  double score1 = playerGetScore(player1);
  double score2 = playerGetScore(player2);
  return (int)(score1-score2);
}

void playerDestroy(Player player, bool remove_from_tournaments)
{
  if(player == NULL)
  {
    return;
  }
  if(!remove_from_tournaments) //just freeing the player, without changing games he plays
  {
    MatchNode ptr = player->matches;
    MatchNode toDelete;
    while(ptr)
    {
      toDelete = ptr;
      ptr = nextMatchNode(ptr);
      matchNodeDestroy(toDelete, false);
    }
    free(player);
    return;
  }
  //setting all games in runnning tournements that [player] played so that he's the loser
  matchNode ptr = player->matches;
  while(ptr)
  {
    Match current = getMatchFromMatchNode(ptr);
    Tournament current_tour = matchGetTournament(current));
    if(tournamentIsEnded == false) //if tournament is still running 
    {
      matchSetLoser(current, player); //set player as the loser
    }
    ptr = nextMatchNode(ptr);
  }
  //now freeing the player using same func with false value
  playerDestroy(player, false);
}

Player playerCopy(Player original)
{
  if(original == NULL)
  {
    return NULL;
  }
  int id = playerGetId(original);
  Player new_player = playerCreate(id);
  if(new_player == NULL)
  {
    return NULL;
  }
  matchNode list_of_matches = playerGetMatches(original);
  while(list_of_matches) //copying matches
  {
    Match copy_match = matchCopy(list_of_matches);
    if(playerAddMatch(new_player, copy_match) != CHESS_SUCCESS) //memory issue occured
    {
      playerDestroy(new_player);
      return NULL;
    }
    list_of_matches = nextMatchNode(list_of_matches);
  }
  return new_player;
}
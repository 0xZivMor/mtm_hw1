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

Match matchCreate(Player first_player, Player second_player, Player winner, Tournament tournament, int duration)
{
  if(first_player == NULL || second_player == NULL || tournament == NULL)
  {
    return NULL;
  }
  if(duration == NULL || duration < 0)
  {
    return NULL;
  }
  if(playerGetId(first_player) == playerGetId(second_player)) //the two players are the same player
  {
    return NULL;
  }
  Match match = (Match)malloc(sizeof(*match));
  if(match == NULL)
  {
    return NULL;
  }
  match->first = first_player;
  match->second = second_player;
  match->winner = winner;
  match->tournament = tournament;
  match->duration = duration;
  return match;
}

Player matchGetFirst(Match match)
{
  if(match == NULL)
  {
    return NULL;
  }
  return match->first;
}

Player matchGetSecond(Match match);
{
  if(match == NULL)
  {
    return NULL;
  }
  return match->second;

ChessResult matchSetWinner(Match match, Player winner)
{
  if(match == NULL)
  {
    return CHESS_NULL_ARGUMENT;
  }
  if(winner == NULL) //if it was a draw
  {
    match->winner = NULL;
    return CHESS_SUCCESS;
  }
  if(!matchIsParticipant(match, winner)) //if winner is not one of the players
  {
    return CHESS_PLAYER_NOT_EXIST;
  }
  if(winner == match->first)
  {
    match->winner = match->first;
  }
  else
  {
    match->winner = match->second;
  }
  return CHESS_SUCCESS;
}

ChessResult matchSetLoser(Match match, Player loser)
{
  if(match == NULL)
  {
    return CHESS_NULL_ARGUMENT;
  }
  if(loser == NULL)
  {
    match->winner = NULL;
    return CHESS_SUCCESS;
  }
  if(!matchIsParticipant(match, loser)) //if loser is not one of the players
  {
    return CHESS_PLAYER_NOT_EXIST;
  }
  if(match->first == loser)
  {
    return matchSetWinner(match, match->second);
  }
  return matchSetWinner(match, match->first);
}

bool matchIsParticipant(Match match, Player player)
{
  if(match == NULL || player == NULL)
  {
    return false;
  }
  if(player != match->first && player != match->second) //is not one of the players
  {
    return false;
  }
  return true;
}

ChessResult matchGetWinner(Match match, Player *winner)
{
  if(match == NULL)
  {
    winner = NULL;
    return CHESS_NULL_ARGUMENT;
  }
  winner = match->winner;
  return CHESS_SUCCESS;
}

int matchGetDuration(Match match)
{
  if(match == NULL)
  {
    return -1;
  }
  return match->duration;
}

Tournament matchGetTournament(Match match)
{
  if(match == NULL)
  {
    return NULL;
  }
  return match->tournament;
}

void matchDestroy(Match match)
{
  if(match == NULL)
  {
    return;
  }
  tournamentRemoveMatch(match->tournament, match);
  free(match);
}

bool isSamePlayers(Match match1, Match match2)
{
  if(match1 == NULL || match2==NULL)
  {
    return false;
  }
  if(matchIsParticipant(match1, match2->first) && matchIsParticipant(match1, match2->second))
  {
    return true;
  }
  return false;

}

int matchCompare(Match match1, Match match2)
{
  if(match1 == NULL || match2 == NULL)
  {
    return -1;
  }
  if(!isSamePlayers(match1, match2)) //if not the same players on both games
  {
    return -1;
  }
  if(match1->winner != match2->winner || match1->duration != match2->duration || 
  match1->tournament != match2->tournament)
  {
    return -1;
  }

  return 0;
}

Match matchCopy(Match original)
{
  if(original == NULL)
  {
    return NULL;
  }
  Match match = (Match)malloc(sizeof(*match));
  if(match == NULL)
  {
    return NULL;
  }
  match->first = original->first;
  match->second = original->second;
  match->winner = original->winner;
  match->duration = original->duration;
  match->tournament = original->tournament;
  return match;
}
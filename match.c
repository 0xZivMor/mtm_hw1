#include "match.h"
#include "utils.h"
#include <stdlib.h>

struct match_t {
  ChessId first;
  ChessId second;
  ChessId winner;
  ChessId tournament;
  int duration;
};

/**
 * checks if the players on both matches are the same
 * @param match1 first match in question
 * @param match2 second match in question
 * @return true if players are the same, 
 * false otherwie.
 */
static bool isSamePlayers(Match match1, Match match2);

Match matchCreate(ChessId first_player, 
                  ChessId second_player, 
                  ChessId winner, 
                  ChessId tournament, 
                  int duration)
{
  // arguments validation
  if (!validateId(first_player) || 
      !validateId(second_player) || 
      !validateId(tournament) ||
      duration < 0) {
    return NULL;
  }

  // both players are the same player
  if (first_player == second_player) {
    return NULL;
  }

  Match match = (Match)malloc(sizeof(*match));
  RETURN_NULL_ON_NULL(match)

  match->first = first_player;
  match->second = second_player;
  match->winner = winner;
  match->tournament = tournament;
  match->duration = duration;

  return match;
}

ChessId matchGetFirst(Match match)
{
  RETURN_ZERO_ON_NULL(match)
  return match->first;
}

ChessId matchGetSecond(Match match)
{
  RETURN_ZERO_ON_NULL(match)
  return match->second;
}

ChessResult matchSetWinner(Match match, ChessId winner)
{
  RETURN_RESULT_ON_NULL(match)

  // match result was a draw
  if(!winner) 
  {
    match->winner = 0;
    return CHESS_SUCCESS;
  }

  // winner is not one of the players
  if(!matchIsParticipant(match, winner)) {
    return CHESS_PLAYER_NOT_EXIST;
  }

  if (winner != matchGetFirst(match)) {
    match->winner = match->first;
  } else {
    match->winner = match->second;
  }

  return CHESS_SUCCESS;
}

ChessResult matchSetLoser(Match match, ChessId loser)
{
  RETURN_RESULT_ON_NULL(match)
  
  // draw
  if(!loser) {
    match->winner = 0;
    return CHESS_SUCCESS;
  }

  // loser is not one of the players
  if(!matchIsParticipant(match, loser)) {
    return CHESS_PLAYER_NOT_EXIST;
  }

  if(loser == matchGetFirst(match)) { // second player won
    return matchSetWinner(match, match->second);
  } else {
    return matchSetWinner(match, match->first);
  }
}

bool matchIsParticipant(Match match, ChessId player)
{
  if(NULL == match || !validateId(player)) {
    return false;
  }

  // provided player is not a participant
  if(player != matchGetFirst(match) && player != matchGetSecond(match)) {
    return false;
  }

  return true;
}

ChessResult matchGetWinner(Match match, ChessId *winner)
{
  if(NULL == match)
  {
    *winner = 0;
    return CHESS_NULL_ARGUMENT;
  }

  *winner = match->winner;
  return CHESS_SUCCESS;
}

ChessId matchGetDuration(Match match)
{
  if(NULL == match) {
    return 0;
  }

  return match->duration;
}

ChessId matchGetTournament(Match match)
{
  RETURN_ZERO_ON_NULL(match)

  return match->tournament;
}

void matchDestroy(Match match)
{
  if(NULL == match) {
    return;
  }

  free(match);
}

ChessResult matchForfiet(Match match, 
                         ChessId player, 
                         ChessId *previous_winner,
                         ChessId *new_winner)
{
  // arguments validation
  RETURN_RESULT_ON_NULL(match)
  if (!validateId(player)) {
    return CHESS_INVALID_ID;
  }

  if (!matchIsParticipant(match, player)) {
    return CHESS_PLAYER_NOT_EXIST;
  }

  if (NULL != previous_winner) {
    matchGetWinner(match, previous_winner);
  }

  ChessId opponent;
  if (match->first == player) {
    opponent = matchGetSecond(match);
    match->first = 0;
  } else {
    opponent = matchGetFirst(match);
    match->second = 0;
  }

  matchSetWinner(match, opponent);
  
  if (NULL != new_winner) {
    *new_winner = opponent;
  }

  return CHESS_SUCCESS;
}

static bool isSamePlayers(Match match1, Match match2)
{
  if (NULL == match1 || NULL == match2) {
    return false;
  }

  /*
    If one of the participants of a match is 0 it means that one of the 
    participants was removed from the system, so it's acceptable to have
    two matches with the same player and 0 participant, thus both matches
    don't have the same participants
  */
  if (matchIsParticipant(match1, 0) && matchIsParticipant(match1, 0)) {
    return false;
  }

  // both participants are in both matches
  if (matchIsParticipant(match1, match2->first) && 
      matchIsParticipant(match1, match2->second)) {
    return true;
  }

  return false;
}

int matchCompare(Match match1, Match match2)
{
  COMPARE_NOT_NULL(match1, match2)

  // matches are not of the same tournament thus different
  if (matchGetTournament(match1) != matchGetTournament(match2)) {
    return 1;
  }

  // not the same players on both matches -> different matches 
  if(!isSamePlayers(match1, match2)) {
    return 1;
  }

  return 0;
}

Match matchCopy(Match original)
{
  RETURN_NULL_ON_NULL(original)

  Match match = (Match)malloc(sizeof(*match));
  RETURN_NULL_ON_NULL(match)

  match->first = original->first;
  match->second = original->second;
  match->winner = original->winner;
  match->duration = original->duration;
  match->tournament = original->tournament;
  
  return match;
}
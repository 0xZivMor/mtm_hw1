#include "tournament.h"
#include "matchnode.h"
#include "string.h"
#include "utils.h"
#include "map.h"
#include <stdlib.h>

struct tournament_t {
  int id;
  matchNode matches;
  Map scores;
  char *location;
  int max_matches_per_player;
  bool finished;
  int winner;
};

/**
 * Makes sure the both participants haven't reach the games limit in the 
 * tournament yet
 * 
 * @param tournament Tournament in question
 * @param player1 ID of first participant
 * @param player2 ID of second participant
 * @return CHESS_OUT_OF_MEMORY - memory error occured
 *         CHESS_INVALID_MAX_GAMES - one of the particiapnts has already 
 *                                   participated in maximum games allowed
 *         CHESS_SUCCESS - match was added successfully.
 */
static ChessResult verifyGamesLimit(Tournament tournament,
                                    int player1,
                                    int player2);

/**
 * Calculates the player's score in the tournament according to the following
 * key:
 *    win: +2 points
 *    draw: +1 point
 *    loss: no change
 * 
 * @param tournament Tournament in question
 * @param player ID of player in question
 * @return >=0 player's score; 
 *         <0 on memory error
 */
static int calcPlayerScore(Tournament tournament, int player);

Tournament tournamentCreate(int id, const char *location, int max_games_per_player)
{
  // checking for incorrect parameters
  if(!validateId(id) || !validateId(max_games_per_player) || 
     !validateLocation(location)) {
    return NULL;
  }
  
  Tournament tournament = (Tournament) malloc(sizeof(*tournament));
  RETURN_NULL_ON_NULL(tournament)

  tournament->id = id;
  tournament->matches = NULL;
  tournament->scores = mapCreate(copyInt, 
                                 copyInt, 
                                 freeInt, 
                                 freeInt, 
                                 idCompare);
  RETURN_NULL_ON_NULL(tournament->scores)

  int length = strlen(location) + 1;
  char *new_loc = (char *)malloc(sizeof(char) * length);
  
  if(NULL == new_loc) {
    tournamentDestroy(tournament);
    return NULL;
  }

  strcpy(new_loc, location);
  new_loc[length] = '\0';

  tournament->location = new_loc;
  tournament->max_matches_per_player = max_games_per_player;
  tournament->finished = false;
  tournament->winner = NULL;
}

ChessResult tournamentAddMatch(Tournament tournament, Match match)
{
  if(NULL == tournament || NULL == match) {
    return CHESS_NULL_ARGUMENT;
  }

  if (tournamentIsEnded(tournament)) {
    return CHESS_TOURNAMENT_ENDED;
  }

  // match already in the tournament
  if(matchNodeInList(tournament->matches, match)) {
    return CHESS_GAME_ALREADY_EXISTS;
  }

  int player1 = matchGetFirst(match), player2 = matchGetSecond(match);
  if (!tournamentIsParticipant(tournament, player1) ||
      !tournamentIsParticipant(tournament, player2)) {
    return CHESS_PLAYER_NOT_EXIST;
  }

  switch (verifyGamesLimit(tournament, player1, player2)) {
    case CHESS_OUT_OF_MEMORY:
      return CHESS_OUT_OF_MEMORY;
    case CHESS_INVALID_MAX_GAMES:
      return CHESS_INVALID_MAX_GAMES;
    default:
      break;
  }

  matchNode new_node = matchNodeCreate(match, tournament->matches); 
  if (NULL == new_node) {
    return CHESS_OUT_OF_MEMORY;
  } 
    
  tournament->matches = new_node;
  return CHESS_SUCCESS;
}

ChessResult tournamentEnd(Tournament tournament)
{
  RETUN_RESULT_ON_NULL(tournament)

  int *current_player_id, max_score = -1, *current_score;

  // going over the players
  MAP_FOREACH(int *, current_player_id, tournament->scores) {
    current_score = (int *)mapGet(tournament->scores, (MapKeyElement)current_player_id);

    // replace winner if we got a better result
    if (current_score > max_score) {
      max_score = current_score;
      tournament->winner = *current_player_id;
    } else if (current_score == max_score) {
      // choose the one with lower id
      if (tournament->winner > *current_player_id) {
        tournament->winner = *current_player_id;
      }
    }
    freeInt((MapKeyElement)current_player_id);
  }

  tournament->finished = true; // updating status
  return CHESS_SUCCESS;
}

ChessResult tournamentGetMatchesByPlayer(Tournament tournament, 
                                         int player_id, 
                                         matchNode *list)
{
  RETUN_RESULT_ON_NULL(tournament)

  if (!tournamentIsParticipant(tournament, player_id)) {
    *list = NULL;
    return CHESS_PLAYER_NOT_EXIST;
  }

  // if a player exists in a tournament, it must participate in atleast one match

  matchNode matches = tournament->matches, new_node = NULL;
  
  FOREACH_MATCH(matches) {
    Match match = matchNodeGetMatch(matches);
    if (matchIsParticipant(match, player_id)) {
      new_node = matchNodeCreate(match, new_node);
      if (NULL == new_node) {  // memory error
        matchNodeDestroyList(*list, false);
        *list = NULL;
        return CHESS_OUT_OF_MEMORY;
      }
      *list = new_node;
    }
  }
  return CHESS_SUCCESS;
}

void tournamentDestroy(Tournament tournament)
{
  if(tournament == NULL) {
    return;
  }

  matchNodeDestroyList(tournament->matches, false);
  mapDestroy(tournament->scores);
  free(tournament->location);
  free(tournament);
}

void tournamentDestroyMap(MapDataElement tournament)
{
  tournamentDestory((Tournament)tournament);
}

int tournamentCompare(MapKeyElement tournament1, MapKeyElement tournament2)
{
  COMPARE_NOT_NULL(tournament1, tournament2)

  return *(int *)tournament1 - *(int *)tournament2;
}

bool tournamentIsEnded(Tournament tournament)
{
  RETURN_NULL_ON_NULL(tournament)

  return tournament->finished;
}

bool tournamentIsParticipant(Tournament tournament, int player_id)
{
  return mapContains(tournament->scores, (MapKeyElement)&player_id);
}

MapDataElement tournamentCopy(MapDataElement original_tournament)
{
  Tournament original = (Tournament) original_tournament;
  RETURN_NULL_ON_NULL(original)
  
  Tournament new_tournament;
  new_tournament = tournamentCreate(original->id, 
                                    original->location, 
                                    original->max_matches_per_player);
  RETURN_NULL_ON_NULL(new_tournament)

  new_tournament->finished = original->finished;
  new_tournament->winner = original->winner;
  new_tournament->matches = original->matches;
  new_tournament->scores = original->scores;
  
  return new_tournament;
}

static ChessResult verifyGamesLimit(Tournament tournament,
                                    int player1,
                                    int player2)
{
  // creating list of matches played in the tournament by the players
  matchNode player1_matches, player2_matches;
  
  ChessResult first = tournamentGetMatchesByPlayer(tournament,
                                                   player1,
                                                   &player1_matches);
  if (CHESS_OUT_OF_MEMORY == first) {
    return CHESS_OUT_OF_MEMORY;
  }

  ChessResult second = tournamentGetMatchesByPlayer(tournament,
                                                    player2,
                                                    &player2_matches);
  if (CHESS_OUT_OF_MEMORY == second) {
    matchNodeDestroyList(player1_matches, false);
    return CHESS_OUT_OF_MEMORY;
  }

  ChessResult result = CHESS_SUCCESS;

  // over the limit matches by atleast one of the participants
  if (CHESS_SUCCESS == first && 
      matchNodeGetSize(player1_matches) <= tournament->max_matches_per_player) {
    result = CHESS_INVALID_MAX_GAMES;
  }
  if (CHESS_SUCCESS == second && 
      matchNodeGetSize(player2_matches) <= tournament->max_matches_per_player) {
    result = CHESS_INVALID_MAX_GAMES;
  }

  matchNodeDestroyList(player1_matches, false);
  matchNodeDestroyList(player2_matches, false);
  return result;
}

static int calcPlayerScore(Tournament tournament, int player)
{
  matchNode matches;
  if (CHESS_OUT_OF_MEMORY == tournamentGetMatchesByPlayer(tournament, 
                                                          player, 
                                                          &matches)) {
    return -1;
  }
  
  matchNode current = matches;
  int winner_id;
  int score = 0;

  FOREACH_MATCH(current) {
    if (CHESS_SUCCESS == matchGetWinner(matchNodeGetMatch(current), &winner_id)) {
      // draw
      if (!winner_id) {
        score += 1;

      // player won the match
      } else if (player == winner_id) {
        score += 2;
      }
    }
  }

  matchNodeDestroyList(matches, false);
  return score;
}
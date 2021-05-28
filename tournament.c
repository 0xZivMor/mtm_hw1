#include "chessSystem.h"
#include "tournament.h"
#include "matchnode.h"
#include "string.h"
#include "utils.h"
#include "map.h"
#include <stdlib.h>

struct tournament_t {
  ChessId id;
  matchNode matches;
  Map scores;
  char *location;
  int max_matches_per_player;
  bool finished;
  int winner;
};

/**
 * returns true if toCheck is in array, false otherwise
 */ 
static bool isInArray(int array[], int length, int toCheck);

/**
 * Makes sure the both participants haven't reach the games limit in the 
 * tournament yet
 * 
 * @param tournament Tournament in question
 * @param player1 ID of first participant
 * @param player2 ID of second participant
 * @return CHESS_OUT_OF_MEMORY - memory error occured
 *         CHESS_EXCEEDED_GAMES - one of the particiapnts has already 
 *                                participated in maximum games allowed
 *         CHESS_SUCCESS - match was added successfully.
 */
static ChessResult verifyGamesLimit(Tournament tournament,
                                    ChessId player1,
                                    ChessId player2);

/**
 * Adds a match's participants to the tournament if they aren't participating
 * already.
 * 
 * @param tournament Tournament to add to
 * @param player1 ID of the first player
 * @param player2 ID of the second player
 * @return CHESS_OUT_OF_MEMORY memory error occured; 
 *         CHESS_SUCCESS operation succeded.
 */
static ChessResult addPlayersIfNotParticipants(Tournament tournament, 
                                               ChessId player1, 
                                               ChessId player2);

/**
 * Updates a match participants scores in the tournament's scores map.
 * This private function assumes the match was validated.
 * 
 * @param tournament tournament in which the match took place
 * @param match validated match.
 * @return CHESS_OUT_OF_MEMORY memory error occured; 
 *         CHESS_SUCCESS players scores were updated successfully
 */
static ChessResult updatePlayersScores(Tournament tournament, Match match);

/**
 * Updates a player's score in the tournament after her opponent forfieted.
 * 
 * @param tournament Tournament in which the match took place
 * @param new_winner ID of the player whose score is to be updated
 * @param old_winner ID of the winner before the forfiet
 * @return CHESS_OUT_OF_MEMORY memory error occured; 
 *         CHESS_SUCCESS score was updated successfully
 */
static ChessResult updatePlayerScoreAfterForfiet(Tournament tournament, 
                                                 ChessId new_winner, 
                                                 ChessId old_winner);

Tournament tournamentCreate(ChessId id, const char *location, int max_games_per_player)
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
  tournament->scores = mapCreate(copyId, // copies int
                                 copyId, 
                                 freeId, // frees int
                                 freeId, 
                                 idCompare);
  RETURN_NULL_ON_NULL(tournament->scores)

  int length = strlen(location);
  char *new_loc = (char *)malloc(sizeof(char) * (length + 1));
  
  if(NULL == new_loc) {
    tournamentDestroy(tournament);
    return NULL;
  }

  strcpy(new_loc, location);
  new_loc[length] = '\0';

  tournament->location = new_loc;
  tournament->max_matches_per_player = max_games_per_player;
  tournament->finished = false;
  tournament->winner = 0;

  return tournament;
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

  ChessId player1 = matchGetFirst(match), player2 = matchGetSecond(match);
  if (CHESS_OUT_OF_MEMORY == addPlayersIfNotParticipants(tournament, 
                                                         player1, 
                                                         player2)) {
    return CHESS_OUT_OF_MEMORY;
  }

  switch (verifyGamesLimit(tournament, player1, player2)) {
    case CHESS_OUT_OF_MEMORY:
      return CHESS_OUT_OF_MEMORY;
    case CHESS_EXCEEDED_GAMES:
      return CHESS_EXCEEDED_GAMES;
    default:
      break;
  }

  matchNode new_node = matchNodeCreate(match, tournament->matches); 
  if (NULL == new_node) {
    return CHESS_OUT_OF_MEMORY;
  }

  if (CHESS_OUT_OF_MEMORY == updatePlayersScores(tournament, match)) {
    matchNodeDestroy(new_node, false);
    return CHESS_OUT_OF_MEMORY;
  }
    
  tournament->matches = new_node;
  return CHESS_SUCCESS;
}

ChessId tournamentGetWinner(Tournament tournament)
{
    RETURN_ZERO_ON_NULL(tournament);

    // no winner in unfinished tournament
    if (!tournament->finished) {
      return 0;
    }

    return tournament->winner;
}

char* tournamentGetLocation(Tournament tournament)
{
    RETURN_NULL_ON_NULL(tournament);
    return tournament->location;
}

ChessResult tournamentEnd(Tournament tournament)
{
  RETURN_RESULT_ON_NULL(tournament)
  
  if (tournament->finished) {
    return CHESS_TOURNAMENT_ENDED;
  }

  if (!tournamentNumberOfMatches(tournament)) {
    return CHESS_NO_GAMES;
  }

  int max_score = -1;

  // going over the players
  MAP_FOREACH(ChessId *, current_player_id, tournament->scores) {
    int *current_score = MAP_GET(tournament->scores, current_player_id, ChessId *);

    // replace winner if we got a better result
    if (*current_score > max_score) {
      max_score = *current_score;
      tournament->winner = *current_player_id;
    } else if (*current_score == max_score) {
      // choose the one with lower id
      if (tournament->winner > *current_player_id) {
        tournament->winner = *current_player_id;
      }
    }
    freeId((MapKeyElement)current_player_id);
  }

  tournament->finished = true; // updating status
  return CHESS_SUCCESS;
}

ChessResult tournamentGetMatchesByPlayer(Tournament tournament, 
                                         ChessId player_id, 
                                         matchNode *list)
{
  RETURN_RESULT_ON_NULL(tournament)

  // dummy value for list
  *list = matchNodeCreate(NULL, NULL);

  if (!tournamentIsParticipant(tournament, player_id)) {
    matchNodeDestroy(*list, false);
    *list = NULL;
    return CHESS_PLAYER_NOT_EXIST;
  }

  // compile a list of all the matches the player took part in
  FOREACH_MATCH(tournament->matches, current) {
    Match match = matchNodeGetMatch(current);
    
    if (matchIsParticipant(match, player_id)) {
      matchNode new_node = matchNodeCreate(match, *list);
      
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
  tournamentDestroy((Tournament)tournament);
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

bool tournamentIsParticipant(Tournament tournament, ChessId player_id)
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

  // no need to copy matches if there aren't any
  if (NULL != original->matches) {
    new_tournament->matches = (matchNode)matchNodeCopy(original->matches);
  }

  // destroy new map and copy old
  mapDestroy(new_tournament->scores);
  new_tournament->scores = mapCopy(original->scores);
  
  return new_tournament;
}

int tournamentLongestPlayTime(Tournament tournament)
{
  RETURN_ZERO_ON_NULL(tournament)
  int max = 0;

  FOREACH_MATCH(tournament->matches, node) {
    Match current = matchNodeGetMatch(node);
    if (max < matchGetDuration(current)) {
      max = matchGetDuration(current);
    }
  }
  return max;
}

int tournamentNumberOfMatches(Tournament tournament)
{
  RETURN_ZERO_ON_NULL(tournament)

  return matchNodeGetSize(tournament->matches);
}

int tournamentNumberOfPlayers(Tournament tournament)
{
  RETURN_ZERO_ON_NULL(tournament)

  matchNode matches = tournament->matches;
  int num_of_matches = matchNodeGetSize(matches);
  
  //creating a temporary array with enough space, assuming all players played once 
  ChessId *players = (ChessId *)malloc(sizeof(ChessId) * num_of_matches * 2);
  
  // initialize array to -1 to avoid errors
  for(int i = 0; i < num_of_matches * 2; i++) {
    players[i] = -1;
  }

  int index_for_players = 0;
  
  FOREACH_MATCH(tournament->matches, current) {
    Match match = matchNodeGetMatch(current);
    ChessId first = matchGetFirst(match);
    ChessId second = matchGetSecond(match);

    // first is not in the array already
    if (!isInArray(players, num_of_matches * 2, first)) {
      players[index_for_players] = first;
      index_for_players++;
    }

    // second is not in the array already
    if(!isInArray(players, num_of_matches*2, second)) {
      players[index_for_players] = second;
      index_for_players++;
    }
  }
  free(players);
  return index_for_players;
}

double tournamentAveragePlayTime(Tournament tournament)
{
  if(NULL == tournament) {
    return 0.0;
  }

  double total_time = matchNodeTotalTime(tournament->matches);
  double num_of_matches = matchNodeGetSize(tournament->matches);
  
  if(!num_of_matches) {
    return 0.0;
  }
  return total_time / num_of_matches;
}

ChessResult tournamentRemovePlayer(Tournament tournament, ChessId player_id, bool *removed)
{
  // arguments validation
  if (NULL == tournament || NULL == removed) {
    return CHESS_NULL_ARGUMENT;
  }

  if (!tournamentIsParticipant(tournament, player_id)) {
    return CHESS_PLAYER_NOT_EXIST;
  }

  *removed = false;

  // tournament has ended, can't remove player
  if (tournamentIsEnded(tournament)) {
    return CHESS_SUCCESS;
  }

  FOREACH_MATCH(tournament->matches, current) {
    Match match = matchNodeGetMatch(current);
    if (!matchIsParticipant(match, player_id)) {
      continue;
    }

    ChessId old_winner, new_winner;
    matchForfiet(match, player_id, &old_winner, &new_winner);
    if (CHESS_OUT_OF_MEMORY == updatePlayerScoreAfterForfiet(tournament, new_winner, old_winner)) {
      return CHESS_OUT_OF_MEMORY;
    }
  }
  
  mapRemove(tournament->scores, (MapKeyElement)&player_id);
  *removed = true;
  return CHESS_SUCCESS;
}

static bool isInArray(int array[], int length, int toCheck)
{
  for(int i = 0; i < length; i++) {
    if(array[i] == toCheck) {
      return true;
    }
  }
  return false;
}

static ChessResult updatePlayerScoreAfterForfiet(Tournament tournament, 
                                                 ChessId new_winner, 
                                                 ChessId old_winner)
{
  // winner wasn't changed
  if (old_winner == new_winner) {
    return CHESS_SUCCESS;
  }

  int change;

  // previous result was a draw
  if (!old_winner) {
    change = 1;
  } else {  // new winner previouly lost
    change = 2;
  }

  int score = *(MAP_GET(tournament->scores, &new_winner, int *)) + change;
  IF_MAP_PUT(tournament->scores, &new_winner, &score) {
    return CHESS_OUT_OF_MEMORY;
  }
  return CHESS_SUCCESS;
}

static ChessResult verifyGamesLimit(Tournament tournament,
                                    ChessId player1,
                                    ChessId player2)
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
      matchNodeGetSize(player1_matches) >= tournament->max_matches_per_player) {
    result = CHESS_EXCEEDED_GAMES;
  }
  if (CHESS_SUCCESS == second && 
      matchNodeGetSize(player2_matches) >= tournament->max_matches_per_player) {
    result = CHESS_EXCEEDED_GAMES;
  }

  matchNodeDestroyList(player1_matches, false);
  matchNodeDestroyList(player2_matches, false);
  return result;
}

static ChessResult updatePlayersScores(Tournament tournament, Match match)
{
  ChessId player1_id = matchGetFirst(match), player2_id = matchGetSecond(match);
  int player1_score = *(MAP_GET(tournament->scores, &player1_id, int *));
  int player2_score = *(MAP_GET(tournament->scores, &player2_id, int *));
  
  ChessId winner; 
  matchGetWinner(match, &winner);

  if (!winner) {  // draw
    player1_score++;
    player2_score++;
  } else if (winner == player1_id) {
    player1_score += 2;
  } else {
    player2_score += 2;
  }

  IF_MAP_PUT(tournament->scores, &player1_id, &player1_score) {
    return CHESS_OUT_OF_MEMORY;
  }

  /**
   * if memory error occures here, we can't undo the change to player1's
   * score because we're already out of memory
   */
  IF_MAP_PUT(tournament->scores, &player2_id, &player2_score) {
    return CHESS_OUT_OF_MEMORY;
  }

  return CHESS_SUCCESS;
}

static ChessResult addPlayersIfNotParticipants(Tournament tournament, 
                                               ChessId player1, 
                                               ChessId player2)
{
  bool player1_added = false;
  int zero = 0;

  if (!tournamentIsParticipant(tournament, player1)) {
    IF_MAP_PUT(tournament->scores, &player1, &zero) {
      return CHESS_OUT_OF_MEMORY;
    }
    player1_added = true;
  }

  if (!tournamentIsParticipant(tournament, player2)) {
    IF_MAP_PUT(tournament->scores, &player2, &zero) {
      // player1 was added to the tournament on this call, remove it
      if (player1_added) {
        mapRemove(tournament->scores, (MapKeyElement)&player1);
      }
      return CHESS_OUT_OF_MEMORY;
    }
  }

  return CHESS_SUCCESS;
}


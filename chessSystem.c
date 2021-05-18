#include <stdlib.h>
#include "chessSystem.h"
#include "tournament.h"
#include "matchnode.h"
#include "match.h"
#include "utils.h"
#include "map.h"

struct chess_system_t
{
  Map tournaments;
  Map players;
  matchNode matches;
};

/**
 * Returns the winning player according to the winner argument
 * 
 * @param first_player first participant's id
 * @param second_player second participant's id
 * @param winner winner indicator
 * @return
 *    Winning player's ID (0 in case of draw)
 */
static int getWinner(int first_player, int second_player, Winner winner);

/**
 * Removes and destroys all matches that were part of the tournament from
 * the system. Removes matches from the relevant participants matches lists.
 * 
 * @param chess chess system to remove the Matches from
 * @param tournament Tournament to be removed
 */
static void chessRemoveMatchesByTournament(ChessSystem chess, int tournament);

/**
 * Adds a match to all relevant data structures.
 * 
 * @param chess chess system that contains the match
 * @param tournament tournament of which the match was a part of 
 * @param match Match to add
 * @return  CHESS_GAME_ALREADY_EXIST - if there is already a game in the 
 *                                     tournament with the same two players.
 *          CHESS_OUT_OF_MEMORY - if memory error occured.
 *          CHESS_SUCCESS - if game was added successfully.

 */
static ChessResult addMatch(ChessSystem chess, Tournament tournament, Match match);

ChessSystem chessCreate()
{
  ChessSystem chess = (ChessSystem)malloc(sizeof(*chess));

  if (NULL == chess) {
    return NULL;
  }

  chess->tournaments = mapCreate(tournamentCopy, 
                                 copyInt, 
                                 tournamentDestroyMap, 
                                 freeInt, 
                                 tournamentCompare);
  if (NULL == chess->tournaments) {
    return NULL;
  }

  chess->players = mapCreate(matchNodeCopy, 
                             copyInt, 
                             matchNodeDestroyMap, 
                             freeInt, 
                             idCompare);
  if (NULL == chess->players) {
    return NULL;
  }

  chess->matches = NULL;
}

#define VALIDATE_ID(arg)      \
  if (!validateId(arg)) {     \
    return CHESS_INVALID_ID;  \
  }

#define GET_TOURNAMENT(tournament_id, tournament)                         \
  tournament = mapGet(chess->tournaments, (MapKeyElement)&tournament_id); \
  if (NULL == tournament) {                                               \
    return CHESS_TOURNAMENT_NOT_EXIST;                                    \
  }

#define GET_PLAYER(player_id, player)                         \
  player = mapGet(chess->players, (MapKeyElement)&player_id); \
  if (NULL == player) {                                       \
    return CHESS_PLAYER_NOT_EXIST;                            \
  }


void chessDestroy(ChessSystem chess)
{
  mapDestroy(chess->players);
  mapDestroy(chess->tournaments);
  matchNodeDestroyList(chess->matches, true);

  free(chess);
}

ChessResult chessAddTournament(ChessSystem chess,
                               int tournament_id,
                               int max_games_per_player,
                               const char *tournament_location)
{
  if (NULL == chess || NULL == tournament_location) {
    return CHESS_NULL_ARGUMENT;
  }

  VALIDATE_ID(tournament_id)

  if (!validateLocation(tournament_location)) {
    return CHESS_INVALID_LOCATION;
  }

  // tournament with that id was already added
  if (mapContains(chess->tournaments, &tournament_id)) {
    return CHESS_TOURNAMENT_ALREADY_EXISTS;
  }

  Tournament tournament = tournamentCreate(tournament_id,
                                           tournament_location,
                                           max_games_per_player);

  if (NULL == tournament) {
    return CHESS_OUT_OF_MEMORY;
  }

  // all parameters are certainly not null so an error must be memory related
  if (MAP_SUCCESS != mapPut(chess->tournaments, 
                            (MapKeyElement) &tournament_id, 
                            (MapDataElement) tournament)) {
    return CHESS_OUT_OF_MEMORY;
  }

  return CHESS_SUCCESS;
}

ChessResult chessAddGame(ChessSystem chess, int tournament_id, int first_player,
                         int second_player, Winner winner, int play_time)
{
  RETUN_RESULT_ON_NULL(chess)

  if (!validateId(tournament_id) || 
      !validateId(first_player) || 
      !validateId(second_player) ||
      first_player == second_player ||
      winner > DRAW) {
    return CHESS_INVALID_ID;
  }

  if (play_time < 0) {
    return CHESS_INVALID_PLAY_TIME;
  }

  Tournament tournament;
  GET_TOURNAMENT(tournament_id, tournament)

  // if a player was created "without need" and will be never used again,
  // it will be freed when the chess instance is destroyed.
  
  int player_winner = getWinner(first_player, second_player, winner);
  Match match = matchCreate(first_player, 
                            second_player, 
                            player_winner, 
                            tournament_id, 
                            play_time);

  if (NULL == match) {
    return CHESS_OUT_OF_MEMORY;
  }

  return addMatch(chess, tournament, match);
}

ChessResult chessRemoveTournament(ChessSystem chess, int tournament_id)
{
  RETUN_RESULT_ON_NULL(chess)
  VALIDATE_ID(tournament_id)

  Tournament tournament;
  GET_TOURNAMENT(tournament_id, tournament)

  chessRemoveMatchesByTournament(chess, tournament);
  tournamentDestroy(tournament);
  mapRemove(chess->tournaments, (MapKeyElement) &tournament_id);
  return CHESS_SUCCESS;
}

ChessResult chessRemovePlayer(ChessSystem chess, int player_id)
{
  RETUN_RESULT_ON_NULL(chess)
  VALIDATE_ID(player_id)

  if (!mapContains(chess->players, (MapKeyElement)&player_id)) {
    return CHESS_PLAYER_NOT_EXIST;
  }

  int *current_tournament;
  Tournament tournament;
  bool removed = false;
  MAP_FOREACH(int *, current_tournament, chess->tournaments) {
    tournament = mapGet(chess->tournaments, current_tournament);
    removed |= tournamentRemovePlayer(tournament, player_id);
    freeInt(current_tournament);
  }

  if (removed) { // player was successfully removed from all tournaments
    mapRemove(chess->players, (MapKeyElement) &player_id);
  }
  return CHESS_SUCCESS;
}

ChessResult chessEndTournament(ChessSystem chess, int tournament_id)
{
  RETUN_RESULT_ON_NULL(chess)
  VALIDATE_ID(tournament_id)

  Tournament tournament;
  GET_TOURNAMENT(tournament_id, tournament)

  return tournamentEnd(tournament);
}

double chessCalculateAveragePlayTime(ChessSystem chess, int player_id, ChessResult* chess_result)
{
  if (NULL == chess_result) {
    return 0.0;
  }
  if (NULL == chess) {
    *chess_result = CHESS_NULL_ARGUMENT;
    return 0.0;
  }

  if (!validateId(player_id)) {
    *chess_result = CHESS_PLAYER_NOT_EXIST;
    return 0.0;
  }

  if (!mapContains(chess->players, (MapKeyElement)&player_id)) {                                      
    *chess_result = CHESS_PLAYER_NOT_EXIST;
    return 0.0;                            
  }
}

static int getWinner(int first_player, int second_player, Winner winner)
{
  switch (winner) {
  case FIRST_PLAYER:
    return first_player;
  case SECOND_PLAYER:
    return second_player;
  default:
    return 0;
  }
}

// all destroy functions will do nothing if NULL is passed 
#define MEMORY_ERROR(node1, node2, node3, match)  \
  matchNodeDestroy(node1, false);                 \
  matchNodeDestroy(node2, false);                 \
  matchNodeDestroy(node3, false);                  \
  matchDestroy(match);                            \
  return CHESS_OUT_OF_MEMORY;

static ChessResult addMatch(ChessSystem chess, Tournament tournament, Match match)
{
  if (tournamentAddMatch(tournament, match) == CHESS_GAME_ALREADY_EXISTS) {
    matchDestroy(match);
    return CHESS_GAME_ALREADY_EXISTS;
  }

  int first = matchGetFirst(match), second = matchGetSecond(match);
  matchNode matches_node = matchNodeCreate(match, chess->matches);
  matchNode p1_node = matchNodeCreate(match, 
                                      (matchNode)mapGet(chess->players, 
                                                        first));
  matchNode p2_node = matchNodeCreate(match, 
                                      (matchNode)mapGet(chess->players, 
                                                        second));
  
  // failed to allocate new node
  if (NULL == matches_node || NULL == p1_node || NULL == p2_node) {
    MEMORY_ERROR(matches_node, p1_node, p2_node, match)
  }

  matchNode old = (matchNode)mapGet(chess->players, first);
  if (MAP_SUCCESS != mapPut(chess->players, first, (MapDataElement)p1_node)) {
    MEMORY_ERROR(matches_node, p1_node, p2_node, match)
  }
  
  if (MAP_SUCCESS != mapPut(chess->players, second, (MapDataElement)p2_node)) {
    // attempt to undo the changes in the first players list, may not work
    // because we already have memory issues
    mapPut(chess->players, first, old);
    MEMORY_ERROR(matches_node, p1_node, p2_node, match)
  }
  
  chess->matches = matches_node;
  return CHESS_SUCCESS;
}
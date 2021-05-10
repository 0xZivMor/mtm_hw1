#include <stdlib.h>
#include "chessSystem.h"
#include "tournament.h"
#include "matchnode.h"
#include "player.h"
#include "match.h"
#include "utils.h"
#include "map.h"

struct chess_system_t
{
  Map tournaments;
  Map players;
  matchNode matches;
};

static MapKeyElement copyId(MapKeyElement element);
static void freeId(MapKeyElement element);

/**
 * Validates that the provided location string is in compliance
 * with the location demands:
 *    Not empty
 *    Starts with a capital letter
 *    All following characters are small letters and spaces
 * 
 * @param location the location string
 * @return true location is valid
 * @return false location is invalid
 */
static bool validateLocation(const char *location);

/**
 * Confirms that provided tournament's or player's ID are in compliance with
 * the specification: > 0
 * 
 * @param id ID to validate
 * @return true ID is valid
 * @return false ID is not valid
 */
static inline bool validateId(int id);

/**
 * Returns the winning player according to the winner argument
 * 
 * @param first_player first participant 
 * @param second_player second participant
 * @param winner winner indicator
 * @return
 *    Winning Player (NULL in case of draw)
 */
static Player getWinner(Player first_player, Player second_player, Winner winner);

/**
 * Removes and destroys all matches that were part of the tournament from
 * the system. Removes matches from the relevant participants matches lists.
 * 
 * @param chess chess system to remove the Matches from
 * @param tournament Tournament to be removed
 */
static void chessRemoveMatchesByTournament(ChessSystem chess, Tournament tournament);

ChessSystem chessCreate()
{
  ChessSystem chess = (ChessSystem)malloc(sizeof(struct chess_system_t));

  if (NULL == chess) {
    return NULL;
  }

  chess->tournaments = mapCreate(tournamentCopy, 
                                 copyId, 
                                 tournamentDestroy, 
                                 freeId, 
                                 tournamentCompare);
  if (NULL == chess->tournaments) {
    return NULL;
  }

  chess->players = mapCreate(playerCopy, 
                             copyId, 
                             playerDestroy, 
                             freeId, 
                             playerCompare);
  if (NULL == chess->players) {
    return NULL;
  }

  chess->matches = NULL;
}

#define NOT_NULL(arg)           \
  if (NULL == arg) {            \
    return CHESS_NULL_ARGUMENT; \
  }

#define VALIDATE_ID(arg)      \
  if (!validateId(arg)) {     \
    return CHESS_INVALID_ID;  \
  }

#define GET_TOURNAMENT(tournament_id, tournament)         \
  tournament = mapGet(chess->tournaments, tournament_id); \
  if (NULL == tournament) {                               \
    return CHESS_TOURNAMENT_NOT_EXIST;                    \
  }

#define GET_PLAYER(player_id, player)                         \
  player = mapGet(chess->players, (MapKeyElement)&player_id); \
  if (NULL == player) {                                       \
    return CHESS_PLAYER_NOT_EXIST;                            \
  }

// TODO destory on memory error macro

void chessDestroy(ChessSystem chess)
{
  mapDestroy(chess->players);
  mapDestroy(chess->tournaments);

  matchNode head = chess->matches, next;
  while (NULL != head) {
    next = nextMatchNode(head);
    matchNodeDestroy(head, true);
    head = next;
  }

  free(chess);
}

ChessResult chessAddTournament(ChessSystem chess,
                               int tournament_id,
                               int max_games_per_player,
                               const char *tournament_location)
{
  if ((NULL == chess) || (NULL == tournament_location)) {
    return CHESS_NULL_ARGUMENT;
  }

  VALIDATE_ID(tournament_id)

  if (!validateLocation(tournament_location)) {
    return CHESS_INVALID_LOCATION;
  }

  // tournament with that id was already added
  if (mapContains(chess->tournaments, tournament_id)) {
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

#define GET_CREATE_PLAYER(player_id, player)                                        \
  player = mapGet(chess->players, (MapKeyElement)&player_id);                       \
  if (NULL == player) {                                                             \
    player = playerCreate(first_player);                                            \
    if (NULL == player) {                                                           \
      return CHESS_OUT_OF_MEMORY;                                                   \
    }                                                                               \
    if (MAP_SUCCESS != mapPut(chess->players, (MapKeyElement)&player_id, player)) { \
      playerDestroy(player, false);                                                 \
      return CHESS_OUT_OF_MEMORY;                                                   \
    }                                                                               \
  }

ChessResult chessAddGame(ChessSystem chess, int tournament_id, int first_player,
                         int second_player, Winner winner, int play_time)
{
  NOT_NULL(chess)

  if (!validateId(tournament_id) || 
      !validateId(first_player) || 
      !validateId(second_player) ||
      winner > DRAW) {
    return CHESS_INVALID_ID;
  }

  Tournament tournament;
  GET_TOURNAMENT(tournament_id, tournament)

  Player player1, player2;
  
  GET_CREATE_PLAYER(first_player, player1)
  GET_CREATE_PLAYER(second_player, player2)

  // if a player was created "without need" and will be never used again,
  // it will be freed when the chess instance is destroyed.
  
  Player player_winner = getWinner(player1, player2, winner);

  Match match = matchCreate(player1, player2, player_winner, tournament, play_time);

  if (NULL == match) {
    return CHESS_OUT_OF_MEMORY;
  }

  if (tournamentAddMatch(tournament, match) == CHESS_GAME_ALREADY_EXISTS) {
    matchDestroy(match);
    return CHESS_GAME_ALREADY_EXISTS;
  }

  matchNode node = newMatchNode(match, chess->matches);
  if (NULL == node) {  // failed to allocate new node
    matchDestroy(match);

    return CHESS_OUT_OF_MEMORY;
  }

  chess->matches = node;
  
  if (CHESS_OUT_OF_MEMORY == playerAddMatch(player1, match) ||
      CHESS_OUT_OF_MEMORY == playerAddMatch(player2, match)) {
    chessDestroy(chess);
    return CHESS_OUT_OF_MEMORY;
  }

  return CHESS_SUCCESS;
}

ChessResult chessRemoveTournament(ChessSystem chess, int tournament_id)
{
  NOT_NULL(chess)
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
  NOT_NULL(chess)
  VALIDATE_ID(player_id)

  Player player;
  GET_PLAYER(player_id, player)

  playerDestroy(player, true);
  mapRemove(chess->players, (MapKeyElement) &player_id);
  return CHESS_SUCCESS;
}

ChessResult chessEndTournament (ChessSystem chess, int tournament_id)
{
  NOT_NULL(chess)
  VALIDATE_ID(tournament_id)

  Tournament tournament;
  GET_TOURNAMENT(tournament_id, tournament)

  return tournamentEnd(tournament);
}

double chessCalculateAveragePlayTime (ChessSystem chess, int player_id, ChessResult* chess_result)
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

  Player player;
  player = mapGet(chess->players, (MapKeyElement)&player_id);
  if (NULL == player) {                                      
    *chess_result = CHESS_PLAYER_NOT_EXIST;
    return 0.0;                            
  }


}

static Player getWinner(Player first_player, Player second_player, Winner winner)
{
  switch (winner) {
  case FIRST_PLAYER:
    return first_player;
  case SECOND_PLAYER:
    return second_player;
  }
  return NULL;
}
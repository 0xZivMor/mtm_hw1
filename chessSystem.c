#include <stdlib.h>
#include "chessSystem.h"
#include "tournament.h"
#include "matchnode.h"
#include "match.h"
#include "utils.h"
#include "map.h"

struct chess_system_t {
  Map tournaments;
  Map players;
  matchNode matches;
};


/**
 * Private struct used to compile the players rating file
 */
typedef struct player_rating_t {
  ChessId player;
  double rating;
} PlayerRating;

/**
 * Returns the winning player according to the winner argument
 * 
 * @param first_player first participant's id
 * @param second_player second participant's id
 * @param winner winner indicator
 * @return
 *    Winning player's ID (0 in case of draw)
 */
static ChessId getWinner(ChessId first_player, ChessId second_player, Winner winner);

/**
 * Removes and destroys all matches that were part of the tournament from
 * the system. Removes matches from the relevant participants matches lists.
 * 
 * @param chess chess system to remove the Matches from
 * @param tournament Tournament to be removed
 */
static void chessRemoveMatchesByTournament(ChessSystem chess, ChessId tournament);

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

/**
 * Calculates player's level in the system.
 *
 * 
 * @param player_id player whose level is to be calculated
 * @param matches List of player's matches
 * @return player's level
 */
static double calcLevel(ChessId player_id, matchNode matches);

/**
 * Populates the provided array of PlayerRatings and sorts it with accordance
 * to the specifications: Decending order of levels, ascending order of 
 * IDs when levels are the same
 * 
 * @param chess Chess system involved
 * @param player_ratings array of PlayerRatings
 * @param length size fo the provided array
 */
static void sortPlayersByRating(ChessSystem chess, PlayerRating *player_ratings, int length);

/**
 * Swaps two PlayerRatings in an array 
 */
static void swap(PlayerRating *p1, PlayerRating *p2);

/**
 * Compares two PlayerRating instances, with "greater" defined as:
 * - has higher level
 * - has the lower ID if levels equal
 * 
 * @param first Pointer to first PlayerRating to compare
 * @param second Pointer to second PlayerRating to compare
 * @return >0 first is greater; 
 *         <0 second is greater; 
 *         0 ratings of the same player 
 */
static int ratingCompare(PlayerRating *first, PlayerRating *second);

ChessSystem chessCreate()
{
  ChessSystem chess = (ChessSystem)malloc(sizeof(*chess));
  RETURN_NULL_ON_NULL(chess)

  chess->tournaments = mapCreate(tournamentCopy, 
                                 copyId, 
                                 tournamentDestroyMap, 
                                 freeId, 
                                 idCompare);
  RETURN_NULL_ON_NULL(chess->tournaments)

  chess->players = mapCreate(matchNodeCopy, 
                             copyId, 
                             matchNodeDestroyMap, 
                             freeId, 
                             idCompare);
  RETURN_NULL_ON_NULL(chess->players)

  chess->matches = NULL;
  return chess;
}

#define VALIDATE_ID(arg)      \
  if (!validateId(arg)) {     \
    return CHESS_INVALID_ID;  \
  }

#define GET_TOURNAMENT(tournament_id, tournament)                         \
  tournament = MAP_GET(chess->tournaments, &tournament_id, Tournament);   \
  if (NULL == tournament) {                                               \
    return CHESS_TOURNAMENT_NOT_EXIST;                                    \
  }

#define GET_PLAYER(player_id, player)                         \
  player = MAP_GET(chess->players, &player_id, ChessId *);        \
  if (NULL == player) {                                       \
    return CHESS_PLAYER_NOT_EXIST;                            \
  }


void chessDestroy(ChessSystem chess)
{
  if (NULL == chess) {
    return;
  }

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

  // tournament with that id was already added
  if (mapContains(chess->tournaments, &tournament_id)) {
    return CHESS_TOURNAMENT_ALREADY_EXISTS;
  }

  if (!validateLocation(tournament_location)) {
    return CHESS_INVALID_LOCATION;
  }

  if (max_games_per_player <= 0) {
    return CHESS_INVALID_MAX_GAMES;
  }

  Tournament tournament = tournamentCreate(tournament_id,
                                           tournament_location,
                                           max_games_per_player);

  if (NULL == tournament) {
    return CHESS_OUT_OF_MEMORY;
  }

  // all parameters are certainly not null so an error must be memory related
  IF_MAP_PUT(chess->tournaments, &tournament_id, tournament) {
    tournamentDestroy(tournament);
    return CHESS_OUT_OF_MEMORY;
  }

  tournamentDestroy(tournament);
  return CHESS_SUCCESS;
}

// dummy node as the first node in the matches list
#define ADD_PLAYER_IF_NEW(chess, player)                                \
   if (!mapContains(chess->players, (MapKeyElement)&player)) {          \
    IF_MAP_PUT(chess->players, &player, matchNodeCreate(NULL, NULL)) {  \
      return CHESS_OUT_OF_MEMORY;                                       \
    }                                                                   \
  }
ChessResult chessAddGame(ChessSystem chess, int tournament_id, int first_player,
                         int second_player, Winner winner, int play_time)
{
  RETURN_RESULT_ON_NULL(chess)

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

  // players seen for the first time
  ADD_PLAYER_IF_NEW(chess, first_player)
  ADD_PLAYER_IF_NEW(chess, second_player)

  ChessId player_winner = getWinner(first_player, second_player, winner);
  Match match = matchCreate(first_player,
                            second_player,
                            player_winner,
                            tournament_id,
                            play_time);

  if (NULL == match) {
    return CHESS_OUT_OF_MEMORY;
  }

  ChessResult result = addMatch(chess, tournament, match);
  if (CHESS_SUCCESS != result) {
    matchDestroy(match);
  }

  return result;
}

ChessResult chessRemoveTournament(ChessSystem chess, int tournament_id)
{
  RETURN_RESULT_ON_NULL(chess)
  VALIDATE_ID(tournament_id)

  Tournament tournament;
  GET_TOURNAMENT(tournament_id, tournament)

  chessRemoveMatchesByTournament(chess, tournament_id);
  mapRemove(chess->tournaments, (MapKeyElement) &tournament_id);
  return CHESS_SUCCESS;
}

ChessResult chessRemovePlayer(ChessSystem chess, int player_id)
{
  RETURN_RESULT_ON_NULL(chess)
  VALIDATE_ID(player_id)

  if (!mapContains(chess->players, (MapKeyElement)&player_id)) {
    return CHESS_PLAYER_NOT_EXIST;
  }

  MAP_FOREACH(ChessId *, current_tournament, chess->tournaments) {
    Tournament tournament = MAP_GET(chess->tournaments, current_tournament, Tournament);
    freeId(current_tournament);
    
    if (CHESS_OUT_OF_MEMORY == tournamentRemovePlayer(tournament, player_id)) {
      return CHESS_OUT_OF_MEMORY;
    }
  }

    mapRemove(chess->players, (MapKeyElement) &player_id);
  return CHESS_SUCCESS;
}

ChessResult chessEndTournament(ChessSystem chess, int tournament_id)
{
  RETURN_RESULT_ON_NULL(chess)
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
    *chess_result = CHESS_INVALID_ID;
    return 0.0;
  }

  if (!mapContains(chess->players, (MapKeyElement)&player_id)) {                                      
    *chess_result = CHESS_PLAYER_NOT_EXIST;
    return 0.0;                            
  }

  double total_time = 0.0, number_of_games = 0.0;
  bool player_found = false;

  // going over all of the tournaments and taking matches played by player_id
  MAP_FOREACH(ChessId *, current, chess->tournaments)
  {
    matchNode matches_to_calculate;
    Tournament tournament = MAP_GET(chess->tournaments, current, Tournament);
    ChessResult result = tournamentGetMatchesByPlayer(tournament, player_id, &matches_to_calculate);
    freeId(current); // free the key copy
    
    switch (result) {
      case CHESS_SUCCESS:
        //calculating number of games played and total time by player_id
        player_found = true;
        number_of_games += matchNodeGetSize(matches_to_calculate);
        total_time += matchNodeTotalTime(matches_to_calculate);
        break;
      case CHESS_OUT_OF_MEMORY:
        // all resources are free, report error and exit
        *chess_result = CHESS_OUT_OF_MEMORY;
        return 0.0;
      default:
        continue;
    }
  }

  if (!player_found) {
    *chess_result = CHESS_PLAYER_NOT_EXIST;
    return 0.0;
  }
  
  if(!number_of_games) {
    return 0.0;
  }

  *chess_result = CHESS_SUCCESS;
  return total_time / number_of_games; //the average play time
}

ChessResult chessSavePlayersLevels(ChessSystem chess, FILE* file)
{
  if (NULL == chess || NULL == file) {
    return CHESS_NULL_ARGUMENT;
  }

  int players_count = mapGetSize(chess->players);
  PlayerRating *ratings = malloc(sizeof(*ratings) * players_count);

  sortPlayersByRating(chess, ratings, players_count);

  for (int i = 0; i < players_count; i++) {
    int result = fprintf(file, 
                         "%d %.2f\n", 
                         ratings[i].player, 
                         ratings[i].rating);
    if (result < 0) {
      free(ratings);
      return CHESS_SAVE_FAILURE;
    }
  }

  free(ratings);
  return CHESS_SUCCESS;
}

ChessResult chessSaveTournamentStatistics(ChessSystem chess, char* path_file)
{
  RETURN_RESULT_ON_NULL(chess);

  FILE *stats_file;
  stats_file = fopen(path_file, "w");
  if (NULL == stats_file) {
    return CHESS_SAVE_FAILURE;
  }

  bool no_tourmanet_ended = true;
  
  MAP_FOREACH(ChessId *, tournament_id, chess->tournaments) {
    Tournament current = MAP_GET(chess->tournaments, tournament_id, Tournament);
    freeId(tournament_id);

    // adding to stats only if tournament is over
    if(!tournamentIsEnded(current)) {
      continue;
    }

    no_tourmanet_ended = false;
    ChessId winner = tournamentGetWinner(current);
    int longest_game = tournamentLongestPlayTime(current);
    int num_of_matches = tournamentNumberOfMatches(current);
    int num_of_players = tournamentNumberOfPlayers(current);
    char *location = tournamentGetLocation(current);
    double average_game_time = tournamentAveragePlayTime(current);

    int result = fprintf(stats_file, 
                     "%d\n%d\n%.2f\n%s\n%d\n%d\n",
                     winner, longest_game, average_game_time, location,
                     num_of_matches, num_of_players);
    if (result < 0) { // writing to file error
      return CHESS_SAVE_FAILURE;
    }
  }

  if (no_tourmanet_ended) {
    return CHESS_NO_TOURNAMENTS_ENDED;
  }

  return CHESS_SUCCESS;
}

static ChessId getWinner(ChessId first_player, ChessId second_player, Winner winner)
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
  switch (tournamentAddMatch(tournament, match)) {
  case CHESS_GAME_ALREADY_EXISTS:
    return CHESS_GAME_ALREADY_EXISTS;
  case CHESS_TOURNAMENT_ENDED:
    return CHESS_TOURNAMENT_ENDED;
  case CHESS_EXCEEDED_GAMES:
    return CHESS_EXCEEDED_GAMES;
  default:
    break;
  }

  ChessId first = matchGetFirst(match), second = matchGetSecond(match);
  matchNode matches_node = matchNodeCreate(match, chess->matches);
  matchNode p1_node = matchNodeCreate(match, MAP_GET(chess->players, &first, matchNode));
  matchNode p2_node = matchNodeCreate(match, MAP_GET(chess->players, &second, matchNode));
  
  // failed to allocate new node
  if (NULL == matches_node || NULL == p1_node || NULL == p2_node) {
    MEMORY_ERROR(matches_node, p1_node, p2_node, match)
  }

  IF_MAP_PUT(chess->players, &first, p1_node) {
    MEMORY_ERROR(matches_node, p1_node, p2_node, match)
  }
  
  IF_MAP_PUT(chess->players, &second, p2_node) {
   /**
   * if memory error occures here, we can't undo the change to player1's
   * matches because we're already out of memory
   */
    MEMORY_ERROR(matches_node, p1_node, p2_node, match)
  }
  
  chess->matches = matches_node;
  matchNodeDestroy(p1_node, false);
  matchNodeDestroy(p2_node, false);
  return CHESS_SUCCESS;
}

static double calcLevel(ChessId player_id, matchNode matches) {
  
  double score = 0.0;
  int number_of_games = matchNodeGetSize(matches);

  if (!number_of_games) {
    return 0.0;
  }

  FOREACH_MATCH(matches, current) {
    Match match = matchNodeGetMatch(current);
    if (NULL == match) {
      continue;
    }
    ChessId winner;
    matchGetWinner(match, &winner);

    if (winner == player_id) {
      score += 6.0;
    } else if (winner == 0) { // draw
      score += 2.0;
    } else {  // player lost
      score -= 10;
    }
  }

  return score / (double)number_of_games; 
}

static void chessRemoveMatchesByTournament(ChessSystem chess, ChessId tournament) {

  matchNode node = chess->matches;
  Match match;

  // iterating manually because the list is changed
  while (NULL != node) {
    match = matchNodeGetMatch(node);
    node = matchNodeNext(node);

    matchNodeRemove(&(chess->matches), match);  // remove the match from the matches list
    matchDestroy(match); // this is the only place where we destroy matches
  }
}

static void sortPlayersByRating(ChessSystem chess, PlayerRating player_ratings[], int length)
{
  int i = 0;
  MAP_FOREACH(ChessId *, player, chess->players) {
    PlayerRating *rating = player_ratings + i++;
    matchNode matches = MAP_GET(chess->players, player, matchNode);
    rating->player = *player;
    rating->rating = calcLevel(*player, matches);
    freeId(player);
  }

  // bubble sort the array
  bool swapped = true;
  while (swapped) {
    swapped = false;

    for (int j = 1; j < length; j++) {
      if (ratingCompare(&player_ratings[j-1], &player_ratings[j]) < 0) {
        swap(player_ratings + j, player_ratings + (j - 1));
        swapped = true;
      }
    }
  }
}

static void swap(PlayerRating *p1, PlayerRating *p2)
{
  PlayerRating tmp = *p2;
  *p2 = *p1;
  *p1 = tmp;
}

static int ratingCompare(PlayerRating *first, PlayerRating *second)
{
  if (first->player == second->player) {
    return 0;
  }

  // both players has the same rating, ID tie break
  if (first->rating == second->rating) {
    return (int) (second->player - first->player);
  }

  return (int) (first->rating - second->rating);
}


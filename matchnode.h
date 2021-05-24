#ifndef _MATCHNODE_H
#define _MATCHNODE_H

#include "match.h"
#include "map.h"

typedef struct match_node_t *matchNode;

/**
 * Creates a new matchNode and adds it as the previous node for the provided 
 * next node
 * 
 * @param match Match to be added to the list
 * @param next matchNode to point to as next. May be NULL
 * @return 
 *    NULL if match was NULL or memory failure occured
 *    new matchNode otherwise 
 */
matchNode matchNodeCreate(Match match, matchNode next);

/**
 * Creates a new matchNode and adds it as the next node for the provided 
 * previous node
 * 
 * @param match Match to be added to the list as next 
 * @param previous matchNode to point to as previous. 
 * @return 
 *    NULL if match was NULL or memory failure occured
 *    the new matchNode otherwise 
 */
matchNode matchNodeAdd(matchNode previous, Match match);

/**
 * Gets the next node in the list
 * 
 * @param node current node
 * @return 
 *    next matchNode (may be NULL)
 */
matchNode matchNodeNext(matchNode node);

/**
 * returns the total sum of all durations from all matches in the list. usefull for calculating statistics.
 * 
 * @param list the list in question
 * @return
 *    total sum of duration, 0 if list is empty / NULL
 */
int matchNodeTotalTime(matchNode list);

/**
 * returns the size of the list from the given node untill reaching NULL
 * 
 * @param list the list given
 * @return 
 *    how many nodes are in the list, 
 *    0 if list is empty or NULL argument
 */
int matchNodeGetSize(matchNode list);

/**
 * Get the Match from matchNode object
 * 
 * @param node matchNode in question
 * @return
 *    Match contained in the node (may be NULL if node includes NULL or is NULL itself)
 */
Match matchNodeGetMatch(matchNode node);

/**
 * Removes the provided match from the list. If match is not found in the list, 
 * does nothing
 * 
 * @param list First matchNode in the list
 * @param match Match to be removed  
 */
void matchNodeRemove(matchNode list, Match match);

/**
 * Destroys the matchNode and (if instructed) the contained Match
 * 
 * @param node matchNode to destroy
 * @param destory_match if true, destorys the Match in the node as well
 */
void matchNodeDestroy(matchNode node, bool destory_match);

/**
 * Destroys the list of matchNode whose head and (if instructed) the contained 
 * Match in each node
 * 
 * @param node head of list to be destroyed
 * @param destory_match if true, destorys the Match in the nodes as well
 */
void matchNodeDestroyList(matchNode head, bool destory_match);

/**
 * Sets the list provided in addition as the next pointer of the provided
 * dest node.
 * 
 * @param dest - matchNode whose next value will be set
 * @param addition - matchNode that will be appended to dest
 * 
 */
void matchNodeConcat(matchNode dest, matchNode addition);

/**
 * Checks if a match is found in the list of which head is the first node.
 * 
 * @param head First node of list (may be dummy)
 * @param match match to look for
 * @return true match was found in the list
 * @return false match wasn't found in the list
 */
bool matchNodeInList(matchNode head, Match match);

/**
 * Removes all matches belonging to the provided tournament from the list.
 * 
 * @param list First node of the list
 * @param tournament_id Id of tournament to be removed
 * @param destory_match If true, destroys the removed matches
 */
void matchNodeRemoveTournamentFromList(matchNode list, int tournament_id, bool destory_match);

/**
 * Creates a Copy of the matchNode list
 * 
 * @param original first matchNode in a list
 * @return MapDataElement 
 */
MapDataElement matchNodeCopy(MapDataElement original_list);

/**
 * Destroy function for GDT map. Basically, a wrapper of 
 * mathNodeDestoryList(list, true).
 * 
 * @param element First matchNode in list to destroy
 */
void matchNodeDestroyMap(MapDataElement element);

#define FOREACH_MATCH(list, iter) \
  for(matchNode iter = list ;iter; iter = matchNodeNext(iter))

#endif // _MATCHNODE_H
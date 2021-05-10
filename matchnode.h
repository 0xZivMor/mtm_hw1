#ifndef _MATCHNODE_H
#define _MATCHNODE_H

#include "match.h"

typedef struct match_node_t *matchNode;

/**
 * Creates a new matchNode and adds it as the next node for the provided
 * previous node
 * 
 * @param match Match to be added to the list
 * @param next matchNode to point to as next. May be NULL
 * @return 
 *    NULL if match was NULL or memory failure occured
 *    new matchNode otherwise 
 */
matchNode newMatchNode(Match match, matchNode next);

/**
 * Gets the next node in the list
 * 
 * @param node current node
 * @return 
 *    next matchNode (may be NULL)
 */
matchNode nextMatchNode(matchNode node);

/**
 * Get the Match from matchNode object
 * 
 * @param node matchNode in question
 * @return
 *    Match contained in the node (may be NULL if node includes NULL or is NULL itself)
 */
Match getMatchFromMatchNode(matchNode node);

/**
 * Removes the provided match from the list. If match is not found in the list,
 * does nothing
 * 
 * @param list First matchNode in the list
 * @param match Match to be removed
 *      
 */
void matchNodeRemove(matchNode list, Match match);

/**
 * Destroys the matchNode and (if instructed) the contained Match
 * 
 * @param node matchNode to destroy
 * @param destory_match if true, destorys the Match in the node as well
 */
void matchNodeDestroy(matchNode node, bool destory_match);
#endif // _MATCHNODE_H
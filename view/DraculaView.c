////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// DraculaView.c: the DraculaView ADT implementation
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
///////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "DraculaView.h"
#include "Game.h"
#include "GameView.h"
#include "Map.h"
// add your own #includes here
#include "Queue.h"
// TODO: ADD YOUR OWN STRUCTS HERE

struct draculaView {
	GameView gv;
};

////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

DraculaView DvNew(char *pastPlays, Message messages[])
{
	DraculaView new = malloc(sizeof(*new));
	if (new == NULL) {
		fprintf(stderr, "Couldn't allocate DraculaView\n");
		exit(EXIT_FAILURE);
	}
	new->gv = GvNew(pastPlays, messages);
	return new;
}

void DvFree(DraculaView dv)
{
	
	//GvFree(dv->gv);
	free(dv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round DvGetRound(DraculaView dv)
{   
	return GvGetRound(dv->gv);
}

int DvGetScore(DraculaView dv)
{   
	return GvGetScore(dv->gv);
}

int DvGetHealth(DraculaView dv, Player player) {	
	
	return GvGetHealth(dv->gv, player);

}

PlaceId DvGetPlayerLocation(DraculaView dv, Player player) {
	
	return GvGetPlayerLocation(dv->gv, player);

}

PlaceId DvGetVampireLocation(DraculaView dv)
{
	return GvGetVampireLocation(dv->gv);
}

PlaceId *DvGetTrapLocations(DraculaView dv, int *numTraps)
{	
	PlaceId *traps = GvGetTrapLocations(dv->gv, numTraps);
	return traps;
}

////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId *DvGetValidMoves(DraculaView dv, int *numReturnedMoves)
{
	// If we are on the first move return an empty array (NULL is satisfactory)
	int round = DvGetRound(dv);
	if (round == 0) {
		return NULL;
	}
	
	int DB_moves = 0;
	int hide_moves = 0;
	int Trail_moves = *numReturnedMoves;
	
	// Get the current location of Dracula
	PlaceId Last_loc = DvGetPlayerLocation(dv, PLAYER_DRACULA);
	char *visited = calloc(NUM_REAL_PLACES, sizeof(*visited));
	// Get Draculas Trail
	bool canfree = false;
	PlaceId *Draculas_trail = GvGetLastMoves(dv->gv, PLAYER_DRACULA, TRAIL_SIZE, &Trail_moves, &canfree);
	// Create array to store valid moves/locations
	PlaceId* Valid_moves = calloc (Trail_moves, sizeof (*Valid_moves));
	// Get all the adjacent cities to current city
	int num_reachable = -1;
	PlaceId* Reachable_places = GvGetReachable(dv->gv, PLAYER_DRACULA, round, Last_loc, &num_reachable);	
	
	// Calculate (1) visited places (2) whether HI or DB is available
	/*for (int i = 0; i < Trail_moves; i++) {
		if (NOT_HI_DB_MOVE(Draculas_trail[i])) {
			visited[Draculas_trail[i]] = 1;
		}
		if (HIDE_MOVE(Draculas_trail[i])) {
			hide_moves++;
		}
		if (DB_MOVE(Draculas_trail[i])) {
			DB_moves++;
		}
	}
	*/
	int j = 0;
	// Copy cities which are (not in trail && adjacent) into Valid_moves array
	for(int i=0; i < num_reachable; i++) {
		if (visited[Reachable_places[i]] != 1 && Reachable_places[i != Last_loc]) {
			Valid_moves[j] = Reachable_places[i];
			j++;
		}
	}

	// DB
	// Count how many of the reacheable places are in the trail
	int reach_trail = 0;
	if (DB_moves == 0) {
		// Find which cities are both in trail and adjacent then copy the relating Dn to valid_moves
		for(int i=0; i < num_reachable; i++) {
			if (visited[Reachable_places[i]] == 1) {
				for (int k = 0; k < Trail_moves; k++) {
					if (Reachable_places[i] == Draculas_trail[k]) {
						if (k == 0) {
							Valid_moves[j] = DOUBLE_BACK_5;
							j++;
							reach_trail++;
						} else if (k == 1) {
							Valid_moves[j] = DOUBLE_BACK_4;
							j++;
							reach_trail++;
						} else if (k ==2) {
							Valid_moves[j] = DOUBLE_BACK_3;
							j++;
							reach_trail++;
						} else if (k == 3) {
							Valid_moves[j] = DOUBLE_BACK_2;
							j++;
							reach_trail++;
						} else if (k == 4) {
							Valid_moves[j] = DOUBLE_BACK_1;
							j++;
							reach_trail++;
						} 
					}
				}
			}
		}
	}

	// HI
	if (hide_moves == 0) {
		Valid_moves[j] = Last_loc;
		j++;
	}

	if (hide_moves ==1 && DB_moves == 1 && reach_trail == 6) {
		return NULL;
	}
	
	*numReturnedMoves = 0;
	return NULL;
}

PlaceId *DvWhereCanIGo(DraculaView dv, int *numReturnedLocs)
{
	
	*numReturnedLocs = 0;
	return NULL;
}

PlaceId *DvWhereCanIGoByType(DraculaView dv, bool road, bool boat,
                             int *numReturnedLocs)
{
	
	*numReturnedLocs = 0;
	return NULL;
}

PlaceId *DvWhereCanTheyGo(DraculaView dv, Player player,
                          int *numReturnedLocs)
{
	
	*numReturnedLocs = 0;
	return NULL;
}

PlaceId *DvWhereCanTheyGoByType(DraculaView dv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs)
{
	
	*numReturnedLocs = 0;
	return NULL;
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// TODO

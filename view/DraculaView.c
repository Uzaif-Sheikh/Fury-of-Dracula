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
	bool road = true;
	bool boat = true;
	bool rail = false;
	int numLocs = *numReturnedMoves;
	PlaceId* Dracula_Valid_moves = DvGetValidMovesbyType(dv, &numLocs, road, rail, boat);
	
	*numReturnedMoves = numLocs;
	return Dracula_Valid_moves;
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
	
	assert(player != PLAYER_DRACULA);
	
	int numLocs = -1;
	
	PlaceId from = GvGetPlayerLocation(dv->gv, player);
	int round = GvGetRound(dv->gv);
	
	PlaceId *locs = GvGetReachable(dv->gv, player,round, from, &numLocs);
	
	*numReturnedLocs = numLocs;
	return locs;
}

PlaceId *DvWhereCanTheyGoByType(DraculaView dv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs)
{
	
	int numLocs = -1;
	PlaceId from = GvGetPlayerLocation(dv->gv, player);
	int round = GvGetRound(dv->gv);
	PlaceId *locs = GvGetReachableByType(dv->gv, player,round, from, road, rail, boat, &numLocs);
	*numReturnedLocs = numLocs;
	return locs;
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// TODO
PlaceId* DvGetValidMovesbyType(DraculaView dv,int *numReturnedMoves, bool road, bool rail, bool boat) {

	PlaceId Last_loc = DvGetPlayerLocation(dv, PLAYER_DRACULA);
	
	//need to update this to if there is no turn of the player yet...
	if (Last_loc == NOWHERE){
		*numReturnedMoves = 0;
		return NULL;
	}
	
	int DB_moves = 0;
	int hide_moves = 0;
	int Trail_moves = *numReturnedMoves;
	bool canfree = false;
	int visited_places = 0;
	//int index_Last_Loc = -1;
	
	char *visited = calloc(NUM_REAL_PLACES, sizeof(*visited));
	//int *Double_back = calloc(10, sizeof(*Double_back));
	
	PlaceId *Draculas_trail = GvGetLastMoves(dv->gv, PLAYER_DRACULA, TRAIL_SIZE, &Trail_moves, &canfree);
	PlaceId *Valid_moves = calloc (NUM_REAL_PLACES+6, sizeof (*Valid_moves));

	
	
	for (int i = 0; i < Trail_moves; i++) {
		if (NOT_HI_DB_MOVE(Draculas_trail[i]) && Draculas_trail[i] != TELEPORT ) {
			visited[Draculas_trail[i]] = 1;
			visited_places++;
		}
		if (HIDE_MOVE(Draculas_trail[i])) {
			hide_moves++;
		}
		if (DB_MOVE(Draculas_trail[i])) {
			DB_moves++;
		}
		// if (Draculas_trail[i] == Last_loc){
		// 	index_Last_Loc = i;
		// }
	}
	
	int num_reachable = -1;
	int teleport_check = 0;
	//PlaceId double_back = DOUBLE_BACK_1;
	int round = DvGetRound(dv);
	PlaceId* Reachable_places = GvGetReachableByType(dv->gv, PLAYER_DRACULA, round, Last_loc, road, rail, boat, &num_reachable);	
	

	if (num_reachable == 1 && Reachable_places[0] == Last_loc) {
		*numReturnedMoves = 0;
		return NULL;
	}							
	//printf ("%d\n", Trail_moves);

		

	int j = 0;
	if (DB_moves == 0) {
		for (int i = 0; i < num_reachable; i++) {
			if (visited[Reachable_places[i]]) {
				for (int k = 0; k < Trail_moves; k++) {
						if (Reachable_places[i] == Draculas_trail[k]) {
							if (k == 0 && Trail_moves < 6) {
								int DB_value = Trail_moves - k;
								int Double_back_what = DB_value + HIDE;
								Valid_moves[j] = Double_back_what;
								j++;
							} 

							else if (k != 0) {
								int DB_value = Trail_moves - k;
								int Double_back_what = DB_value + HIDE;
								Valid_moves[j] = Double_back_what;
								j++;
							}
							
							// if (k == 1) {
							// 	int DB_value = Trail_moves - k;
							// 	int Double_back_what = DB_value + HIDE;
							// 	Valid_moves[j] = Double_back_what;
							// 	j++;
							// } 
							
							// else if (k == 2) {
							// 	int DB_value = Trail_moves - k;
							// 	int Double_back_what = DB_value + HIDE;
							// 	Valid_moves[j] = Double_back_what;
							// 	j++;
							// } 
							
							// else if (k == 3) {
							// 	printf ("Entered");
							// 	int DB_value = Trail_moves - k;
							// 	int Double_back_what = DB_value + HIDE;
							// 	Valid_moves[j] = Double_back_what;
							// 	j++;
								
							// } 
							// else if (k == 4) {
							// 	int DB_value = Trail_moves - k;
							// 	int Double_back_what = DB_value + HIDE;
							// 	Valid_moves[j] = Double_back_what;
							// 	j++;
								
							// }
							// else if (k == 5) {
							// 	int DB_value = Trail_moves - k;
							// 	int Double_back_what = DB_value + HIDE;
							// 	Valid_moves[j] = Double_back_what;
							// 	j++;
								
							// } 
						}
				}
				
			}
			
		}

	}

	
	if (DB_moves == 0) {
		if (Draculas_trail[Trail_moves-1] == HIDE) {
	 		Valid_moves[j] = DOUBLE_BACK_1;
			 j++;
		}
	}
	
	int num_real_reachable = num_reachable - 1;
	int t = j;
	// for (int i = 0; i < t; i++) {
	// 	printf  ("Check 1%s\n", placeIdToName(Valid_moves[i]));
	// }

	for (int i = 0; i < num_reachable; i++) {
		if (!visited[Reachable_places[i]] && Reachable_places[i] != Last_loc) {
			Valid_moves[t] = Reachable_places[i];
			t++;
		}
		if (visited[Reachable_places[i]]) {
			teleport_check++;
		}

	}
	
	
	int visited_adjacent = teleport_check-1;
	
	if (hide_moves == 0) {
		Valid_moves[t] = HIDE;
		t++;
	}
	
	if (hide_moves + DB_moves == 2 && visited_adjacent == num_real_reachable) {

	 	Valid_moves = NULL;
	 	t = 0;
	}

	for (int i = 0; i < t; i++) {
		printf ("%s\n", placeIdToName(Valid_moves[i]));
	}
	
	free (Draculas_trail);
	*numReturnedMoves = t;
	return Valid_moves;
}

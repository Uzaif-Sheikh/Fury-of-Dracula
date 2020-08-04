////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// dracula.c: your "Fury of Dracula" Dracula AI
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include "dracula.h"
#include "DraculaView.h"
#include "Game.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void decideDraculaMove(DraculaView dv)
{	
	char *move = calloc (3, sizeof(*move));
	int *hunters_places_movable = calloc (NUM_REAL_PLACES*4, sizeof(*hunters_places_movable));
	//PlaceId *Valid_moves = calloc()
	int numMoves = -1;
	//int numLocsDracula = -1;
	PlaceId* Valid_moves = DvGetValidMoves(dv, &numMoves);
	srand(time(0));
	int num = rand() % (numMoves);
	int moves = Valid_moves[num];
	move = placeIdToAbbrev(moves);
	// PlaceId* DraaculasLocation = DvWhereCanIGo (dv, &numLocsDracula);
	// printf ("Moves\n");
	// for (int i = 0; i < numMoves; i++) {
	// 	printf ("%s\n", placeIdToName(Valid_moves[i]));
	// }
	// printf ("Locations\n");
	// for (int i = 0; i < numLocsDracula; i++) {
	// 	printf ("%s\n", placeIdToName(DraaculasLocation[i]));
	// }
	// //move = placeIdToAbbrev(Valid_moves[0]);
	if (Valid_moves == NULL) {
	 	move = placeIdToAbbrev(TELEPORT);
	}
	// printf ("\n\n");
	// int numLocs = 0;
	// int numPlacesHunter = 0;
	// int k = 0;
	
	// for (int i = 0; i < NUM_PLAYERS-1; i++) {
		
	// 	PlaceId *WherecantheyGo = DvWhereCanTheyGo(dv, i, &numLocs);
	// 	printf ("Hunter %d reachable\n", i);
	// 	for (int t = 0; t < numLocs; t++) {
	// 		printf ("%s\n", placeIdToName(WherecantheyGo[t]));
	// 	}
	// 	printf ("\n\n");
		
	// 	int j = 0;
	// 	numPlacesHunter = k;
		
	// 	for (k = numPlacesHunter; k < numLocs+numPlacesHunter; k++) {
	// 		hunters_places_movable[k] = WherecantheyGo[j];
	// 		j++;
	// 	}
		
	// }
	// int total_reachable_places_H = k; 

	// printf ("\n\n");
	// printf ("%d\n", total_reachable_places_H);
	// for (int i = 0; i < total_reachable_places_H; i++) {
	// 	printf ("%s\n", placeIdToName(hunters_places_movable[i]));
	// }
	
	// for (int i = 0; i < total_reachable_places_H; i++) {
	// 	for (int j = 0; j < numMoves; j++) {
	// 		if (hunters_places_movable[i] != )
	// 	}
	// }
	
	registerBestPlay(move, "Mwahahahaha");

}

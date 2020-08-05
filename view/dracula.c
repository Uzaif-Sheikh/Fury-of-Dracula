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
#include "Queue.h"
#include "Map.h"

int* findDistancetoallCities (PlaceId curr_location);

void decideDraculaMove(DraculaView dv)
{	
	char *move = calloc (3, sizeof(*move));
	int *hunters_places_movable = calloc (NUM_REAL_PLACES*4, sizeof(*hunters_places_movable));
	
	
	int numMoves = -1;
	int numLocsDracula = -1;
	PlaceId Curr_Location = DvGetPlayerLocation(dv, PLAYER_DRACULA);
	int *distanceFromCurrLocation = findDistancetoallCities (Curr_Location);
	
	for (PlaceId i = 0; i < NUM_REAL_PLACES; i++) {
		printf ("Place %s 		Distance from Curr: %d\n", placeIdToName(i), distanceFromCurrLocation[i]);
	}
 	PlaceId* Valid_moves = DvGetValidMoves(dv, &numMoves);
	PlaceId *Best_moves = calloc(numMoves, sizeof(*Best_moves));
	
	srand(time(0));
	
	int num = rand() % (numMoves);
	int moves = Valid_moves[num];
	move = placeIdToAbbrev(moves);
	
	PlaceId* DraaculasLocation = DvWhereCanIGo (dv, &numLocsDracula);
	printf ("\n\n");

	printf ("Moves\n");
	for (int i = 0; i < numMoves; i++) {
		printf ("%s\n", placeIdToName(Valid_moves[i]));
	}
	printf ("\n\n");
	printf ("Locations\n");
	for (int i = 0; i < numLocsDracula; i++) {
		printf ("%s\n", placeIdToName(DraaculasLocation[i]));
	}
	
	move = placeIdToAbbrev(Valid_moves[0]);
	int curr_round = DvGetRound(dv);
	if (curr_round == 0) {
		move = placeIdToAbbrev(STRASBOURG);
	}
	if (Valid_moves == NULL && curr_round != 0) {
	 	move = placeIdToAbbrev(TELEPORT);
	}
	
	printf ("\n\n");
	int numLocs = 0;
	int numPlacesHunter = 0;
	int k = 0;
	
	for (int i = 0; i < NUM_PLAYERS-1; i++) {
		
		PlaceId *WherecantheyGo = DvWhereCanTheyGo(dv, i, &numLocs);
		printf ("Hunter %d reachable\n", i);
		for (int t = 0; t < numLocs; t++) {
			printf ("%s\n", placeIdToName(WherecantheyGo[t]));
		}
		printf ("\n\n");
		
		int j = 0;
		numPlacesHunter = k;
		
		for (k = numPlacesHunter; k < numLocs+numPlacesHunter; k++) {
			hunters_places_movable[k] = WherecantheyGo[j];
			j++;
		}
		
	}
	int total_reachable_places_H = k; 

	printf ("\n\n");
	printf ("%d\n", total_reachable_places_H);
	for (int i = 0; i < total_reachable_places_H; i++) {
		printf ("place: %s distance from DRA movable places: %d\n", placeIdToName(hunters_places_movable[i]), 
		(distanceFromCurrLocation[hunters_places_movable[i]]-1));
	}
	
	int num_best_moves = 0;
	int Location_reachable = 0;
	
	for (int j = 0; j < numLocsDracula; j++) {
		for (int i = 0; i < total_reachable_places_H; i++) {
			if (DraaculasLocation[j] == hunters_places_movable[i]) {
				Location_reachable = 1;
			}
		}
		if (Location_reachable == 0) {
			Best_moves[num_best_moves] = DraaculasLocation[j];
			num_best_moves++;
		}
		Location_reachable = 0;
	}

	if (num_best_moves == 0) {
		srand(time(0));
		int num = rand() % (numMoves);
		int moves = Valid_moves[num];
		move = placeIdToAbbrev(moves);
	}
	
	printf ("\n\n");
	printf ("Best_Moves :");

	for (int i = 0; i < num_best_moves; i++) {
		printf ("%s\n", placeIdToName(Best_moves[i]));
	}
	
	registerBestPlay(move, "Mwahahahaha");

}

int* findDistancetoallCities (PlaceId curr_location) {
	
	Map m = MapNew();
	int *distance = calloc(NUM_REAL_PLACES+1, sizeof(*distance));
	// for (int i = 0; i < NUM_REAL_PLACES+1; i++) {
	// 	distance[i] = 1;
	// }
	distance[curr_location] = 1;
	
	Queue q = newQueue();
	QueueJoin(q, curr_location);
	
	int distance_from_curr = 1;

	while (!QueueIsEmpty(q)) {
		
		int num_places_curr_level = QueueSize(q);
		
		while (num_places_curr_level> 0) {
			
			PlaceId distance_check_from_curr = QueueLeave(q);
			ConnList Places_next_depth = MapGetConnections(m, distance_check_from_curr);
			
			while (Places_next_depth) {
				
				if (Places_next_depth->type != RAIL) {
					if (distance[Places_next_depth->p] == 0) {
						distance[Places_next_depth->p] = distance_from_curr;
						QueueJoin(q, Places_next_depth->p);
					}
					else if (distance[Places_next_depth->p] != 0) {
						if (distance_from_curr <= distance[Places_next_depth->p]) {
							distance[Places_next_depth->p] = distance_from_curr;
						}
					}
				}

				Places_next_depth = Places_next_depth->next;
			}

			num_places_curr_level = num_places_curr_level - 1;
		}	
		
		
		distance_from_curr++;
		
	}
	distance[curr_location] = 0;
	dropQueue (q);
	return distance;
}
////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// hunter.c: your "Fury of Dracula" hunter AI.
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include "Game.h"
#include "hunter.h"
#include "HunterView.h"
#include <string.h>
#include <stdio.h>
#include "Places.h"
#include <time.h>
#include <stdlib.h>

// START is simply a #define used to initialise 
// integer variables in a stylistic way, or to
// stylistically revert a varibale back to zero
#define START		0

#define TRUE		1
#define	FALSE		0

#define NO_BEST_MOVES_AVAILABLE		0

#define HIDE_MOVE(id) (id == HIDE)
#define HUNTER(id) (id != PLAYER_DRACULA)
#define NOT_HI_DB_MOVE(id) (id != HIDE && id != DOUBLE_BACK_1 \
				&& id != DOUBLE_BACK_2 && id != DOUBLE_BACK_3 \
				&& id != DOUBLE_BACK_4 && id != DOUBLE_BACK_5)

#define DB_MOVE(id) (id == DOUBLE_BACK_1 || id == DOUBLE_BACK_2 \
				|| id == DOUBLE_BACK_3 || id == DOUBLE_BACK_4 \
				|| id == DOUBLE_BACK_5)
			
#define PASTPLAYS_NOT_FINISHED(c) (c != NULL)

void decideHunterMove(HunterView hv)
{
	int round = START;
	Player curr_player = HvGetPlayer(hv);
	PlaceId curr_loc_known = HvGetLastKnownDraculaLocation(hv,&round);
	int hv_round = HvGetRound(hv);
	char city[2];
	
	// If its is the first round, find a random city for the hunter
	// and store this in the city variable
	if(hv_round == START && curr_player != PLAYER_DRACULA) {
		int index = 70 - (curr_player * 2);
		strcpy(city,placeIdToAbbrev(index));
	}

	// if the hunter is dead, send them to the hospital
	else if(HvGetHealth(hv,curr_player) <= 0) {
		strcpy(city,placeIdToAbbrev(HOSPITAL_PLACE));
	}

	// If the current location of the hunter is NOWHERE
	// then we are in the first round. Randomly select the first
	// city to go to
	else if(curr_loc_known != NOWHERE) {

		int numPlace = START;
	    	PlaceId* where_dracula_can_go = HvWhereCanTheyGo(hv,PLAYER_DRACULA,&numPlace);
		
		srand(time(NULL));
		int id = (rand() % numPlace);
		int path_length = START;
		PlaceId * shortest_path = HvGetShortestPathTo(hv, curr_player, where_dracula_can_go[id], &path_length);
		
		if(path_length != 0) {
			strcpy(city,placeIdToAbbrev(shortest_path[0]));
		}

		else{
			int numPlace1 = START;
			PlaceId* where_can_i_go = HvWhereCanIGo(hv,&numPlace1);
			
			srand(time(NULL));
			int id = (rand() % numPlace1);
			strcpy(city,placeIdToAbbrev(where_can_i_go[id]));
		}
		
	}

	// If we are not in the first round, then select a random city again
	else {
		int numPlace1 = START;
		PlaceId* where_can_i_go = HvWhereCanIGo(hv,&numPlace1);
		
		srand(time(NULL));
		int id = (rand() % numPlace1);
		strcpy(city,placeIdToAbbrev(where_can_i_go[id]));
	}

	registerBestPlay(&city[0], "Have we nothing Toulouse?");
}
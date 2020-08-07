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

void decideHunterMove(HunterView hv)
{
	//int round = HvGetRound(hv);
	Player curr_player = HvGetPlayer(hv);
	PlaceId curr_loc_hunter = HvGetPlayerLocation(hv,curr_player);
	PlaceId vampire_loc = HvGetVampireLocation(hv);

	int numPlace = 0;
	PlaceId* where_dracula_can_go = HvWhereCanTheyGo(hv,PLAYER_DRACULA,&numPlace);
	char city[2];

	if(HvGetHealth(hv,curr_player) <= 4 && HvGetHealth(hv,curr_player) != 0){
		strcpy(city,placeIdToAbbrev(curr_loc_hunter));
	}
	else if(HvGetHealth(hv,curr_player) == 0){
		strcpy(city,placeIdToAbbrev(HOSPITAL_PLACE));
	}
	else if(where_dracula_can_go == NULL && vampire_loc == NOWHERE){
		int numPlace1 = 0;
		PlaceId* where_i_can_go = HvWhereCanIGo(hv,&numPlace1);
		if(numPlace1 > 0){
			strcpy(city,placeIdToAbbrev(where_i_can_go[0]));
		}

	}
	else if(where_dracula_can_go == NULL && vampire_loc != NOWHERE){

		int numPlace2 = 0;
		PlaceId* shortest_path = HvGetShortestPathTo(hv,curr_player,vampire_loc,&numPlace2);
		if(numPlace2 > 0){
			strcpy(city,placeIdToAbbrev(shortest_path[0]));
		}

	}
	else if(where_dracula_can_go != NULL){
		if(numPlace > 0){
			int numPlace2 = 0;
			PlaceId* shortest_path = HvGetShortestPathTo(hv,curr_player,where_dracula_can_go[0],&numPlace2);
			if(numPlace2 > 0){
				strcpy(city,placeIdToAbbrev(shortest_path[0]));
			}
		}
	}

	registerBestPlay(city, "Have we nothing Toulouse?");
}

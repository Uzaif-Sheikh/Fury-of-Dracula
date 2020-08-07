
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

void decideHunterMove(HunterView hv)
{
	//int round = 0;
	
	Player curr_player = HvGetPlayer(hv);
	//PlaceId curr_loc_known = HvGetLastKnownDraculaLocation(hv,&round);
	int hv_round = HvGetRound(hv);

	// int numPlace = 0;
	// PlaceId* where_dracula_can_go = HvWhereCanTheyGo(hv,PLAYER_DRACULA,&numPlace);
	char city[2];

	if(hv_round == 0 && curr_player != PLAYER_DRACULA){
		int index = 70 - (curr_player * 2);
		strcpy(city,placeIdToAbbrev(index));
	}
	else{
		int numPlace1 = 0;
		PlaceId* where_can_i_go = HvWhereCanIGo(hv,&numPlace1);
		printf("%d numPlace \n",numPlace1);
		for(int i = 0;i < numPlace1;i++){
			printf("curr loc known = nowhere %s --\n",placeIdToAbbrev(where_can_i_go[i]));
		}
		srand(time(NULL));
		int id = (rand() % numPlace1);
		printf("%d --\n",id); 
		strcpy(city,placeIdToAbbrev(where_can_i_go[id]));
	}
	// else if(curr_loc_known == NOWHERE){
	// 	int numPlace1 = 0;
	// 	PlaceId* where_can_i_go = HvWhereCanIGo(hv,&numPlace1);
	// 	printf("%d numPlace \n",numPlace1);
	// 	for(int i = 0;i < numPlace1;i++){
	// 		printf("curr loc known = nowhere %s --\n",placeIdToAbbrev(where_can_i_go[i]));
	// 	}
	// 	int id = (rand() % numPlace1);
	// 	printf("%d --\n",id); 
	// 	strcpy(city,placeIdToAbbrev(where_can_i_go[id]));
		
	// }
	// else if(curr_loc_known != NOWHERE){
	// 	//if(HvGetPlayerLocation(hv,curr_player) == curr_loc_known){
	// 		int numPlace1 = 0;
	// 		PlaceId* where_can_i_go = HvWhereCanIGo(hv,&numPlace1);
	// 		printf("%d numPlace \n",numPlace1);
	// 		for(int i = 0;i < numPlace1;i++){
	// 			printf("curr loc known != nowhere %s --\n",placeIdToAbbrev(where_can_i_go[i]));
	// 		}
	// 		int id = (rand() % numPlace1);
	// 		printf("%d --\n",id); 
	// 		strcpy(city,placeIdToAbbrev(where_can_i_go[id]));
	// 	//}
	// 	/*else{
	// 		int numPath = 0;
	// 		PlaceId* shortest_path = HvGetShortestPathTo(hv,curr_player,curr_loc_known,&numPath);
	// 		printf("%d numPlace \n",numPath);
	// 		for(int i = 0;i < numPath;i++){
	// 			printf("shortesh path %s --\n",placeIdToAbbrev(shortest_path[i]));
	// 		}
	// 		strcpy(city,placeIdToAbbrev(shortest_path[0]));
	// 	}*/
	// }
	// else{
	// 	int numPlace1 = 0;
	// 	PlaceId* where_can_i_go = HvWhereCanIGo(hv,&numPlace1);
	// 	printf("%d numPlace \n",numPlace1);
	// 	for(int i = 0;i < numPlace1;i++){
	// 		printf("where the fuck i can go %s --\n",placeIdToAbbrev(where_can_i_go[i]));
	// 	}
	// 	int id = (rand() % numPlace1);
	// 	printf("%d --\n",id); 
	// 	strcpy(city,placeIdToAbbrev(where_can_i_go[id]));
		
	// }

	registerBestPlay(&city[0], "Have we nothing Toulouse?");
}


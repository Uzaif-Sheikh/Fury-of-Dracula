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

// static void random_moves(HunterView hv,char city[2]);
// static bool check_moves(PlaceId* array,PlaceId player_loc,int num);

void decideHunterMove(HunterView hv)
{
	int round = 0;
	Player curr_player = HvGetPlayer(hv);
	PlaceId curr_loc_known = HvGetLastKnownDraculaLocation(hv,&round);
	int hv_round = HvGetRound(hv);
	char city[2];
	printf("%d\n", HvGetPlayer(hv));
	printf("%d\n", HvGetHealth(hv,curr_player));
	if(hv_round == 0 && curr_player != PLAYER_DRACULA){
		int index = 70 - (curr_player * 2);
		strcpy(city,placeIdToAbbrev(index));
	}
	else if(HvGetHealth(hv,curr_player) <= 0){
		printf("Sending To Hospital: %d\n", HvGetHealth(hv,curr_player));
		strcpy(city,placeIdToAbbrev(HOSPITAL_PLACE));
	}
	else if(curr_loc_known != NOWHERE) {

		int numPlace = 0;
	    PlaceId* where_dracula_can_go = HvWhereCanTheyGo(hv,PLAYER_DRACULA,&numPlace);
		// for(int i = 0;i < numPlace;i++){
		// 	printf("%s dracula loc \n",placeIdToAbbrev(where_dracula_can_go[i]));
		// }
		srand(time(NULL));
		int id = (rand() % numPlace);
		int path_length = 0;
		PlaceId * shortest_path = HvGetShortestPathTo(hv, curr_player, where_dracula_can_go[id], &path_length);
		// for(int i = 0;i < path_length;i++){
		// 	printf("%s shortest path \n",placeIdToAbbrev(shortest_path[i]));
		// }
		if(path_length != 0){
			strcpy(city,placeIdToAbbrev(shortest_path[0]));
		}
		else{
			int numPlace1 = 0;
			PlaceId* where_can_i_go = HvWhereCanIGo(hv,&numPlace1);
			// for(int i = 0;i < numPlace1;i++){
			// 	printf("curr loc known = nowhere %s --\n",placeIdToAbbrev(where_can_i_go[i]));
			// }
			srand(time(NULL));
			int id = (rand() % numPlace1);
			strcpy(city,placeIdToAbbrev(where_can_i_go[id]));
		}
		
		

		/*if(check_moves(where_dracula_can_go,HvGetPlayerLocation(hv,curr_player),numPlace)){
			random_moves(hv,city);
		}

		if(HvGetPlayerLocation(hv,curr_player) == curr_loc_known){
			int numPlace1 = 0;
			PlaceId* where_can_i_go = HvWhereCanIGo(hv,&numPlace1);
			printf("%d numPlace \n",numPlace1);
			for(int i = 0;i < numPlace1;i++){
				printf("curr loc known != nowhere %s --\n",placeIdToAbbrev(where_can_i_go[i]));
			}
			int id = (rand() % numPlace1);
			printf("%d --\n",id); 
			strcpy(city,placeIdToAbbrev(where_can_i_go[id]));
		}*/
		
	}
	else{
		int numPlace1 = 0;
		PlaceId* where_can_i_go = HvWhereCanIGo(hv,&numPlace1);
		//printf("%d numPlace \n",numPlace1);
		// for(int i = 0;i < numPlace1;i++){
		// 	printf("curr loc known = nowhere %s --\n",placeIdToAbbrev(where_can_i_go[i]));
		// }
		srand(time(NULL));
		int id = (rand() % numPlace1);
		//printf("%d --\n",id); 
		strcpy(city,placeIdToAbbrev(where_can_i_go[id]));
	}

	registerBestPlay(&city[0], "Have we nothing Toulouse?");
}

// static void random_moves(HunterView hv,char city[2]){
// 	int numPlace1 = 0;
// 	PlaceId* where_can_i_go = HvWhereCanIGo(hv,&numPlace1);
// 	printf("%d numPlace \n",numPlace1);
// 	for(int i = 0;i < numPlace1;i++){
// 		printf("curr loc known = nowhere %s --\n",placeIdToAbbrev(where_can_i_go[i]));
// 	}
// 	srand(time(NULL));
// 	int id = (rand() % numPlace1);
// 	//printf("%d --\n",id); 
// 	strcpy(city,placeIdToAbbrev(where_can_i_go[id]));
// }

// static bool check_moves(PlaceId* array,PlaceId player_loc,int num){
	
// 	for(int i = 0;i < num;i++){
// 		if(array[i] == player_loc) return true;
// 	}

// 	return false;
// }
///////////////////////////////////////////////////////////////////////
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
#include "testUtils.h"

#define START 0
#define HIDE_MOVE(id) (id == HIDE)
#define HUNTER(id) (id != PLAYER_DRACULA)
#define NOT_HI_DB_MOVE(id) (id != HIDE && id != DOUBLE_BACK_1 \
				&& id != DOUBLE_BACK_2 && id != DOUBLE_BACK_3 \
				&& id != DOUBLE_BACK_4 && id != DOUBLE_BACK_5)

#define DB_MOVE(id) (id == DOUBLE_BACK_1 || id == DOUBLE_BACK_2 \
			|| id == DOUBLE_BACK_3 || id == DOUBLE_BACK_4 \
			|| id == DOUBLE_BACK_5)
			
#define PASTPLAYS_NOT_FINISHED(c) (c != NULL)

int* findDistancetoallCities (PlaceId curr_location);
//static PlaceId* Reachable(DraculaView dv, Player hunter,int round,PlaceId p,int *numReturnedLocs);
int ShortestPathforClosestHunter(DraculaView dv,Player hunter, PlaceId from, PlaceId dest);

void decideDraculaMove(DraculaView dv)
{
    char *move = calloc (3, sizeof(*move));
	int *hunters_places_movable = calloc (NUM_REAL_PLACES*4, sizeof(*hunters_places_movable));
	
	int numMoves = -1;
	int numLocsDracula = -1;
	
 	PlaceId* Valid_moves = DvGetValidMoves(dv, &numMoves);
	PlaceId* DraaculasLocation = DvWhereCanIGo (dv, &numLocsDracula);
	

	PlaceId Curr_Location = DvGetPlayerLocation(dv, PLAYER_DRACULA);
	int curr_round = DvGetRound(dv);

	if (Curr_Location == NOWHERE) {

		int numLocs = 0;
        int numPlacesHunter = 0;
        int k = 0;
        int *Reachable_Places = calloc (NUM_REAL_PLACES+1, sizeof(*Reachable_Places));
        
		for (int i = 0; i < NUM_PLAYERS-1; i++) {
            
            PlaceId *WherecantheyGo = DvWhereCanTheyGo(dv, i, &numLocs);
            int j = 0;
            numPlacesHunter = k;
            
            for (k = numPlacesHunter; k < numLocs+numPlacesHunter; k++) {
                hunters_places_movable[k] = WherecantheyGo[j];
				if (Reachable_Places[WherecantheyGo[j]] != 1) {
					Reachable_Places[hunters_places_movable[k]] = 1;
				}
				
                j++;
            }
            
        }
        
        //int total_reachable_places_H = k; 
		
		// printf ("Reachable Places\n");
		// for (int i = 0; i < total_reachable_places_H; i++) {
		// 	printf (" %d PLace : %s\n", i, placeIdToName(hunters_places_movable[i]));
		// }
		// printf ("Reachable Places\n");
		// for (int i = 0; i < NUM_REAL_PLACES; i++) {
		// 	printf ("%d %d\n",i, Reachable_Places[i]);
		// }
		//int unreachable_places = NUM_REAL_PLACES - total_reachable_places_H;
		
		int *Movable_places = calloc (NUM_REAL_PLACES, sizeof(*Movable_places));
		
		int t = 0;
		for (int i = 0; i < NUM_REAL_PLACES; i++) {
			if (Reachable_Places[i] != 1 && placeIdToType(i) != SEA && i != ST_JOSEPH_AND_ST_MARY && i != CASTLE_DRACULA) {
				Movable_places[t] = i;
				t++;
			}
		}

		// printf ("\n\n");
		// printf ("Movable places\n");
		
		int unreachable_places_excluding_sea = t;
		// for (int i = 0; i < unreachable_places_excluding_sea; i ++) {
		// 	printf ("%d Place : %s\n", i, placeIdToName(Movable_places[i]));
		// }
		
		srand(time(0));
        int num = rand() % (unreachable_places_excluding_sea);
		int moves = Movable_places[num];
		//printf ("%d\n", moves);
    	move = placeIdToAbbrev(moves);
		//printf ("\n%s", move);
		//free (Reachable_Places);
        
	}

	else if (Valid_moves == NULL && curr_round != 0) {
	 	move = placeIdToAbbrev(TELEPORT);
	}
	
	else if (Valid_moves != NULL && curr_round != 0) {
	    
		int *distanceFromCurrLocation = findDistancetoallCities (Curr_Location);
        PlaceId *Best_moves = calloc(numMoves, sizeof(*Best_moves));
        int numLocs = 0;
        int numPlacesHunter = 0;
        int k = 0;
        
        for (int i = 0; i < NUM_PLAYERS-1; i++) {
            
            PlaceId *WherecantheyGo = DvWhereCanTheyGo(dv, i, &numLocs);
            int j = 0;
            numPlacesHunter = k;
            
            for (k = numPlacesHunter; k < numLocs+numPlacesHunter; k++) {
                hunters_places_movable[k] = WherecantheyGo[j];
                j++;
            }
            
        }
        
        int total_reachable_places_H = k; 

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

        else if (num_best_moves > 0) {
    
            PlaceId* HI_Dn_moves = calloc (6, sizeof(*HI_Dn_moves));
            PlaceId *HI_Dn_Locs = calloc (6, sizeof(*HI_Dn_Locs));
            int num_HI_Dn_moves = 0;
            
            for (int i = 0; i < numMoves; i++) {
                if (HIDE_MOVE(Valid_moves[i]) || DB_MOVE(Valid_moves[i])) {
                    HI_Dn_moves[num_HI_Dn_moves] = Valid_moves[i];
                    num_HI_Dn_moves++;
                }
            }
            
            if (num_HI_Dn_moves > 0) {
                for (int i = 0; i < num_HI_Dn_moves; i++) {
                    HI_Dn_Locs[i] = DraaculasLocation[i];
                }
            }
            
			int depth_Loc = 0;
            int min_depth = 0;
            PlaceId Closest_hunter_Loc = NOWHERE;
            Player closest_hunter = PLAYER_LORD_GODALMING;
            
            for (int k = 0; k < NUM_PLAYERS-1; k++) {
                min_depth = depth_Loc;
                PlaceId Location = DvGetPlayerLocation(dv, k);
                depth_Loc = distanceFromCurrLocation[Location] - 1;
                if (min_depth <= depth_Loc) {
                    min_depth = depth_Loc;
                    Closest_hunter_Loc = Location;
                    closest_hunter = k;
                }
            }
            
            int path_length = 0;
            int path_received = 0;
            
            for (int i = 0; i < num_best_moves; i++) {
                path_length = path_received;
                path_received = ShortestPathforClosestHunter (dv, closest_hunter, Closest_hunter_Loc, Best_moves[i]);
                if (path_received >= path_length) {
                    move = placeIdToAbbrev(Best_moves[i]);
                }
            }

			for (int i = num_HI_Dn_moves-1; i >= 0; i--) {
                if (placeAbbrevToId(move) == HI_Dn_Locs[i]) {
                    move = placeIdToAbbrev(HI_Dn_moves[i]);
                }
            }
            
        }

    }
	
	registerBestPlay(move, "Mwahahahaha");
}

int* findDistancetoallCities (PlaceId curr_location) {
	
	Map m = MapNew();
	int *distance = calloc(NUM_REAL_PLACES+1, sizeof(*distance));

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
	free (m);
	return distance;
}

int ShortestPathforClosestHunter(DraculaView dv,Player hunter, PlaceId from, PlaceId dest)
{
	Map m = MapNew();

	// round keeps track of round since the hunter last moved
	int round = START;

	int num_places = MapNumPlaces(m);
	
	// Allocating space for parent array
	PlaceId* parent = malloc(num_places*sizeof(PlaceId));

	for(PlaceId i = 0;i < num_places;i++){
		parent[i] = NOWHERE;
	}

	// Getting the new queue
	Queue q = newQueue();
	QueueJoin(q,from);
	parent[from] = from;

	// flag keeps track of whether the destination has been found or not
	int flag = START;

	// while Queue is empty and destination not found
	while(!QueueIsEmpty(q) && flag != 1){
		PlaceId p ;
		int size_q = QueueSize(q);
		while(size_q > 0 && flag != 1) {
			p = QueueLeave(q);
			size_q--;
			int numlocs = 0;

			// Reachable helper function called
			PlaceId *reach = DvReachablePlaces(dv, hunter, round, p, &numlocs);
			
			for (int j = 0; j < numlocs ; j++) {
				if (parent[reach[j]] == NOWHERE && reach[j] != p) {
					if (reach[j] == dest) {
						flag++;
						parent[dest] = p;
						break;
					}
					QueueJoin(q, reach[j]);
					parent[reach[j]] = p;
				}
			}
			free(reach);
		}
		round++;
	}
	PlaceId temp[50];
	int index = START;
	for(PlaceId i = dest;i != from;i = parent[i]){
		temp[index++] = i;
	}

	PlaceId* shortest_path = calloc(num_places,sizeof(PlaceId));
	int count = START;
	for(PlaceId j = index-1;j >= 0;j--){
		shortest_path[count++] = temp[j];
	}
	
	free(parent);
	MapFree(m);
	return count;
}
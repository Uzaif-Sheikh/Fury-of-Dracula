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

////////////////////////////////////////////
// Reference: Queue.h is adapted from one //
// of the labs done earlier in the term   //
////////////////////////////////////////////

// START is simply a #define used to initialise 
// integer variables in a stylistic way, or to
// stylistically revert a varibale back to zero
#define START		0

int* findDistancetoallCities (PlaceId curr_location);
int ShortestPathforClosestHunter(DraculaView dv,Player hunter, PlaceId from, PlaceId dest);

// Decide Dracula's next move, while keeping in consideration
// what actions the hunters take
void decideDraculaMove(DraculaView dv)
{
    char *move = calloc (3, sizeof(*move));
	int *hunters_places_movable = calloc (NUM_REAL_PLACES * 4, sizeof(*hunters_places_movable));
	
	int numMoves = START - 1;
	int numLocsDracula = START - 1;
	
 	PlaceId* Valid_moves = DvGetValidMoves(dv, &numMoves);
	PlaceId* DraaculasLocation = DvWhereCanIGo (dv, &numLocsDracula);
	

	PlaceId Curr_Location = DvGetPlayerLocation(dv, PLAYER_DRACULA);
	int curr_round = DvGetRound(dv);

	// If the Dracula is on his first turn (current loaction is NOWHERE)
	// then analyse where the hunters decided to go, and go to a place
	// which is far away and not easily reachable by the hunters
	if (Curr_Location == NOWHERE) {

		// Integer k is used to copy the value of numPlacesHunter and then let k
		// interate
		int numLocs = START;
        int numPlacesHunter = START;
        int k = START;
        int *Reachable_Places = calloc (NUM_REAL_PLACES+1, sizeof(*Reachable_Places));
        
		for (int i = START; i < NUM_PLAYERS - 1; i++) {
            
            PlaceId *WherecantheyGo = DvWhereCanTheyGo(dv, i, &numLocs);
            numPlacesHunter = k;
            
			// Reaffirming, the integer k is used in this for loop to assist in 
			// finding where the hunters can go for their next move. This will
			// be used to try and make Dracula avoid going to these places

			// The integer j is used to iterate through the Wherecantheygo array
            int j = START;
            for (k = numPlacesHunter; k < numLocs + numPlacesHunter; k++) {
				hunters_places_movable[k] = WherecantheyGo[j];
				
				if (Reachable_Places[WherecantheyGo[j]] != 1) {
					Reachable_Places[hunters_places_movable[k]] = 1;
				}

                j++;
            }
            
        }
        
		int *Movable_places = calloc (NUM_REAL_PLACES, sizeof(*Movable_places));
		
		// Find the moveable places for Dracula which the hunters will find it 
		// hard to reach
		// The integer t is used to iterate through the Moveable_places array
		int t = START;
		for (int i = START; i < NUM_REAL_PLACES; i++) {
			if (Reachable_Places[i] != 1 && placeIdToType(i) != SEA && i != ST_JOSEPH_AND_ST_MARY && i != CASTLE_DRACULA) {
				Movable_places[t] = i;
				t++;
			}
		}
		
		int unreachable_places_excluding_sea = t;
		
		srand(time(0));
        int num = rand() % (unreachable_places_excluding_sea);
		int moves = Movable_places[num];
		move = placeIdToAbbrev(moves);
		
    }

	// If we are not on the first round and there are no valid moves, 
	// teleport to Castle Dracula
	else if (Valid_moves == NULL && curr_round != 0) {
	 	move = placeIdToAbbrev(TELEPORT);
	}
	
	else if (Valid_moves != NULL && curr_round != 0) {
	    
		// k is an integer used to interate through the hunters_places_moveable array
		int *distanceFromCurrLocation = findDistancetoallCities (Curr_Location);
        PlaceId *Best_moves = calloc(numMoves, sizeof(*Best_moves));
        int numLocs = START;
        int numPlacesHunter = START;
        int k = START;
        
        for (int i = 0; i < NUM_PLAYERS-1; i++) {
            
            PlaceId *WherecantheyGo = DvWhereCanTheyGo(dv, i, &numLocs);
            int j = START;
            numPlacesHunter = k;
            
			// Find out where the hunters may go in their next move
            for (k = numPlacesHunter; k < numLocs+numPlacesHunter; k++) {
                hunters_places_movable[k] = WherecantheyGo[j];
                j++;
            }
            
        }
        
        int total_reachable_places_H = k; 
        int num_best_moves = START;
        int Location_reachable = START;
        
		// Find whether Draculas loaction is within the moveable places
		// for hunters, then decide on what the best move would be
        for (int j = START; j < numLocsDracula; j++) {
            for (int i = START; i < total_reachable_places_H; i++) {
                if (DraaculasLocation[j] == hunters_places_movable[i]) {
                    Location_reachable = TRUE;
                }
            }

			// If any of the reachable places by dracula are hard to reach
			// for hunters, then add this to the Best_moves array
            if (Location_reachable == FALSE) {
                Best_moves[num_best_moves] = DraaculasLocation[j];
                num_best_moves++;
            }

            Location_reachable = START;
        }

		// A best move is to a place which a hunter will not go;
		// if all of the reachable places for dracula are also easily 
		// reachable by the hunters, then randomise the place selection
        if (num_best_moves == NO_BEST_MOVES_AVAILABLE) {
            srand(time(0));
            int num = rand() % (numMoves);
            int moves = Valid_moves[num];
            move = placeIdToAbbrev(moves);
	    }

        else if (num_best_moves > 0) {
    
            PlaceId* HI_Dn_moves = calloc (6, sizeof(*HI_Dn_moves));
            PlaceId *HI_Dn_Locs = calloc (6, sizeof(*HI_Dn_Locs));
            int num_HI_Dn_moves = START;
            
            // Calculate the number of HIDE moves and/or DOUBLE_BACK
			// moves that are available for Dracula to play
			for (int i = START; i < numMoves; i++) {
                if (HIDE_MOVE(Valid_moves[i]) || DB_MOVE(Valid_moves[i])) {
                    HI_Dn_moves[num_HI_Dn_moves] = Valid_moves[i];
                    num_HI_Dn_moves++;
                }
            }
            
			// Get the location of the HIDE or DOUBLE_BACK
			// moves avalable
            if (num_HI_Dn_moves > 0) {
                for (int i = 0; i < num_HI_Dn_moves; i++) {
                    HI_Dn_Locs[i] = DraaculasLocation[i];
                }
            }
            
			int depth_Loc = START;
            int min_depth = START;
            PlaceId Closest_hunter_Loc = NOWHERE;
            Player closest_hunter = PLAYER_LORD_GODALMING;
            
            // Find the depth of a Hunter to a given place
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
            
            int path_length = START;
            int path_received = START;
            
			// Find the shortest path to a location of the hunter which is
			// closest to that particular location
			// Then pick a place for Dracula to go to which has a greater depth
			// than this
            for (int i = 0; i < num_best_moves; i++) {
                path_length = path_received;
                path_received = ShortestPathforClosestHunter (dv, closest_hunter, Closest_hunter_Loc, Best_moves[i]);
                if (path_received >= path_length) {
                    move = placeIdToAbbrev(Best_moves[i]);
                }
            }

			// If the above condition is not possible, and a HIDE or 
			// DOUBLE_BACK move is available, commit to that move
			for (int i = num_HI_Dn_moves-1; i >= 0; i--) {
                if (placeAbbrevToId(move) == HI_Dn_Locs[i]) {
                    move = placeIdToAbbrev(HI_Dn_moves[i]);
                }
            }
            
        }

    }
	
	registerBestPlay(move, "Mwahahahaha");
}

// Find the distance from a current player loaction to all the cities
// On the map using a queue
int* findDistancetoallCities (PlaceId curr_location) 
{
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

// Find the closest hunter to a strategically picked location, and find out
// their shortest path to this place. This will be used to decide whether or
// not it is feasible to send Dracula to that location
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
			int numlocs = START;

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
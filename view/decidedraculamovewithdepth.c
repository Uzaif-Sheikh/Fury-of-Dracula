void decideDraculaMove(DraculaView dv)
{
    char *move = calloc (3, sizeof(*move));
	int *hunters_places_movable = calloc (NUM_REAL_PLACES*4, sizeof(*hunters_places_movable));
	
	
	int numMoves = -1;
	int numLocsDracula = -1;
	
 	PlaceId* Valid_moves = DvGetValidMoves(dv, &numMoves);
	PlaceId* DraaculasLocation = DvWhereCanIGo (dv, &numLocsDracula);
   
    int curr_round = DvGetRound(dv);
	
	if (curr_round == 0) {
		move = placeIdToAbbrev(STRASBOURG);
	}

	else if (Valid_moves == NULL && curr_round != 0) {
	 	move = placeIdToAbbrev(TELEPORT);
	}
	
	else if (Valid_moves != NULL && curr_round != 0) {
	    
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
                if (path_received > path_length) {
                    move = placeIdToAbbrev(Best_moves[i]);
                }
            }
            
            for (int i = 0; i < num_HI_Dn_moves; i++) {
                if (placeAbbrevToId(move) == HI_Dn_Locs[i]) {
                    move = placeIdToAbbrev(HI_Dn_moves[i]);
                }
            }
        }

    }
	
	registerBestPlay(move, "Mwahahahaha");
}
PlaceId *GvGetReachableByType(GameView gv, Player player, Round round,
                              PlaceId from, bool road, bool rail,
                              bool boat, int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	int numLocs = 0;
	Map Places = MapNew();
	PlaceId *ReachablebyType = calloc(NUM_REAL_PLACES, sizeof(*ReachablebyType));
	
	ReachablebyType[0] = from;
	int Location_by_type = 1;

	if (boat == true) {
		for (ConnList c = MapGetConnections(Places, from); c != NULL; c = c->next) {
			if (c->type == BOAT) {
				ReachablebyType[Location_by_type] = c->p;
				Location_by_type++;
			}
		}
	}
	
	if (road == true) {
		for (ConnList c = MapGetConnections(Places, from); c != NULL; c = c->next) {
			if (c->type == ROAD)
				ReachablebyType[Location_by_type] = c->p;
				Location_by_type++;
			}
		}
	}
	
	if (rail == true) {
		for (ConnList c = MapGetConnections(Places, from); c != NULL; c = c->next) {
			if (c->type == RAIL) {
				ReachablebyType[Location_by_type] = c->p;
				Location_by_type++;
			}
		}
	}
    
	
	
	
	*numReturnedLocs = Location_by_type;
	printf("\n\n");
	for (int i = 0; i < Location_by_type; i++) {
		printf ("%s\n", placeIdToName(ReachablebyType[i]));
	}
	return ReachablebyType;
}
assert(placeIsReal(from));
	Map Places = MapNew();
	
	PlaceId *GetReachable = calloc(NUM_REAL_PLACES, sizeof(*GetReachable));
    	int movable_places = 0;

	if (player != PLAYER_DRACULA) {
		
		ConnList Reachable_Places = MapGetConnections(Places, from);
		ConnList Places_Movable = Reachable_Places;
		
		while (Places_Movable) {
			GetReachable[movable_places] = Places_Movable->p;
			Places_Movable = Places_Movable->next;
			movable_places++;
		}
		
	}
	
	*numReturnedLocs = movable_places;
	return GetReachable;
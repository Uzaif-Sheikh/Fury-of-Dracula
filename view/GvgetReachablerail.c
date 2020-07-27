int RailRoutesFind (int max_rail_size, PlaceId *GetReachable, int movable_places, 
TransportType* TransportConnections, Map places) {

	int num_rail_type = 0;
	Queue q = newQueue();
	
	for (int i = 0; i < movable_places; i++) {
		if (TransportConnections[i] == RAIL) {
			QueueJoin(q, GetReachable[i]);
		}
		
	}
	
	int num_rail_moves = 1;
	char visited[NUM_REAL_PLACES] = {0}; 
	visited[GetReachable[0]] = 1;
	
	while (!QueueIsEmpty(q) && num_rail_moves <= max_rail_size) {

		int num_curr_level_rail = QueueSize(q);
		printf ("%d\n", num_curr_level_rail);
		
		while (num_curr_level_rail--) {
			
			PlaceId Rail_connecting = QueueLeave(q);
			if (visited[Rail_connecting]) continue;
			
			visited[Rail_connecting] = 1;
			printf ("%s\n", placeIdToName(Rail_connecting));
			
			ConnList Rail_route = MapGetConnections(places, Rail_connecting);
			
			ConnList connecting_rail = Rail_route;

			while (connecting_rail) {
				if (connecting_rail->type == RAIL && !visited[connecting_rail->p]) {
					num_rail_type++;
					GetReachable[movable_places+num_rail_type] = connecting_rail->p;
					TransportConnections[movable_places + num_rail_type] = RAIL;
					QueueJoin(q, connecting_rail->p);
				}
				connecting_rail = connecting_rail->next;
			}
		}

		num_rail_moves++;
	}
	
	return num_rail_type;
}
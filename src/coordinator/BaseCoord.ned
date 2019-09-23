package src.coordinator;


simple BaseCoord like ICoord
{
    parameters:
        double minTripLength @unit(m) = default(1000m);
        int requestAssignmentStrategy = default(0);
        
        @display("i=block/cogwheel_s");
        
        
        // ------ SIGNALS -------- //
        @signal[traveledDistance](type="double");       	//Traveled distance per vehicle
        //@statistic[traveledDistance](title="Traveled distance per vehicle (km)";source="traveledDistance";unit=km;record=vector,stats;interpolationmode=none);

        @signal[waitingTime](type="double");				//Pick-up waiting time per request
        //@statistic[waitingTime](title="Waiting time (minute)";source="waitingTime";unit=minute;record=vector,stats;interpolationmode=none);

        @signal[actualTripTime](type="double");				//Actual time scored on vehicle (per request)
        //@statistic[actualTripTime](title="Actual Trip Time (minute)";source="actualTripTime";unit=minute;record=vector,stats;interpolationmode=none);

        @signal[stretch](type="double");
        //@statistic[stretch](title="Stretch";source="stretch";record=vector,stats;interpolationmode=none);

        @signal[tripDistance](type="double");				//Distance per trip
        //@statistic[tripDistance](title="Trip Distance (km)";source="tripDistance";unit=km;record=vector,stats;interpolationmode=none);

        @signal[passengersOnBoard](type="double");			//Passengers on board per vehicle at snapshot time
        //@statistic[passengersOnBoard](title="Passengers On Board";source="passengersOnBoard";record=vector,stats;interpolationmode=none);

        @signal[toDropoffRequests](type="double");       	//Picked-up requests not yet dropped-off (at snapshot time)
        //@statistic[toDropoffRequests](title="Picked-up Requests";source="toDropoffRequests";record=vector,stats;interpolationmode=none);

        @signal[toPickupRequests](type="double");       	//Requests assigned but not yet picked-up
        //@statistic[toPickupRequests](title="Requests To Pickup";source="toPickupRequests";record=vector,stats;interpolationmode=none);

        @signal[requestsAssignedPerVehicle](type="double");  //Number of requests assigned per vehicle
        //@statistic[requestsAssignedPerVehicle](title="Requests assigned per vehicle";source="requestsAssignedPerVehicle";record=vector,stats;interpolationmode=none);
        
        @signal[travel*-time](type="double"); 
		//@statisticTemplate[travelTime](title="Travel Time Per State (minute)";unit=minute;record=vector,stats;interpolationmode=none);
		
		@signal[totalRequestsPerTime](type="double");
		//@statistic[totalRequestsPerTime](title="Total Requests Per Time";source="totalRequestsPerTime";record=vector;interpolationmode=none);
		
		@signal[assignedRequestsPerTime](type="double");
		//@statistic[assignedRequestsPerTime](title="Assigned Requests Per Time";source="assignedRequestsPerTime";record=vector;interpolationmode=none);
		       
        @signal[pickedupRequestsPerTime](type="double");
		//@statistic[pickedupRequestsPerTime](title="Picked-up Requests Per Time";source="pickedupRequestsPerTime";record=vector;interpolationmode=none);
        
        @signal[droppedoffRequestsPerTime](type="double");
		//@statistic[droppedoffRequestsPerTime](title="Dropped-off Requests Per Time";source="droppedoffRequestsPerTime";record=vector;interpolationmode=none);
		
		@signal[freeVehiclesPerTime](type="double");
		//@statistic[freeVehiclesPerTime](title="Free Vehicles Per Time";source="freeVehiclesPerTime";record=vector;interpolationmode=none);
	
		@signal[signal_civilEvacuated](type="int");       
        @statistic[signal_civilEvacuated](title="Number of civil evacuated";source="signal_civilEvacuated";record=vector,count;interpolation=none);
		
		@signal[differenceFromRequestToPickup](type="double");       
      	@statistic[differenceFromRequestToPickup](title="Difference From Emergency request creation To Pickup";source="differenceFromRequestToPickup";record=vector,mean;interpolation=none);

        @signal[emergencyRequest](type="int");       
        @statistic[emergencyRequest](title="Emergency requests in time";source="emergencyRequest";record=vector,count;interpolation=none);
       
		@signal[decayPheromoneValue](type="double");
		
		@signal[differenceFromRedCodeRequestToPickup](type="double");       
        @statistic[differenceFromRedCodeRequestToPickup](title="Difference From Red code Emergency request creation To Pickup";source="differenceFromRedCodeRequestToPickup";record=vector,mean;interpolation=none);
		
		@signal[redCodeRequest](type="int");       
        @statistic[redCodeRequest](title="Red codeEmergency requests in time";source="redCodeRequest";record=vector,count;interpolation=none);
       
		@signal[signal_noVehicle](type="int");       
        @statistic[signal_noVehicle](title="Requests lost in time";source="signal_noVehicle";record=vector,count;interpolation=none);
        
      	@signal[truckRequest](type="int");       
        @statistic[truckRequest](title="Number of truck request in time";source="truckRequest";record=vector,count;interpolation=none); 
          
      	@signal[pickupEmergencies](type="int");       
        @statistic[pickupEmergencies](title="Number of pickups emergencies";source="pickupEmergencies";record=vector,count;interpolation=none); 
       
}
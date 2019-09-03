package src.node;

//
// Node's application level
//
simple App like IApp
{
    parameters:
        int address;  			//local node address
        volatile int seatsPerVehicle = default(1);
        bool enableCivilTraffic = default(true);
        @display("i=block/browser");
        
        int CivilTrafficN; 		//Times every node generates civil vehicles
        
        
          // ------ SIGNALS -------- //
        @signal[signalDelayTravelTime](type="double");       	//Traveled distance per vehicle
        @statistic[signalDelayTravelTime](title="Delay between ideal time to serve a request and actual time to serve that request";source="signalDelayTravelTime";record=vector,stats;interpolation=none);
        
    gates:
        input in;
        output out;
}



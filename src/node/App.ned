package src.node;

//
// Node's application level
//
simple App like IApp
{
    parameters:
        int address;  			//local node address
        int seatsPerVehicle = default(1);
        double civilEscapeInterval @unit(s) = default(600s); 
        int numberOfCivils = default(1);
        @display("i=block/browser");

        
        
          // ------ SIGNALS -------- //
        @signal[signal_ambulanceDelayTravelTime](type="double");       	
        @statistic[signal_ambulanceDelayTravelTime](title="Ambulances Delay between ideal time to serve a request and actual time to serve that request";source="signal_ambulanceDelayTravelTime";record=vector,mean;interpolation=none);

        @signal[signal_truckTravelTime](type="double");       	
        @statistic[signal_truckTravelTime](title="Truck time to serve a request";source="signal_truckTravelTime";record=vector,mean;interpolation=none);

        @signal[signal_civilDelayTravelTime](type="double");       
        @statistic[signal_civilDelayTravelTime](title="Civil Delay between ideal time to serve a request and actual time to serve that request";source="signal_civilDelayTravelTime";record=vector,mean;interpolation=none);
        
        @signal[signal_ambulancesIdle](type="int");       
        @statistic[signal_ambulancesIdle](title="Number of idle ambulances in hospital";source="signal_ambulancesIdle";record=vector,mean;interpolation=none);
       
       
        @signal[signal_civilTravelTime](type="double");       
        @statistic[signal_civilTravelTime](title="Civil travel time to reach the destination";source="signal_civilTravelTime";record=vector,mean;interpolation=none);
    
   		


    gates:
        input in;
        output out;
}



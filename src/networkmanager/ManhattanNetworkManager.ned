
package src.networkmanager;

simple ManhattanNetworkManager like INetworkManager
{
    parameters:
        int numberOfVehicles = default(0);
        int numberOfNodes;      

        int numberOfEpicenters = default(0); 
        
        int numberOfCollectionPoints = default(1);
        int numberOfStoragePoints= default(1);
        
        int numberOfHospitals; 
        
        int numberOfEmergencyVehicles = default(0);
        int numberOfTrucks = default(0);
        
		double ambulanceSpeed @unit(mps);
		double truckSpeed @unit(mps);
		
		          // ------ SIGNALS -------- //
		@signal[signal_ambulanceTravelTime](type="double");       
        @statistic[signal_ambulanceTravelTime](title="Ambulance travel time to serve a request";source="signal_ambulanceTravelTime";record=vector,mean;interpolation=none);
		
        @display("i=block/network2_l");
        }

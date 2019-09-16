
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
		
		
        @display("i=block/network2_l");
        }

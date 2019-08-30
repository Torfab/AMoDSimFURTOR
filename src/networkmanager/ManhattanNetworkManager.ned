
package src.networkmanager;

simple ManhattanNetworkManager like INetworkManager
{
    parameters:
        int numberOfVehicles = default(1);
        int numberOfNodes;      
        int hospitalAddress; 
        int epicenterAddress; 
        int collectionPointAddress;
        
        int numberOfEmergencyVehicles = default(0);
        int numberOfTrucks = default(0);
        
		double ambulanceSpeed @unit(mps);
		double truckSpeed @unit(mps);
		
		int disasterRadius = default(1);
		
        @display("i=block/network2_l");
        }


package src.networkmanager;

simple ManhattanNetworkManager like INetworkManager
{
    parameters:
        int numberOfVehicles = default(1);
        int numberOfNodes;      
        int hospitalAddress; 
        int epicenterAddress; 
        int numberOfEmergencyVehicles = default(0);
		double ambulanceSpeed @unit(mps);
		double truckSpeed @unit(mps);
        @display("i=block/network2_l");
        }

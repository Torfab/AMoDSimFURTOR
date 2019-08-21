
package src.networkmanager;

simple ManhattanNetworkManager like INetworkManager
{
    parameters:
        int numberOfVehicles = default(1);
        int numberOfNodes;      
        int hospitalAddress;  
        int numberOfEmergencyVehicles = default(0);
        @display("i=block/network2_l");
}

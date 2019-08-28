package src.networkmanager;

moduleinterface INetworkManager
{
    parameters:
        int numberOfVehicles;
        int numberOfNodes;
        
		double ambulanceSpeed @unit(mps);
		double truckSpeed @unit(mps);
		@display("i=block/network2_l");    
}
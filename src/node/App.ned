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
        
        
    gates:
        input in;
        output out;
}



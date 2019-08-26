package src.node;

//
// Node's application level
//
simple App like IApp
{
    parameters:
        int address;  			//local node address
        int hospitalAddress;
        volatile int seatsPerVehicle = default(1);
        double StartTime @unit(s);
        @display("i=block/browser");
        
        int CivilTrafficN; 		//Times every node generates civil vehicles
        
        
    gates:
        input in;
        output out;
}



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
        
    gates:
        input in;
        output out;
}




package src.node;

simple TripRequestSubmitter
{
    parameters:
        int address;
        double maxDelay @unit(s) = default(2minute); //max allowed delay from pickup/dropoff request
        volatile double sendIaTime @unit(s)= default(exponential(2minute)); // time between generating packets           
        int numberOfEmergencies;
        
        @display("i=block/user");
        
        
        @signal[skilledRequests](type="int");       	//Traffic per channel
        @statistic[skilledRequests](title="skilledRequests";source="skilledRequests";record=vector,count;interpolation=none);
      


}

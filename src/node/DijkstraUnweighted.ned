 package src.node;

simple DijkstraUnweighted like IRouting
{
    parameters:
        // ------ SIGNALS -------- //
        @signal[signalFeromoneE](type="double");       	//Traveled distance per vehicle
        @statistic[signalFeromoneE](title="Feromone Canale E (destra)";source="signalFeromoneE";record=vector,count, min, max, mean;interpolation=linear);
        
        @signal[signalFeromoneW](type="double");       	//Traveled distance per vehicle
        @statistic[signalFeromoneW](title="Feromone Canale W (sinistra)";source="signalFeromoneW";record=vector,count, min, max, mean;interpolation=linear);
        
        @signal[signalFeromoneS](type="double");       	//Traveled distance per vehicle
        @statistic[signalFeromoneS](title="Feromone Canale S (south)";source="signalFeromoneS";record=vector,count, min, max, mean;interpolation=linear);
        
        @signal[signalFeromoneN](type="double");       	//Traveled distance per vehicle
        @statistic[signalFeromoneN](title="Feromone Canale N (north)";source="signalFeromoneN";record=vector,count, min, max, mean;interpolation=linear);
        
        @signal[signalTrafficE](type="double");       	//Traffic per channel
        @statistic[signalTrafficE](title="Traffic Canale E (destra)";source="signalTrafficE";record=vector,count, min, max, mean;interpolation=linear);
        
        @signal[signalTrafficW](type="double");       	//Traffic per channel
        @statistic[signalTrafficW](title="Traffic Canale W (sinistra)";source="signalTrafficW";record=vector,count, min, max, mean;interpolation=linear);
        
        @signal[signalTrafficS](type="double");       	//Traffic per channel
        @statistic[signalTrafficS](title="Traffic Canale S (south)";source="signalTrafficS";record=vector,count, min, max, mean;interpolation=linear);
        
        @signal[signalTrafficN](type="double");       	//Traffic per channel
        @statistic[signalTrafficN](title="Traffic Canale N (north)";source="signalTrafficN";record=vector,count, min, max, mean;interpolation=linear);
        
    gates:
        input in[];
        output out[];
        input localIn;
        output localOut;
}

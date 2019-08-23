package src.node;

simple ManhattanRouting like IRouting
{
    parameters:
        // ------ SIGNALS -------- //
        @signal[signalFeromoneE](type="double");       	//Traveled distance per vehicle
        @statistic[signalFeromoneE](title="Feromone Canale E (destra)";source="signalFeromoneE";record=vector,stats;interpolation=linear);
        
        @signal[signalFeromoneW](type="double");       	//Traveled distance per vehicle
        @statistic[signalFeromoneW](title="Feromone Canale W (sinistra)";source="signalFeromoneW";record=vector,stats;interpolation=linear);
        
        @signal[signalFeromoneS](type="double");       	//Traveled distance per vehicle
        @statistic[signalFeromoneS](title="Feromone Canale S (south)";source="signalFeromoneS";record=vector,stats;interpolation=linear);
        
        @signal[signalFeromoneN](type="double");       	//Traveled distance per vehicle
        @statistic[signalFeromoneN](title="Feromone Canale N (north)";source="signalFeromoneN";record=vector,stats;interpolation=linear);
        
        
    gates:
        input in[];
        output out[];
        input localIn;
        output localOut;
}

package src.node;

simple ManhattanRouting like IRouting
{
    parameters:
        double feromone_N = default(0);
        double feromone_S = default(0);
        double feromone_W = default(0);
        double feromone_E = default(0);

    gates:
        input in[];
        output out[];
        input localIn;
        output localOut;
}

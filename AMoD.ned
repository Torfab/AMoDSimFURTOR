import src.coordinator.ICoord;
import src.node.Node;
import src.networkmanager.INetworkManager;
import ned.DatarateChannel;

network AMoD
{
	parameters:
	    //------Pheromone-----
	    double pheromoneDecayFactor = default(0.1);  		// 10% 	Feromone decay factor
	    double pheromoneDecayTime @unit(s) = default(600s); // Feromone increase factor
	    
	    
	    //------Network Parameters------
        int height @prompt("Number of rows") = default(5); 	//nodes on y axes
        int width @prompt("Nodes per row") = default(5); 	//nodes on x axes
        double xNodeDistance @unit(m) = default(200m);		//Distance between nodes on X axes
        double yNodeDistance @unit(m) = default(200m);		//Distance between nodes on Y axes
        double h @unit(m)= height * yNodeDistance + 100m; 	//simulation area: y size
        double w @unit(m) = width * xNodeDistance + 100m; 	//simulation area: x size
        
        int density = default(100);
        
        //------Vehicle Parameters------
        double speed @unit(mps) = default (9.7mps); 		//Vehicles speed
        double acceleration @unit(mpss) = default (1.676mpss); //Vehicles acceleration/deceleration
        int boardingTime @unit(s) = default (0s);  			//Time needed for boarding passengers
        int alightingTime @unit(s) = default (0s); 			//Time needed for alighting passengers
        
        //------Channel Parameters------
        //--Don't touch 
        double xTravelTime @unit(s) = 0s; //(dropUnit(xNodeDistance)/dropUnit(speed))*1s; //time needed to travel a street at cruise speed
        double yTravelTime @unit(s) = 0s; //(dropUnit(yNodeDistance)/dropUnit(speed))*1s; //time needed to travel a street at cruise speed
        
        //------Coordinator type--------
        string coordType = default ("RadioTaxiCoord");
        
        //------NetworkManager type--------
        string netmanagerType = default ("AdaptiveNetworkManager");
        
        @display("bgb=$w,$h");
        
    types:
        channel xRoad extends ned.DatarateChannel
        {
            parameters:
                delay = xTravelTime; //time needed to travel the street (x axes). (propagation delay)
                double length @unit(m) = xNodeDistance;

        }
        
        channel yRoad extends ned.DatarateChannel
        {
            parameters:
                delay = yTravelTime; //time needed to travel the street (y axes). (propagation delay)
                double length @unit(m) = yNodeDistance;

        }

    submodules:
        netmanager: <netmanagerType> like INetworkManager{
            parameters:
                numberOfNodes = height * width;
        	@display("p=99800,50");
        }
                
        tcoord: <coordType> like ICoord{
            @display("p=99600,50");
        }
                
        n[height*width]: Node {
            parameters:
                address = index;
                x=index % width;
                y=index / width;
                @display("p=,,m,$width,$xNodeDistance,$yNodeDistance;b=100,100,rect,green");

                
            gates:
                port[4];
        }
        
        
    connections allowunconnected:
        for i=0..height-1, for j=0..width-1 {
            	n[i*width+j].port[2] <--> yRoad <--> n[(i+1)*width+j].port[0] if i!=height-1;  //Vertical connections
            	n[i*width+j].port[1] <--> xRoad <--> n[(i*width+j)+1].port[3] if j!=width-1;   //Horizontal connections
            }

   
}

/*
########################################################
##           __  __       _____   _____ _             ##
##     /\   |  \/  |     |  __ \ / ____(_)            ##
##    /  \  | \  / | ___ | |  | | (___  _ _ __ ___    ##
##   / /\ \ | |\/| |/ _ \| |  | |\___ \| | '_ ` _ \   ##
##  / ____ \| |  | | (_) | |__| |____) | | | | | | |  ##
## /_/    \_\_|  |_|\___/|_____/|_____/|_|_| |_| |_|  ##
##                                                    ##
## Author:                                            ##
##    Andrea Di Maria                                 ##
##    <andrea.dimaria90@gmail.com>                    ##
########################################################
*/

#include "ManhattanNetworkManager.h"

Define_Module(ManhattanNetworkManager);

std::set<int> ManhattanNetworkManager::buildSetOfDestroyedNodes() {

	int epicenterAddresses[numberOfEpicenters];

	for (int i=0; i<numberOfEpicenters; i++){
		epicenterAddresses[i] = intuniform(0,numberOfNodes-1);
		EV << "epicenter Address [" << i << " ] "<< epicenterAddresses[i] << endl;
		if (disasterRadius > 0)	// Creation of destroyed nodes set
				setOfDestroyedNodes.insert(epicenterAddresses[i]);
	}

	std::set<int> auxSet;
	for (int i = 1; i < disasterRadius; i++) {
		for (auto elem : setOfDestroyedNodes)
			auxSet = propagateEarthquakeBetweenNodes(elem, auxSet);
		setOfDestroyedNodes.insert(auxSet.begin(), auxSet.end());
	}
	return auxSet;
}

void ManhattanNetworkManager::buildSetOfNodesInRedZone(std::set<int> auxSet) {
	// Creation of red zones nodes set
	for (auto elem : setOfDestroyedNodes)
		auxSet = propagateEarthquakeBetweenNodes(elem, auxSet);
	setOfNodesInRedZone.insert(auxSet.begin(), auxSet.end());
	// Get nodes in red zones
	for (auto elem : setOfDestroyedNodes)
		setOfNodesInRedZone.erase(elem);
}

void ManhattanNetworkManager::buildSetOfBorderNodes() {
	for (int i = 0; i < columns; i++) {
		for (int j = 0; j < rows; j++) {
			if (i == 0 || j == 0 || i == (columns - 1) || j == (rows - 1)) {
				setOfBorderNodes.insert(j + i * rows);
			}
		}
	}
}
int ManhattanNetworkManager::pickRandomElemFromSet(std::set<int> s){
	if (s.empty() ) return -1;
	int rnd = intuniform(0, s.size() - 1);
	std::set<int>::const_iterator it(s.begin());
	advance(it, rnd);
	return *it;
}

void ManhattanNetworkManager::initialize()
{
    cModule* parentModule = getParentModule();
    rows = parentModule->par("width");
    columns = parentModule->par("height");
    hospitalAddress=par("hospitalAddress");
    collectionPointAddress=par("collectionPointAddress");

    numberOfVehicles = par("numberOfVehicles");
    numberOfNodes = par("numberOfNodes");
    numberOfEmergencyVehicles = par("numberOfEmergencyVehicles");
    numberOfTrucks = par("numberOfTrucks");

    ambulanceSpeed =  par("ambulanceSpeed");
    truckSpeed =  par("truckSpeed");

    disasterRadius = par("disasterRadius");
    numberOfEpicenters=par("numberOfEpicenters");



    xChannelLength = parentModule->par("xNodeDistance");
	yChannelLength = parentModule->par("yNodeDistance");

	additionalTravelTime = setAdditionalTravelTime(parentModule->par("speed"),parentModule->par("acceleration"));


	// Creation of destroyed nodes set
	std::set<int> auxSet = buildSetOfDestroyedNodes();
	// Creation of red zones nodes set
	buildSetOfNodesInRedZone(auxSet); // Don't change the order

	// Creation of border zones nodes set
	buildSetOfBorderNodes();


	// initialize truck start node
	truckStartNode = pickRandomElemFromSet(setOfBorderNodes);


    EV <<"TRUCKSTARTNODE  "<<truckStartNode<< " numberofTrucks "<<numberOfTrucks<< endl;

    vehiclesPerNode[truckStartNode]=numberOfTrucks;



    for (int i = 0; i < numberOfVehicles; i++)
                {
                int rand = intuniform(0, numberOfNodes - 1, 4);
                if (rand != hospitalAddress) //Nessun veicolo civile puo' partire dall'ospedale
                vehiclesPerNode[rand] += 1;
                }

    vehiclesPerNode[hospitalAddress] = numberOfEmergencyVehicles;

//	EV<<"nodi al limite"<<endl;
//	for(auto elem : setOfBorderNodes)
//	        EV <<elem<< "|";
//	EV<<endl;



}

/**
 * Return the space distance from current node to target one.
 *
 * @param srcAddr
 * @param dstAddress
 * @return
 */
double ManhattanNetworkManager::getSpaceDistance(int srcAddr, int dstAddr)
{
    double space_distance = 0;

    int xSource = srcAddr % rows;
    int xDest = dstAddr % rows;

    int ySource = srcAddr / rows;
    int yDest = dstAddr / rows;

    space_distance += abs(xSource - xDest) * xChannelLength;
    space_distance += abs(ySource - yDest) * yChannelLength;

    return space_distance;
}

/**
 * Return the hop distance from current node to target one.
 *
 * @param srcAddr
 * @param dstAddress
 * @return
 */
double ManhattanNetworkManager::getHopDistance(int srcAddr, int dstAddr)
{
    double hopDistance = 0;

    int xSource = srcAddr % rows;
    int xDest = dstAddr % rows;

    int ySource = srcAddr / rows;
    int yDest = dstAddr / rows;

    hopDistance += abs(xSource - xDest);
    hopDistance += abs(ySource - yDest);

    return hopDistance;
}

/**
 * Return the time distance from current node to target one.
 *
 * @param dstAddress
 * @return
 */
double ManhattanNetworkManager::getTimeDistance(int srcAddr, int dstAddr)
{
    double time_distance = 0;

    int xSource = srcAddr % rows;
    int xDest = dstAddr % rows;

    int ySource = srcAddr / rows;
    int yDest = dstAddr / rows;

    time_distance = abs(xSource - xDest) * xTravelTime;
    double yTime = abs(ySource - yDest) * yTravelTime;
    time_distance += yTime;

    if(time_distance != 0)
        time_distance+= additionalTravelTime;

    return time_distance;
}

/**
 * Return the vehicles started from nodeAddr.
 *
 * @param nodeAddr
 * @return
 */
int ManhattanNetworkManager::getVehiclesPerNode(int nodeAddr)
{
    int nVehicles = 0;
    std::map<int,int>::iterator it;

    it = vehiclesPerNode.find(nodeAddr);
    if (it != vehiclesPerNode.end())
       nVehicles = it->second;

    return nVehicles;
}

/**
 * Propagate the earthquake from epicenter node to neighbours.
 */
std::set<int> ManhattanNetworkManager::propagateEarthquakeBetweenNodes(int epicenterAddress, std::set<int> auxSet) {

	auxSet.insert(epicenterAddress);

    cTopology *topo = new cTopology("topo");

    std::vector<std::string> nedTypes;
    nedTypes.push_back("src.node.Node");
    topo->extractByNedTypeName(nedTypes);

    for (int i = 0; i < topo->getNumNodes(); i++) {

        cTopology::Node *node = topo->getNode(epicenterAddress);

        for (int j = 0; j < node->getNumOutLinks(); j++) {
            cTopology::Node *neighbour = node->getLinkOut(j)->getRemoteNode();
            auxSet.insert(neighbour->getModule()->getIndex());

        }


    }
    delete topo;
    return auxSet;
}

/**
 * Check if the specified address is valid.
 *
 * @param dstAddress
 * @return
 */
bool ManhattanNetworkManager::isValidAddress(int nodeAddr)
{
    if(nodeAddr >= 0 && nodeAddr < numberOfNodes)
        return true;
    return false;
}

/**
 * Return the outputGate index.
 *
 * @param dstAddress
 * @return
 */
int ManhattanNetworkManager::getOutputGate(int srcAddr, int dstAddr)
{
   return -1;
}

/**
 * Return the length of the channel connected to the specified gate.
 *
 * @param dstAddress
 * @param gateIndex
 * @return
 */
double ManhattanNetworkManager::getChannelLength(int nodeAddr, int gateIndex)
{
    return -1;
}

void ManhattanNetworkManager::handleMessage(cMessage *msg)
{

}

/** Check if current node is destroyed.
 *
 * @param addr
 * @return
 **/
bool ManhattanNetworkManager::checkDisconnectedNode(int addr) {

	for (auto elem : setOfDestroyedNodes) {

		if (elem == addr)
			return true;
	}
	return false;
}

bool ManhattanNetworkManager::checkBorderNode(int addr) {
	for (auto elem : setOfBorderNodes) {
			if (elem == addr)
				return true;
		}
		return false;
}
bool ManhattanNetworkManager::checkRedZoneNode(int addr) {
	for (auto elem : setOfNodesInRedZone) {
			if (elem == addr)
				return true;
		}
		return false;
}

int ManhattanNetworkManager::pickRandomNodeInRedZone() {
	return pickRandomElemFromSet(setOfNodesInRedZone);
}

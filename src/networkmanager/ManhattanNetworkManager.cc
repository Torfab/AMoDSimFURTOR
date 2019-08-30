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

void ManhattanNetworkManager::initialize()
{
    cModule* parentModule = getParentModule();
    rows = parentModule->par("width");
    columns = parentModule->par("height");
    hospitalAddress=par("hospitalAddress");

    numberOfVehicles = par("numberOfVehicles");
    numberOfNodes = par("numberOfNodes");
    numberOfEmergencyVehicles = par("numberOfEmergencyVehicles");

    ambulanceSpeed =  par("ambulanceSpeed");
    truckSpeed =  par("truckSpeed");

    disasterRadius = par("disasterRadius");
    epicenterAddress=par("epicenterAddress");


    for (int i = 0; i < numberOfVehicles; i++)
        {
        int rand = intuniform(0, numberOfNodes - 1, 4);
        if (rand != hospitalAddress) //Nessun veicolo civile puo' partire dall'ospedale
        vehiclesPerNode[rand] += 1;
        }

    vehiclesPerNode[hospitalAddress] = numberOfEmergencyVehicles;

    xChannelLength = parentModule->par("xNodeDistance");
	yChannelLength = parentModule->par("yNodeDistance");

	additionalTravelTime = setAdditionalTravelTime(parentModule->par("speed"),
			parentModule->par("acceleration"));

	if (disasterRadius > 0)
		setOfDestroyedNodes.insert(epicenterAddress);

	std::set<int> auxSet;

	for (int i = 1; i < disasterRadius; i++) {

		for(auto elem : setOfDestroyedNodes)
			auxSet=propagateEarthquakeBetweenNodes(elem, auxSet);

		setOfDestroyedNodes.insert(auxSet.begin(), auxSet.end());
	}
	EV << "BEGIN LIST: " << endl;
	for(auto elem : auxSet)
		EV << elem << " | ";
	EV << endl;


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

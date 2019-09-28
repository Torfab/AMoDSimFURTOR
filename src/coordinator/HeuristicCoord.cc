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
 ## Extended by:                                       ##
 ##    Francesco Furnari                               ##
 ##    <franz.furnari@gmail.com>                       ##
 ##    Fabrizio Tornatore                              ##
 ##    <torfab94@gmail.com>                            ##
 ##                                                    ##
 ########################################################
 */

#include <HeuristicCoord.h>
#include <algorithm>

Define_Module(HeuristicCoord);

void HeuristicCoord::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj) {
	if (signalID == tripRequest) {
		TripRequest *tr = check_and_cast<TripRequest *>(obj);
		EV << "New TRIP request from: " << source->getFullPath() << endl;

		if (isRequestValid(*tr)) {
			pendingRequests.insert(std::make_pair(tr->getID(), new TripRequest(*tr)));
			totrequests++;
			emit(totalRequestsPerTime, totrequests);

			handleTripRequest(tr);
		} else {
			EV << "The request " << tr->getID() << " is not valid!" << endl;
		}
	}
}

void HeuristicCoord::handleTripRequest(TripRequest *tr) {
	std::map<int, StopPointOrderingProposal*> vehicleProposals;

	for (auto const &x : vehicles) {
		if (tr->getIsSpecial() == 1) {  //the request want to go to an hospital
			if (x.first->getSpecialVehicle() == 1) {   //the vehicle considered is an ambulance

				//Evaluate the request considering the vehicle
				StopPointOrderingProposal *tmp = eval_Assignment(x.first->getID(), tr);
				if (tmp && !tmp->getSpList().empty()) {
					vehicleProposals[x.first->getID()] = tmp;
				}

			}

		}
		else if (tr->getIsSpecial() == 2) { // the request want to go to a coordination point - truck request
			if (x.first->getSpecialVehicle() == 2 ) { //the vehicle considered is a truck

			    //Evaluate the request considering the vehicle
				StopPointOrderingProposal *tmp = eval_Assignment(x.first->getID(), tr);
				if (tmp && !tmp->getSpList().empty()) {
					vehicleProposals[x.first->getID()] = tmp;
				}

			}
		}
		else if (tr->getIsSpecial() == 3) { // the request want to go to an hospital and it is a red code
			if (x.first->getSpecialVehicle() == 1){ //the vehicle considered is an ambulance

			    //Evaluate the request considering the ambulance
				StopPointOrderingProposal *tmp = eval_RedCodeEmergencyRequestAssignment(x.first->getID(), tr);
				if (tmp && !tmp->getSpList().empty()) {
					vehicleProposals[x.first->getID()] = tmp;
				}
			}
		}

	}
	// //Assign the request to the emergency vehicle
	if (tr->getIsSpecial() == 1 || tr->getIsSpecial() == 3 ) { //1 and 3 are both hospital request, 3 means it's also a redcode
		emergencyAssignment(vehicleProposals, tr);

	}
	// //Assign the request to the emergency vehicle
	else if (tr->getIsSpecial() == 2) {
//		EV << "+++" << vehicleProposals.size() << " Trucks are available for proposal" << endl;
		emergencyAssignment(vehicleProposals, tr);

	}
}


StopPointOrderingProposal* HeuristicCoord::eval_RedCodeEmergencyRequestAssignment(int vehicleID, TripRequest* tr) {

	std::list<StopPoint*> old = rPerVehicle[vehicleID];
	std::list<StopPoint*> newList;
	StopPoint* newTRpickup = new StopPoint(*tr->getPickupSP());
	StopPoint* newTRdropoff = new StopPoint(*tr->getDropoffSP());

	//the stopPoint knows it is a red code stopPoint
	newTRpickup->setRedCode(true);
	newTRdropoff->setRedCode(true);

	StopPointOrderingProposal* toReturn = NULL;

	double additionalCost = -1;
	double timeToPickup = 0;

	//The vehicle is empty
	if (rPerVehicle.find(vehicleID) == rPerVehicle.end() || old.empty()) {

		// request is from getLastVehicleLocation to the trip request location
		additionalCost = netmanager->getHopDistance(getLastVehicleLocation(vehicleID), newTRpickup->getLocation());
		additionalCost += netmanager->getHopDistance( newTRpickup->getLocation(), newTRdropoff->getLocation());

		EV << "Vehicle[" << vehicleID << "]: Cost to operate :" << additionalCost << " FROM " << getLastVehicleLocation(vehicleID) << " to " << newTRpickup->getLocation() << endl;
		EV << "COST :" << additionalCost << " FROM " << getLastVehicleLocation(vehicleID) << " to " << newTRpickup->getLocation() << endl;

		newList.push_back(newTRpickup);
		newList.push_back(newTRdropoff);

		toReturn = new StopPointOrderingProposal(vehicleID, vehicleID, additionalCost, timeToPickup, newList);

	}

	//The vehicle has 1 stop point
	else if (old.size() == 1) {
		EV << " The vehicle " << vehicleID << " has only 1 SP. This means the vehicle is going to an endpoint." << endl;

		StopPoint *last = new StopPoint(*old.back());

		// distance that the vehicle have to travel to reach his previous stoppoint
		additionalCost = netmanager->getHopDistance(getVehicleByID(vehicleID)->getSrcAddr(), getVehicleByID(vehicleID)->getDestAddr());
		// distance that the vehicle have to travel to reach this request
		additionalCost += netmanager->getHopDistance(getVehicleByID(vehicleID)->getDestAddr(), newTRpickup->getLocation());

		newList.push_back(last);
		newList.push_back(newTRpickup);
		newList.push_back(newTRdropoff);

		toReturn = new StopPointOrderingProposal(vehicleID, vehicleID, additionalCost, timeToPickup, newList);
//        delete last;
	}

	//The vehicle has more stop points
	else {
		EV << " The vehicle " << vehicleID << " has more stop points..." << endl;

		bool isDestinationHospital = netmanager->checkHospitalNode((*old.begin())->getLocation());

		if ((*old.begin())->isRedCode() || isDestinationHospital){

		    //the cost start from the cost from his source to his first stop point in the list
			additionalCost = netmanager->getHopDistance(getVehicleByID(vehicleID)->getSrcAddr(), (*old.begin())->getLocation());

			bool checkIt2InHospital = true; //there are 2 cases and we can discriminate between them with this boolean
			std::list<StopPoint*>::iterator it1, it2;
			for (it1 = old.begin(), it2 = ++old.begin(); it2 != old.end();	++it1, ++it2) {

			    //it2 found a non red code so i've to put my red code here
				if (!(*it2)->isRedCode()) {
					checkIt2InHospital = false;
					break;
				}

				// distance between two adjacent stop points
				additionalCost += netmanager->getHopDistance((*it1)->getLocation(), (*it2)->getLocation());	// netmanager->pickClosestHospitalFromNode(elem->getLocation()));

			}

			    //distance of the request considering last useful location
			additionalCost += netmanager->getHopDistance((*(it1))->getLocation(),newTRpickup->getLocation());


			//if the previous for loop break it means I've to insert my stop point in the middle of the list
			if (!checkIt2InHospital) {
				for (auto iter = old.begin(); iter != it2; iter++) {
					auto & value = *iter;
					newList.push_back(new StopPoint(*value)); //all the red code stop point won't change order
				}
				newList.push_back(newTRpickup); //the stop point pick up is placed here
				newList.push_back(newTRdropoff); //the stop point drop off is placed here here

				if (it1 != old.end()) {
					for (auto iter = it2; iter != old.end(); ++iter) {
						auto & value = *iter;
						newList.push_back(new StopPoint(*value)); //the other non red code stop points continue the previous order after the insretion
					}
				}

			} else {
				for (auto iter = old.begin(); iter != it1; ++iter) {
					auto & value = *iter;
					newList.push_back(new StopPoint(*value)); //all the red code stop point won't change order
				}
				newList.push_back(newTRpickup); //The stop point pick up is insert in the tail
				newList.push_back(newTRdropoff); //The stop point drop off is insert in the tail
			}

		}
		else {
		    //the cost is the distance from the source of considered vehicle
			additionalCost = netmanager->getHopDistance(getLastVehicleLocation(vehicleID), newTRpickup->getLocation()); //from last registered location to the pickup

			/**
			 * due his priority, the red code is put in front
			 */
			newList.push_back(newTRpickup);
			newList.push_back(newTRdropoff);
			for (auto const &x : old) //all non red code stop points continue the previous order
				newList.push_back(new StopPoint(*x));

		}
		additionalCost += netmanager->getHopDistance(newTRpickup->getLocation(), newTRdropoff->getLocation()); //it has to be added the distance between pickup and dropoff of the request

		EV << " Total cost " << additionalCost << " for Request " << tr->getID() << endl;
		toReturn = new StopPointOrderingProposal(vehicleID, vehicleID, additionalCost, timeToPickup, newList);

	}

	return toReturn;
}



/**
 * Evaluates a truck or emergency request
 */
StopPointOrderingProposal* HeuristicCoord::eval_Assignment(int vehicleID, TripRequest* tr) {

	std::list<StopPoint*> old = rPerVehicle[vehicleID];
	std::list<StopPoint*> newList;
	StopPoint* newTRpickup = new StopPoint(*tr->getPickupSP());
	StopPoint* newTRdropoff = new StopPoint(*tr->getDropoffSP());
	StopPointOrderingProposal* toReturn = NULL;
	double additionalCost = -1;
	double timeToPickup = 0;    //not used by heuristic

	//The vehicle is empty
	if (rPerVehicle.find(vehicleID) == rPerVehicle.end() || old.empty()) {

		// request is from getLastVehicleLocation to the trip request location
		additionalCost = netmanager->getHopDistance(getLastVehicleLocation(vehicleID), newTRpickup->getLocation());
		additionalCost += netmanager->getHopDistance( newTRpickup->getLocation(), newTRdropoff->getLocation());

		EV << "Vehicle[" << vehicleID << "]: Cost to operate :" << additionalCost << " FROM " << getLastVehicleLocation(vehicleID) << " to " << newTRpickup->getLocation() << endl;

		newList.push_back(newTRpickup);
		newList.push_back(newTRdropoff);

		toReturn = new StopPointOrderingProposal(vehicleID, vehicleID, additionalCost, timeToPickup, newList);

	}

	//The vehicle has 1 stop point
	else if (old.size() == 1) {
		EV << " The vehicle " << vehicleID << " has only 1 SP. This means the truck is going to an endpoint." << endl;

		StopPoint *last = new StopPoint(*old.back());
		additionalCost = netmanager->getHopDistance(getVehicleByID(vehicleID)->getSrcAddr(), getVehicleByID(vehicleID)->getDestAddr());
		additionalCost += netmanager->getHopDistance(getVehicleByID(vehicleID)->getDestAddr(), newTRpickup->getLocation());

		newList.push_back(last);
		newList.push_back(newTRpickup);
		newList.push_back(newTRdropoff);

		toReturn = new StopPointOrderingProposal(vehicleID, vehicleID, additionalCost, timeToPickup, newList);

//	    delete last;
	}
	//The vehicle has more stop points
	else {
		EV << " The vehicle " << vehicleID << " has more stop points..." << endl;

		StopPoint *last = new StopPoint(*old.back());

		additionalCost= netmanager->getHopDistance(getVehicleByID(vehicleID)->getSrcAddr(), (*old.begin())->getLocation());

		for (std::list<StopPoint*>::iterator it1 = old.begin(), it2 = ++old.begin();
				it2 != old.end(); ++it1, ++it2) {
			additionalCost += netmanager->getHopDistance((*it1)->getLocation(),(*it2)->getLocation());

		}
		additionalCost += netmanager->getHopDistance(last->getLocation(), newTRpickup->getLocation());
		additionalCost += netmanager->getHopDistance(newTRpickup->getLocation(), newTRdropoff->getLocation());

		EV << " Total cost " << additionalCost << " for Request " << tr->getID() << endl;

		//the new list is the same of the previous with new request in tail
		for (auto const &x : old)
			newList.push_back(new StopPoint(*x));

		newList.push_back(newTRpickup);
		newList.push_back(newTRdropoff);

		toReturn = new StopPointOrderingProposal(vehicleID, vehicleID, additionalCost, timeToPickup, newList);

	}

	return toReturn;
}



StopPointOrderingProposal* HeuristicCoord::eval_requestAssignment(int vehicleID, TripRequest* tr) {
    return nullptr;
}

std::list<StopPointOrderingProposal*> HeuristicCoord::addStopPointToTrip(int vehicleID, std::list<
		StopPoint*> spl, StopPoint* newSP) {
    std::list<StopPointOrderingProposal*> notused;
    return notused;
}

//Get the additional time-cost allowed by each stop point
std::list<double> HeuristicCoord::getResidualTime(std::list<StopPoint*> spl, int requestID) {
	std::list<double> residuals;
	return residuals;
}


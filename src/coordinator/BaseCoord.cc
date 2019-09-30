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

#include <BaseCoord.h>
#include <sstream>


void BaseCoord::initialize()
{
    /* ---- REGISTER SIGNALS ---- */
    tripRequest = registerSignal("tripRequest");
    newTripAssigned = registerSignal("newTripAssigned");

    decayPheromoneValue = registerSignal("decayPheromoneValue");

    traveledDistance = registerSignal("traveledDistance");
    waitingTime = registerSignal("waitingTime");
    actualTripTime = registerSignal("actualTripTime");
    stretch = registerSignal("stretch");
    tripDistance = registerSignal("tripDistance");
    passengersOnBoard = registerSignal("passengersOnBoard");
    toDropoffRequests = registerSignal("toDropoffRequests");
    toPickupRequests = registerSignal("toPickupRequests");
    requestsAssignedPerVehicle = registerSignal("requestsAssignedPerVehicle");

    totalRequestsPerTime = registerSignal("totalRequestsPerTime");
    assignedRequestsPerTime = registerSignal("assignedRequestsPerTime");
    pickedupRequestsPerTime = registerSignal("pickedupRequestsPerTime");
    droppedoffRequestsPerTime = registerSignal("droppedoffRequestsPerTime");
    freeVehiclesPerTime = registerSignal("freeVehiclesPerTime");

    differenceFromRequestToPickup = registerSignal("differenceFromRequestToPickup");
    differenceFromRedCodeRequestToPickup = registerSignal("differenceFromRedCodeRequestToPickup");
    emergencyRequest = registerSignal("emergencyRequest");
    redCodeRequest= registerSignal("redCodeRequest");
    truckRequest= registerSignal("truckRequest");

    signal_noVehicle= registerSignal("signal_noVehicle");

    pickupEmergencies = registerSignal("pickupEmergencies");
    deadCode = registerSignal("deadCode");

    pickupEmergenciesCount = 0;

    totrequests = 0.0;
    totalAssignedRequests = 0.0;
    totalPickedupRequests = 0.0;
    totalDroppedoffRequest = 0.0;

    alightingTime = getParentModule()->par("alightingTime").doubleValue();
    boardingTime = getParentModule()->par("boardingTime").doubleValue();
    requestAssignmentStrategy = par("requestAssignmentStrategy");
    netmanager = check_and_cast<AbstractNetworkManager *>(getParentModule()->getSubmodule("netmanager"));
    freeVehicles = netmanager->getNumberOfVehicles();
    emit(freeVehiclesPerTime, freeVehicles);

    //netXsize = (getParentModule()->par("width").doubleValue() - 1) * (getParentModule()->par("nodeDistance").doubleValue());
    //netYsize = (getParentModule()->par("height").doubleValue() - 1) * (getParentModule()->par("nodeDistance").doubleValue());

    signal_civilEvacuated = registerSignal("signal_civilEvacuated");
    civilCounter = 0;

    simulation.getSystemModule()->subscribe("tripRequest",this);

    redCodeRequestCounter = 0;
    emergencyRequestCounter = 0;
    truckRequestCounter= 0;
    deadCount = 0;
	//Pheromone
	pheromoneDecayTime = getParentModule()->par("pheromoneDecayTime");
	pheromoneDecayFactor = getParentModule()->par("pheromoneDecayFactor");
    decayPacket = new cMessage("decayPacket");
    scheduleAt(simTime() + pheromoneDecayTime,decayPacket);


}
BaseCoord::~BaseCoord()
{
    cancelAndDelete(decayPacket);
}
void BaseCoord::handleMessage(cMessage *msg) {

	if (msg->isSelfMessage()) {
		emit(decayPheromoneValue, true);
		scheduleAt(simTime() + pheromoneDecayTime, decayPacket);
	}
}

/**
 * Assign the new trip request to the vehicle which minimize the pickup waiting time.
 *
 * @param vehicleProposal The vehicles proposals
 * @param tr The new TripRequest
 *
 * @return The ID of the vehicle which will serve the request or -1 otherwise.
 */
int BaseCoord::minWaitingTimeAssignment (std::map<int,StopPointOrderingProposal*> vehicleProposal, TripRequest *tr)
{
    double pickupDeadline = tr->getPickupSP()->getTime() + tr->getPickupSP()->getMaxDelay();
//    double dropoffDeadline = tr->getDropoffSP()->getTime() + tr->getDropoffSP()->getMaxDelay();
    double pickupActualTime = -1.0;
//    double dropoffActualTime = -1.0;
    int vehicleID = -1;

    //The request has been evaluated
    TripRequest *preq = pendingRequests[tr->getID()];
    pendingRequests.erase(tr->getID());
    delete preq;


    for(auto const &x : vehicleProposal)
    {
        double actualPickupTime = x.second->getActualPickupTime();
        if(actualPickupTime <= pickupDeadline)
        {
             if(pickupActualTime == -1.0 ||  actualPickupTime < pickupActualTime)
             {
                 if(vehicleID != -1) //The current proposal is better than the previous one
                     delete(vehicleProposal[vehicleID]);

                 vehicleID = x.first;
                 pickupActualTime = actualPickupTime;
                 //dropoffActualTime = actualDropoffTime;
             }
             else
                 delete x.second; //Reject the current proposal (A better one has been accepted)
         }
         else
             delete x.second; //Reject the current proposal: it does not respect the time constraints
    }


      if(pickupActualTime > -1)
      {
          EV << "Accepted request of vehicle "<< vehicleID << " for request: " << tr->getID() << " .Actual PICKUP time: " << pickupActualTime
             << "/Requested Pickup Deadline: " << pickupDeadline << endl;
             //" .Actual DROPOFF time: " << dropoffActualTime << "/Requested DropOFF Deadline: " << dropoffDeadline << endl;

          updateVehicleStopPoints(vehicleID, vehicleProposal[vehicleID]->getSpList(), getRequestPickup(vehicleProposal[vehicleID]->getSpList(),tr->getID()));
      }
      else
      {
          EV << "1 No vehicle in the system can serve the request " << tr->getID() << endl;
          uRequests[tr->getID()] = new TripRequest(*tr);
          delete tr;
          return -1;
      }
      delete tr;

      return vehicleID;
}


/**
 * Assign the new trip request to the first truck available in the vehicles proposal.
 *
 * @param vehicleProposal The vehicles proposals
 * @param tr The new TripRequest
 *
 * @return The ID of the vehicle which will serve the request or -1 otherwise.
 */
int BaseCoord::truckAssignment(std::map<int, StopPointOrderingProposal*> vehicleProposal, TripRequest *tr) {
//    double pickupDeadline = tr->getPickupSP()->getTime()+ tr->getPickupSP()->getMaxDelay()*100; //
    double additionalCost = -1.0;
    int vehicleID = -1;

    //The request has been evaluated
    TripRequest *preq = pendingRequests[tr->getID()];
    pendingRequests.erase(tr->getID());
    delete preq;
    if (!vehicleProposal.empty()) {
        double curAdditionalCost =vehicleProposal.begin()->second->getAdditionalCost();
        vehicleID = vehicleProposal.begin()->first;
        additionalCost = curAdditionalCost;

    }
    if (additionalCost > -1) {
        EV << "Accepted request of truck" << vehicleID << " for request: "
                  << tr->getID() << " .The time cost is: " << additionalCost << endl;

        updateVehicleStopPoints(vehicleID, vehicleProposal[vehicleID]->getSpList(),getRequestPickup(vehicleProposal[vehicleID]->getSpList(),tr->getID()));
    } else {
        EV << "2 No vehicle in the system can serve the request " << tr->getID()<< endl;
        uRequests[tr->getID()] = new TripRequest(*tr);
        delete tr;
        return -1;
    }
    delete tr;

    return vehicleID;
}

void BaseCoord::checkRemainingTime(StopPoint* red) {
	if (red->remainingTime() < 0) {
		pendingRedStopPoints.remove(red);
		emit(deadCode, ++deadCount);
	}
}

/**
 * Assign the new trip request to the first emergency vehicle available in the vehicles proposal.
 *
 * @param vehicleProposal The vehicles proposals
 * @param tr The new TripRequest
 *
 * @return The ID of the vehicle which will serve the request or -1 otherwise.
 */
int BaseCoord::emergencyAssignment(std::map<int, StopPointOrderingProposal*> vehicleProposal, TripRequest *tr) {

    double additionalCost = -1.0;
    int vehicleID = -1;


    //The request has been evaluated
    TripRequest *preq = pendingRequests[tr->getID()];
    pendingRequests.erase(tr->getID());
    delete preq;
    if (!vehicleProposal.empty()) {
		int min = -1;
		for (auto const &x : vehicleProposal) { /// Ricerca del minimo numero di hop
			// hospital closest to last vehicle location
			if (min == -1)
				min = x.second->getAdditionalCost() + 1;
			if (x.second->getAdditionalCost() < min) {
				vehicleID = x.first;
				min = x.second->getAdditionalCost();
			}
		}

	}
	if (vehicleID != -1) {
		EV << "Accepted request of emergency vehicle " << vehicleID << " for request: " << tr->getID() << " .The time cost is: " << additionalCost << endl;
		if(tr->getIsSpecial()==3){ //ridondante ma utile
		    tr->getPickupSP()->setRedCode(true);
		    tr->getDropoffSP()->setRedCode(true);
		}
		updateVehicleStopPoints(vehicleID, vehicleProposal[vehicleID]->getSpList(), tr->getPickupSP());
		EV << "stop points updated! " << endl;
		for (auto elem : vehicleProposal[vehicleID]->getSpList())
			EV << elem->getLocation() << " code: " << elem->isRedCode() << " request: " << elem->getRequestID() << endl;
	} else {
		EV << "3 No vehicle in the system can serve the request " << tr->getID() << endl;
		emit(signal_noVehicle, 1);

		//va in coda al coordinatore
		// la coda verra' smaltita dalla prima ambulanza libera

		if (tr->getIsSpecial() == 3) { //red code request
//			if (tr->isInFront())
//				pendingRedStopPoints.push_front(new StopPoint(*tr->getPickupSP()));
//			else
			pendingRedStopPoints.push_back(new StopPoint(*tr->getPickupSP()));

			//posizionare nella lista
			tr->getPickupSP()->getMaxDelay();

			EV << "rosse prima" << endl;
			for (auto red : pendingRedStopPoints){
				checkRemainingTime(red);
				EV << "pendingredstoppoints: " << red->getLocation() << "   " << red->remainingTime() << endl;
			}


			pendingRedStopPoints.sort([]( StopPoint *sp1, StopPoint *sp2)
				{
					return sp1->remainingTime() < sp2->remainingTime();
				});

			EV << "RED PENDIng lista : " << endl;
			for (auto elem : pendingRedStopPoints) {
				EV << "pendingredstoppoints: " << elem->getLocation() << "   " << elem->remainingTime() << endl;
			}



		} else {
			if (tr->isInFront())
				pendingStopPoints.push_front(new StopPoint(*tr->getPickupSP()));
			else
				pendingStopPoints.push_back(new StopPoint(*tr->getPickupSP()));
			EV << "normali PENDIng lista : ";
			for (auto elem : pendingStopPoints) {
				EV << elem->getLocation() << endl;
			}
		}

		uRequests[tr->getID()] = new TripRequest(*tr);
		delete tr;
		return -1;
	}
	delete tr;

    return vehicleID;
}



/**
 * Assign the new trip request to the vehicle which minimize the additional time cost.
 *
 * @param vehicleProposal The vehicles proposals
 * @param tr The new TripRequest
 *
 * @return The ID of the vehicle which will serve the request or -1 otherwise.
 */
int BaseCoord::minCostAssignment(std::map<int, StopPointOrderingProposal*> vehicleProposal, TripRequest *tr) {
    double pickupDeadline = tr->getPickupSP()->getTime()+ tr->getPickupSP()->getMaxDelay();
    double additionalCost = -1.0;
    int vehicleID = -1;

    //The request has been evaluated
    TripRequest *preq = pendingRequests[tr->getID()];
    pendingRequests.erase(tr->getID());
    delete preq;

    for (auto const &x : vehicleProposal) {
        double curAdditionalCost = x.second->getAdditionalCost();
        if (x.second->getActualPickupTime() <= pickupDeadline) {
            if (additionalCost == -1.0 || curAdditionalCost < additionalCost) {
                if (vehicleID != -1) //The current proposal is better than the previous one
                    delete (vehicleProposal[vehicleID]);

                vehicleID = x.first;
                additionalCost = curAdditionalCost;
            } else
                delete x.second; //Reject the current proposal (A better one has been accepted)
        } else
            delete x.second; //Reject the current proposal: it does not respect the time constraints
    }

    if (additionalCost > -1) {
        EV << "Accepted request of vehicle " << vehicleID << " for request: "
                  << tr->getID() << " .The time cost is: " << additionalCost << endl;

        updateVehicleStopPoints(vehicleID, vehicleProposal[vehicleID]->getSpList(),getRequestPickup(vehicleProposal[vehicleID]->getSpList(),tr->getID()));
    } else {
        EV << "4 No vehicle in the system can serve the request " << tr->getID()<< endl;
        uRequests[tr->getID()] = new TripRequest(*tr);
        delete tr;
        return -1;
    }
    delete tr;

    return vehicleID;
}


/**
 * Update the list of stop points assigned to a vehicle.
 *
 * @param vehicleID The vehicle ID
 * @param spList The list of stop points.
 *
 */
void BaseCoord::updateVehicleStopPoints(int vehicleID, std::list<StopPoint*> spList, StopPoint *pickupSP)
{
      EV<<"sto aggiornando gli stoppoint di "<< vehicleID<<endl;
      rAssignedPerVehicle[vehicleID]++;
      totalAssignedRequests++;
      emit(assignedRequestsPerTime, totalAssignedRequests);

      bool toEmit = false;
      if(rPerVehicle[vehicleID].empty())
      {
          EV<< vehicleID<<" era vuoto "<<endl;
          //The node which handle the selected vehicle should be notified
          toEmit = true;
          freeVehicles--;
          emit(freeVehiclesPerTime, freeVehicles);

          updateStateElapsedTime(vehicleID, -1); //update IDLE elapsed time
      }
      else
      {
          EV<< vehicleID<<" era pieno, quanto rosso è pickup: "<< pickupSP->isRedCode()<<endl;
		//clean the old stop point list assigned to the vehicle
		if (pickupSP->isRedCode()) {
		    EV<< vehicleID<<" era pieno sposto "<<endl;

			for (auto &elem : rPerVehicle[vehicleID]) {
				EV << "la richiesta rossa ha spostato " << elem->getLocation() << endl;
				if (!netmanager->checkHospitalNode(elem->getLocation())) {
					//pendingStopPoints.push_front(new StopPoint(*elem)); //inseriti in testa alla coda tutti gli stop point rimanenti sovrascritti dalla rossa

					int code;
					StopPoint *pickupSP = new StopPoint(*elem);

					StopPoint *dropoffSP = new StopPoint(-1, netmanager->pickClosestHospitalFromNode(elem->getLocation()), false, simTime().dbl(), 0);

					if (elem->isRedCode()) { //TODO: permettere ad una rossa urgente di scavalcare una rossa poco urgente (valutare eventualmente se richiede skilled)
						code = 3;
						dropoffSP->setRedCode(true);
					} else
						code = 1;
					//crea trip request in base al tipo di sp
					TripRequest *request = new TripRequest();
					request->setPickupSP(pickupSP);
					request->setDropoffSP(dropoffSP);
					request->setInFront(true);
					request->setIsSpecial(code);
					 //emetti triprequest

					 emit(tripRequest, request);
				}
			}
		}

          cleanStopPointList(rPerVehicle[vehicleID]);
      }

      rPerVehicle[vehicleID] = spList;
      if(toEmit)
      {
          (statePerVehicle[vehicleID][0])->setStartingTime(simTime().dbl());
          emit(newTripAssigned, (double)vehicleID);
      }
}


/**
 * Emit statistical signal before end the simulation
 */
void BaseCoord::finish()
{
	/*
    //--- Total Requests Statistic ---
    char totalRequestSignal[32];
    sprintf(totalRequestSignal, "Total Requests");
    recordScalar(totalRequestSignal, totrequests);

    //--- Unserved Requests Statistic ---
    char unservedRequestSignal[32];
    sprintf(unservedRequestSignal, "Unserved Requests");
    recordScalar(unservedRequestSignal, uRequests.size());

    //--- Pending Requests Statistic ---
    char pendingRequestSignal[32];
    sprintf(pendingRequestSignal, "Pending Requests");
    recordScalar(pendingRequestSignal, pendingRequests.size());
    */

    /*Define vectors for additional statistics (Percentiles) */
    std::vector<double> traveledDistanceVector;
    std::vector<double> requestsAssignedPerVehicleVector;
    std::vector<double> passengersOnBoardVector;
    std::vector<double> toDropoffRequestsVector;
    std::vector<double> toPickupRequestsVector;
    std::map<int, std::vector<double>> statsPerVehiclesVectors;
    double totalToPickup = 0.0;

    /* Register the Travel-Time related signals */
    /*  int maxSeats = getMaxVehiclesSeats();
    std::map<int, simsignal_t> travelStats;
    for(int i=-1; i<=maxSeats; i++)
    {
        char sigName[32];
        sprintf(sigName, "travel%d-time", i);
        simsignal_t travsignal = registerSignal(sigName);
        travelStats[i] = travsignal;

        char statisticName[32];
        sprintf(statisticName, "travel%d-time", i);
        cProperty *statisticTemplate =
            getProperties()->get("statisticTemplate", "travelTime");

        ev.addResultRecorders(this, travsignal, statisticName, statisticTemplate);
    }
   */

    /*--- Per Vehicle related Statistics ---*/
    /*for(std::map<Vehicle*, int>::iterator itr = vehicles.begin(); itr != vehicles.end(); itr++)
    {
        double tmp = (itr->first->getTraveledDistance())/1000;
        emit(traveledDistance, tmp);
            traveledDistanceVector.push_back(tmp);

        tmp=rAssignedPerVehicle[itr->first->getID()];
        emit(requestsAssignedPerVehicle, tmp);
            requestsAssignedPerVehicleVector.push_back(tmp);

        std::map<int, std::list<StopPoint*>>::const_iterator it = rPerVehicle.find(itr->first->getID());
        if(it == rPerVehicle.end() || it->second.empty())
        {
            emit(passengersOnBoard, 0.0);
                passengersOnBoardVector.push_back(0.0);
            emit(toDropoffRequests, 0.0);
                toDropoffRequestsVector.push_back(0.0);
            emit(toPickupRequests, 0.0);
                toPickupRequestsVector.push_back(0.0);
        }
        else
        {
            StopPoint* nextSP = it->second.front();
            tmp=(double)nextSP->getActualNumberOfPassengers() - nextSP->getNumberOfPassengers();
            emit(passengersOnBoard, tmp);
                passengersOnBoardVector.push_back(tmp);

            int pickedupRequests = countOnBoardRequests(itr->first->getID());
            emit(toDropoffRequests, (double)pickedupRequests);
                toDropoffRequestsVector.push_back((double)pickedupRequests);
            double tpr = (double)((it->second.size() - pickedupRequests)/2);
            emit(toPickupRequests, (double)((it->second.size() - pickedupRequests)/2));
                toPickupRequestsVector.push_back(tpr);
                totalToPickup += tpr;
        }


        for(auto const& x : statePerVehicle[itr->first->getID()])
        {
            tmp= x.second->getElapsedTime() / 60;
            //emit(travelStats[x.first], tmp);
            statsPerVehiclesVectors[x.first].push_back(tmp);
        }

        vehicles.erase(itr);
    }
    */
    /*--- Total To Pickup ---*/
    /* char totalRequestsToPickup[32];
    sprintf(totalRequestsToPickup, "Total Requests To Pickup");
    recordScalar(totalRequestsToPickup, totalToPickup);
    */
    /* -- Collect Percentile Statistic -- */
    /*
    collectPercentileStats("traveledDistance", traveledDistanceVector);
    collectPercentileStats("requestsPerVehicle", requestsAssignedPerVehicleVector);
    collectPercentileStats("passengersOnBoard", passengersOnBoardVector);
    collectPercentileStats("toDropoffRequests", toDropoffRequestsVector);
    collectPercentileStats("toPickupRequests", toPickupRequestsVector);
    collectPercentileStats("waitingTime", waitingTimeVector);
    collectPercentileStats("actualTripTime", actualTripTimeVector);
    collectPercentileStats("stretch",stretchVector);
    collectPercentileStats("tripDistance", tripDistanceVector);

    for(auto const& x : statsPerVehiclesVectors)
    {
        char statisticName[32];
        sprintf(statisticName, "travel%d-time", x.first);
        if (!x.second.empty())
            collectPercentileStats(statisticName, x.second);
    }
    */

    /*------------------------------- CLEAN ENVIRONMENT -------------------------------*/




    for(std::map<int, TripRequest*>::iterator itr = pendingRequests.begin(); itr != pendingRequests.end(); itr++)
        delete itr->second;

    for(std::map<int, TripRequest*>::iterator itr = uRequests.begin(); itr != uRequests.end(); itr++)
        delete itr->second;

    for(std::map<int, std::list<StopPoint*>>::iterator itr = rPerVehicle.begin(); itr != rPerVehicle.end(); itr++)
        cleanStopPointList(itr->second);

    for(std::map<int, StopPoint*>::iterator itr = servedPickup.begin(); itr != servedPickup.end(); itr++)
        delete itr->second;

    for(std::map<int, std::map<int, VehicleState*>>::iterator itr = statePerVehicle.begin(); itr != statePerVehicle.end(); itr++)
        for(std::map<int, VehicleState*>::iterator itr2 = itr->second.begin(); itr2 != itr->second.end(); itr2++)
            delete itr2->second;


    /*------------------------------- END CLEAN ENVIRONMENT ----------------------------*/
}

/**
 * Get the number of picked-up requests not yet dropped-off.
 *
 * @param vehicleID
 *
 * @return number of picked-up requests.
 */
int BaseCoord::countOnBoardRequests(int vehicleID)
{
    std::list<StopPoint*> requests = rPerVehicle[vehicleID];
    int onBoard = 0;
    std::set<int> pickupSP;

    for (std::list<StopPoint*>::const_iterator it=requests.begin(); it != requests.end(); ++it)
    {
        if((*it)->getIsPickup())
            pickupSP.insert((*it)->getRequestID());
        else
            if(pickupSP.find((*it)->getRequestID()) == pickupSP.end())
                onBoard++;
    }
    return onBoard;
}

/**
 * Get the pointer to the pickup SP related to the requestID.
 *
 * @param spList List of stop-point where look for the pickup.
 * @param requestID The ID of the TripRequest
 *
 * @return The StopPoint or NULL
 */
StopPoint* BaseCoord::getRequestPickup(std::list<StopPoint*> spList, int requestID)
{
    for (std::list<StopPoint*>::iterator it=spList.begin(); it != spList.end(); ++it)
    {
        if(((*it)->getRequestID() == requestID) && (*it)->getIsPickup())
            return (*it);
    }

    return nullptr;
}


/**
 * Get the pointer to the dropoff SP related to the requestID.
 *
 * @param spList List of stop-point where look for the dropoff.
 * @param requestID The ID of the TripRequest
 *
 * @return The StopPoint or NULL
 */
StopPoint* BaseCoord::getRequestDropOff(std::list<StopPoint*> spList, int requestID)
{
    for (std::list<StopPoint*>::iterator it=spList.begin(); it != spList.end(); ++it)
    {
        if(((*it)->getRequestID() == requestID) && !(*it)->getIsPickup())
            return (*it);
    }

    return nullptr;
}


/**
 * Get the next stop point for the specified vehicle.
 *
 * @param vehicleID
 * @return
 */
StopPoint* BaseCoord::getNextStopPoint(int vehicleID)
{
    if ((rPerVehicle.find(vehicleID) != rPerVehicle.end()) && !(rPerVehicle[vehicleID].empty()))
    {
        StopPoint *front = rPerVehicle[vehicleID].front();
        int currentPassengers = front->getActualNumberOfPassengers();
        rPerVehicle[vehicleID].pop_front();

        if (!rPerVehicle[vehicleID].empty())
        {
            VehicleState *currState = statePerVehicle[vehicleID][currentPassengers];
            currState->setStartingTime(simTime().dbl());

            StopPoint *r = rPerVehicle[vehicleID].front();
//            for (auto elem : rPerVehicle[vehicleID])
//                   	EV << elem->getLocation() << " code: " << elem->isRedCode() << " request: " << elem->getRequestID() <<   endl;
            delete front;
            return r;
        }
        delete front;
    }

    VehicleState *idleState = statePerVehicle[vehicleID][-1];
    idleState->setStartingTime(simTime().dbl());
    freeVehicles++;
    emit(freeVehiclesPerTime, freeVehicles);

    return NULL;
}


/**
 * Get the current stop point for the specified vehicle.
 * Call the function when the vehicle reach a stop-point location.
 *
 * @param vehicleID
 * @return The pointer to the current stop point
 */
StopPoint* BaseCoord::getCurrentStopPoint(int vehicleID)
{
    if ((rPerVehicle.find(vehicleID) != rPerVehicle.end()) && !(rPerVehicle[vehicleID].empty()))
    {
        StopPoint *r = rPerVehicle[vehicleID].front();
        return r;
    }
    return NULL;
}

/**
 * Get the pointer to the first stop point assigned to the vehicle.
 * Call this function when receive a "newTripAssigned" signal.
 *
 * @param vehicleID
 * @return The pointer to the first stop point
 */
StopPoint* BaseCoord::getNewAssignedStopPoint(int vehicleID)
{
    return rPerVehicle[vehicleID].front();
}

/**
 * Update the elapsed time of the specified TravelState.
 *
 * @param vehicleID
 * @param stateID
 */
void BaseCoord::updateStateElapsedTime(int vehicleID, int stateID)
{
    VehicleState *prevState = statePerVehicle[vehicleID][stateID];
    prevState->setElapsedTime(prevState->getElapsedTime() + (simTime().dbl() - prevState->getStartingTime()));

    EV << "Elapsed Time for state "<< stateID << " is: " << prevState->getElapsedTime() << endl;
}

/**
 * Register the vehicle v in a node.
 *
 * @param v
 * @param address The node address
 */
void BaseCoord::registerVehicle(Vehicle *v, int address)
{
    if(statePerVehicle.find(v->getID()) == statePerVehicle.end())
    {
        double currTime = simTime().dbl();

        for(int i=-1; i<=v->getSeats(); i++)
            statePerVehicle[v->getID()].insert(std::make_pair(i, new VehicleState(i,currTime)));
    }
    vehicles[v] = address;
    EV << "Registered vehicle " << v->getID() << " in node: " << address << endl;

}


/**
 * Get the last location where the vehicle was registered.
 *
 * @param vehicleID
 * @return the location address
 */
int BaseCoord::getLastVehicleLocation(int vehicleID)
{
    for(auto const& x : vehicles)
    {
        if(x.first->getID() == vehicleID)
            return x.second;
    }
    return -1;
}


/**
 * Get vehicle from its ID.
 *
 * @param vehicleID
 * @return pointer to the vehicle
 */
Vehicle* BaseCoord::getVehicleByID(int vehicleID)
{
    for(auto const& x : vehicles)
    {
        if(x.first->getID() == vehicleID)
            return x.first;
    }
    return nullptr;
}


/**
 * Delete unused dynamically allocated memory.
 *
 * @param spList The list of stop point
 */
void BaseCoord::cleanStopPointList(std::list<StopPoint*> spList)
{
    for(auto &it:spList) delete it;
    spList.clear();
}


/**
 * Check if a Trip Request is valid.
 *
 * @param tr The trip Request.
 * @return true if the request is valid.
 */
bool BaseCoord::isRequestValid(const TripRequest tr)
{
    bool valid = false;

    if(tr.getPickupSP() && tr.getDropoffSP() &&
            netmanager->isValidAddress(tr.getPickupSP()->getLocation()) && netmanager->isValidAddress(tr.getDropoffSP()->getLocation()))
                valid = true;
    return valid;

}

/**
 * Get from all available vehicles, the max number of seats.
 *
 * @return The max seats
 */
int BaseCoord::getMaxVehiclesSeats()
{
    int vSeats = 0;

    for(auto &it:vehicles)
    {
        if(it.first->getSeats() > vSeats)
            vSeats = it.first->getSeats();
    }
    return vSeats;
}

/**
 * Collect Median and 95th percentile related to the provided vector.
 *
 * @param sigName The signal name
 * @param values
 */
void BaseCoord::collectPercentileStats(std::string sigName, std::vector<double> values)
{
    int size=values.size();
    if(size==0)
    {
        recordScalar((sigName+": Median").c_str(), 0.0);
        recordScalar((sigName+": 95Percentile").c_str(), 0.0);
        return;
    }

    if(size == 1)
    {
        recordScalar((sigName+": Median").c_str(), values[0]);
        recordScalar((sigName+": 95Percentile").c_str(), values[0]);
        return;
    }

    std::sort(values.begin(), values.end());
    double median;
    double percentile95;

    if (size % 2 == 0)
          median= (values[size / 2 - 1] + values[size / 2]) / 2;
    else
        median = values[size / 2];

    recordScalar((sigName+": Median").c_str(), median);


    /*95th percentile Evaluation*/
    double index = 0.95*size;
    double intpart;
    double decpart;

    decpart = modf(index, &intpart);
    if(decpart == 0.0)
        percentile95 = (values[intpart-1]+values[intpart]) / 2;

    else{
        if( decpart > 0.4)
            index=intpart;
        else
            index=intpart-1;
        percentile95 = values[index];
    }

    recordScalar((sigName+": 95Percentile").c_str(), percentile95);

}

/**
 * Evaluate if a stop point is "feasible" by a vehicle.
 *
 * @param vehicleID The vehicleID
 * @param sp The stop-point to evaluate
 *
 * @return true if feasible
 */
bool BaseCoord::eval_feasibility (int vehicleID, StopPoint* sp)
{
    //TODO return the beforeR and afterR

    std::list<StopPoint*> lsp = rPerVehicle[vehicleID];
    std::list<StopPoint*> afterR;
    std::list<StopPoint*> beforeR;
    double currentTime = simTime().dbl();
    bool isFeasible = true;

    for (std::list<StopPoint*>::const_iterator it = lsp.begin(), end = lsp.end(); it != end; ++it) {
        EV <<"Distance from " << sp->getLocation() << " to " << (*it)->getLocation() << " is > " <<  ((*it)->getTime() + (*it)->getMaxDelay() - currentTime) << endl;
        if (netmanager->getTimeDistance(sp->getLocation(), (*it)->getLocation()) > ((*it)->getTime() + (*it)->getMaxDelay() - currentTime))
            beforeR.push_back((*it));
    }

    for (std::list<StopPoint*>::const_iterator it = lsp.begin(), end = lsp.end(); it != end; ++it) {
        EV <<"Distance from " << (*it)->getLocation() << " to " << sp->getLocation() << " is > " <<  (sp->getTime() + sp->getMaxDelay() - currentTime) << endl;
        if (netmanager->getTimeDistance((*it)->getLocation(), sp->getLocation()) > (sp->getTime() + sp->getMaxDelay() - currentTime))
            afterR.push_back((*it));
    }

    if(beforeR.empty() || afterR.empty())
        isFeasible = true;

    else
    {
        for (std::list<StopPoint*>::const_iterator it = beforeR.begin(), end = beforeR.end(); it != end; ++it)
        {
            for(std::list<StopPoint*>::const_iterator it2 = afterR.begin(), end = afterR.end(); it2 != end; ++it2)
            {
                if((*it)->getLocation() == (*it2)->getLocation())
                {
                    EV << "The same node is in before and after list! Node is: " << (*it)->getLocation() << endl;
                    isFeasible = false;
                    break;
                }

                if (netmanager->getTimeDistance((*it)->getLocation(), (*it2)->getLocation()) > ((*it2)->getTime() + (*it2)->getMaxDelay() - currentTime))
                {
                    EV << "The request is not feasible for the vehicle " << vehicleID << endl;
                    isFeasible = false;
                    break;
                }
            }
            if(!isFeasible)
                break;
            EV << "The request could be feasible for the vehicle " << vehicleID << endl;
        }
    }

    return isFeasible;
}



void BaseCoord::updateLinkWeight(cTopology::LinkOut* path, int pkChosenGate) {
	/*cTopology::LinkOut *path = node->getPath(0);
	ev << "We are in " << node->getModule()->getFullPath() << endl;
	EV << "Taking gate " << path->getLocalGate()->getFullName() << " with weight " <<path->getWeight()<< " we arrive in " << path->getRemoteNode()->getModule()->getFullPath() << " on its gate " << path->getRemoteGate()->getFullName() << endl;
	pk->setChosenGate(path->getLocalGate()->getIndex());

	traffic->increaseTraffic(pk->getChosenGate(),pk->getTrafficWeight());

	int pkChosenGate = pk->getChosenGate();
	path->setWeight(traffic->getTraffic(pkChosenGate));
	*/
}

void BaseCoord::emitDifferenceFromRequestToPickup(double diff, bool redCode) {
	if (redCode)
		emit(differenceFromRedCodeRequestToPickup, diff);

	else
		emit(differenceFromRequestToPickup, diff);
}


void BaseCoord::evacuateCivil(int address) {
	emit(signal_civilEvacuated, ++civilCounter);
}

void BaseCoord::emitEmergencyRequest() {
	emit(emergencyRequest,++emergencyRequestCounter);
}
void BaseCoord::emitTruckRequest() {
	emit(truckRequest,++truckRequestCounter);
}


void BaseCoord::emitRedCodeEmergencyRequest() {
	emit(redCodeRequest, ++redCodeRequestCounter);
}

void BaseCoord::emitPickupEmergencies() {
	pickupEmergenciesCount++;
	emit(pickupEmergencies, pickupEmergenciesCount);
}


bool BaseCoord::checkPendingRedStopPoints() {
	return !pendingRedStopPoints.empty();
}

bool BaseCoord::checkPendingStopPoints() {
	return !pendingStopPoints.empty();
}




void BaseCoord::pickPendingRedStopPoints(int vehicleID, int srcAddr) {

	for (auto req : pendingRedStopPoints){
	checkRemainingTime(req);
	}

	StopPoint *sp = new StopPoint(*pendingRedStopPoints.front());
	pendingRedStopPoints.pop_front();

	sp->setRedCode(true);
	std::list<StopPoint*> spList; // inserimento permutazione col minimo costo in lista spList
	spList.push_back(sp);

	int destAddress;
	if (sp->isNeedSkilledHospital()) {	//requires skilled hospital
			destAddress = netmanager->pickSkilledHospitalFromNode(sp->getLocation());
		} else{
			// Generate emergency request to the closest hospital
			destAddress = netmanager->pickClosestHospitalFromNode(sp->getLocation());
		}

	//aggiunta ospedale alla splist
	StopPoint *hospital = new StopPoint(-1,destAddress , false, simTime().dbl(), 0);
	hospital->setRedCode(true);

	spList.push_back(hospital);

	EV << "NEW LIST:";
	for (auto elem: spList)
		EV << elem->getLocation() << " - ";
	EV <<  " vehicle: " << vehicleID << endl;


	updateVehicleStopPoints(vehicleID,spList,hospital);

}

void BaseCoord::pickPendingStopPoints(int vehicleID, int seats, int srcAddr) {

	std::vector<StopPoint*> spVector, spVectorAux;

	// prende i primi seats elementi della lista pending
	for (int i = 0;i<seats; i++){
		if (!pendingStopPoints.empty()) {
			spVector.push_back(new StopPoint(*pendingStopPoints.front()));
			pendingStopPoints.pop_front();
		}
	}

	int cost =0;
	int min = -1;
	UINT i;
	// sort del vettore
	std::sort(spVector.begin(), spVector.end());


	//Calcola il minimo costo in hop per ogni permutazione tra le permutazioni degli stop point considerati
	do {
		cost = netmanager->getHopDistance(srcAddr, spVector[0]->getLocation());
		EV << spVector[0]->getLocation() <<  "  cost: " << cost << " | ";
		for ( i = 0; i < spVector.size()-1; i++){
			cost+=netmanager->getHopDistance(spVector[i]->getLocation(), spVector[i+1]->getLocation() );
			EV << spVector[i+1]->getLocation() << "  cost: " << cost << " | ";
		}
		cost+=netmanager->getHopDistance(spVector[i]->getLocation(),netmanager->pickClosestHospitalFromNode(spVector[i]->getLocation()));
		EV  << "H: " <<netmanager->pickClosestHospitalFromNode(spVector[i]->getLocation()) << "  COST: " << cost << endl;

		if (min == -1){
			min = cost;
			spVectorAux = spVector;
		}
		else if (cost < min){
			min = cost;
			spVectorAux = spVector;
		}

	} while (std::next_permutation(spVector.begin(), spVector.end()));


	std::list<StopPoint*> spList; // inserimento permutazione col minimo costo in lista spList
	for (i = 0; i < spVectorAux.size(); i++)
		spList.push_back(spVectorAux[i]);

	//aggiunta ospedale alla splist
	StopPoint *hospital = new StopPoint(-1, netmanager->pickClosestHospitalFromNode(spVectorAux[i-1]->getLocation()), false, simTime().dbl(), 0);
	spList.push_back(hospital);

	EV << "NEW LIST:";
	for (auto elem: spList)
		EV << elem->getLocation() << " - ";
	EV <<  " vehicle: " << vehicleID << endl;


	updateVehicleStopPoints(vehicleID,spList,hospital);

}

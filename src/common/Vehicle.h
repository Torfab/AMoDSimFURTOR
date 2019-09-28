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

#ifndef VEHICLE_H_
#define VEHICLE_H_

#include <Packet_m.h>

class Vehicle: public Packet {
    protected:
        static int nextID;
        int id;
        int seats;
        int traveledDistance;
        int specialVehicle; //	civil:-1		emergency vehicle:1 	truck:2
        int weight;  // influence of single vehicle in traffic channel. It increases the travel times of other vehicles.

        int chosenGate; // Choose the gate for leaving the node

        double speed;  // Avg. speed of vehicle in channel
        double acceleration; // Avg. acceleration of vehicle
        
        double currentTraveledTime;
        double optimalEstimatedTravelTime;

    public:
        Vehicle();
        Vehicle(int specialVehicle, double speed, int trafficWeight);
        virtual ~Vehicle();
        virtual int getID() const;
        virtual double getTraveledDistance() const;
        virtual void setTraveledDistance(double distance);
        virtual int getSeats() const;
        virtual void setSeats(int seats);

        virtual int getSpecialVehicle() const;
//        virtual void setSpecialVehicle(int specialVehicle);
        virtual int getChosenGate();
        virtual void setChosenGate(int gate);
        virtual double getSpeed() const;
        virtual void setSpeed(double speed);
        virtual int getWeight() const;
        virtual double getCurrentTraveledTime() const;
        virtual void setCurrentTraveledTime(double currentTraveledTime);
        virtual double getOptimalEstimatedTravelTime() const;
        virtual void setOptimalEstimatedTravelTime(double optimalEstimatedTravelTime);
        virtual double getAcceleration() const;
        void setSpecialVehicle(int specialVehicle);
};

#endif /* VEHICLE_H_ */

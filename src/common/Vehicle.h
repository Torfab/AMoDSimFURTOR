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
        int state;
        int specialVehicle; //veicolo di emergenza -1 0 civile 1 ambulanza (TODO: enum)

        int chosenGate; // Choose the gate for leaving the node

    public:
        Vehicle();
        virtual ~Vehicle();
        virtual int getID() const;
        virtual double getTraveledDistance() const;
        virtual void setTraveledDistance(double distance);
        virtual int getSeats() const;
        virtual void setSeats(int seats);
        virtual void setState(int state);
        virtual int getState() const;
        virtual int getSpecialVehicle() const;
        virtual void setSpecialVehicle(int specialVehicle);
        virtual int getChosenGate();
        virtual void setChosenGate(int gate);

};

#endif /* VEHICLE_H_ */

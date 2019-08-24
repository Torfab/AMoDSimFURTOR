//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef TRAFFIC_H_
#define TRAFFIC_H_

class Traffic {

protected:
	int numberOfGates;
	double *traffic; // N E S W
	double DecayTime;
	double DecayFactor;

public:
	Traffic(double DecayTime, double DecayFactor);
	virtual ~Traffic();

	double getDecayFactor() const;
	double getDecayTime() const;

	void increaseTraffic(int i);
	const double getTraffic(int i) const;
	int getNumberOfGates() const;

	void decay();
};

#endif /* TRAFFIC_H_ */

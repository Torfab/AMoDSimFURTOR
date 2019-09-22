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

#ifndef PHEROMONE_H_
#define PHEROMONE_H_

class Pheromone {
private:
	int numberOfGates; //It's the number of ports, 4 is default:  N E S W
	double *pheromone; //Array of pheromone
	double pheromoneDecayTime;
	double pheromoneDecayFactor;
public:
	Pheromone(double DecayTime, double DecayFactor);
	virtual ~Pheromone();

	double getPheromoneDecayFactor() const;
	double getPheromoneDecayTime() const;
	void increasePheromone(int i, int weight);
	const double getPheromone(int i) const;
	int getNumberOfGates() const;

	void decayPheromone();
	void setPheromone(int gate, int pheromone);
};

#endif /* PHEROMONE_H_ */

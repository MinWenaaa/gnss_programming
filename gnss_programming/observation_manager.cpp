#include "pch.h"

#include <fstream>
#include <sstream>
#include <cstring>

#include "observation_manager.h"
#include "navigation_manager.h"


bool ObservationManager::readFile(const char* file_path) {
	std::ifstream obsFile(file_path);
	if (!obsFile.is_open()) return false;
	
	std::string line;
	// ÎÄ¼þÍ·
	obsFile >> version >> line;
	if (line != "OBSERVATION") return false;
	
	std::getline(obsFile, line);
	std::getline(obsFile, line);
	std::getline(obsFile, line);

	obsFile >> approch_x >> approch_y >> approach_z;
	std::getline(obsFile, line);
	std::getline(obsFile, line);
	std::getline(obsFile, line);

	obsFile >> sec;
	std::getline(obsFile, line);
	std::getline(obsFile, line);
	std::getline(obsFile, line);
	std::getline(obsFile, line);
	obsFile >> ant_dh >> ant_de >> ant_dn;

	std::getline(obsFile, line);
	std::getline(obsFile, line);
	std::getline(obsFile, line);
	std::getline(obsFile, line);
	std::getline(obsFile, line);	// # / TYPES OF OBSERV

	int satNum;
	obsFile >> satNum;
	satNames.resize(satNum);
	std::getline(obsFile, line);

	char satName[4] = { 0 };
	for (int i = 0; i < satNum; ++i) {
		obsFile.read(satName, 3); 
		obsFile.read(satName, 3);
		satNames[i] = std::string(satName);
		std::getline(obsFile, line);
	}

	double interval, s1, s2;
	int y1, y2, m1, m2, d1, d2, h1, h2, min1, min2;
	obsFile >> interval;
	std::getline(obsFile, line);
	obsFile >> y1 >> m1 >> d1 >> h1 >> min1 >> s1;
	std::getline(obsFile, line);
	obsFile >> y2 >> m2 >> d2 >> h2 >> min2 >> s2;
	std::getline(obsFile, line);

	double dSec = 0;
	if (s2 < s1) { s2 += 60; min2--; }
	dSec += (s2 - s1);
	if (min2 < min1) { min2 += 60; h2--; }
	dSec += (min2 - min1) * 60;
	if (h2 < h1) { h2 += 24; d1--; }
	dSec += (h2 - h1) * 3600;
	if (d2 < d1) { d1 += 1; m1--; }

	int epochNum = dSec / interval;
	epoches.resize(epochNum);
	std::getline(obsFile, line);

	Epoch* temp = nullptr;
	for (int i = 0; i < epochNum; i++) {
		epoches[i] = new Epoch();
		temp = epoches[i];
		obsFile >> temp->y >> temp->m >> temp->d >> temp->h >> temp->min >> temp->sec >> temp->flag >> temp->obsNum;
		temp->obses.resize(temp->obsNum);

		int count = 0;
		char ch;
		for (int j = 0; j < temp->obsNum; j++) {
			temp->obses[j] = new singleObs();
			while (count < 3 && obsFile.get(ch)) {
				if (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t') continue;
				temp->obses[j]->satName[count++] = ch;
			}
			count = 0;
		}

		char buf[16] = { 0 };
		char* endptr = nullptr;
		double val;
		for (int j = 0; j < temp->obsNum; j++) {
			obsFile.read(buf, 2);
			int k = 0;
			for (; k < 5; k++) {
				obsFile.read(buf, 16); 
				val = std::strtod(buf, &endptr);
				temp->obses[j]->obs[k] = (endptr == buf) ? -1 : val;
			}
			for (; k < 8; k++) {
				obsFile >> temp->obses[j]->obs[k];
			}
		}

	}
}
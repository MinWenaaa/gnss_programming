#pragma once
#include <string>
#include <vector>

enum Band { C1, C2, L1, L2, P1, P2, S1, S2};

struct singleObs {
	char satName[3];
	double obs[8];
};

class Epoch {
public:
	Epoch() = default;
	int y, m, d, h, min, flag, obsNum;
	double sec;
	std::vector<singleObs*> obses;

private:
};

class ObservationManager {
public:
	static ObservationManager& instance() {
		static ObservationManager instance;
		return instance;
	}
	
	bool readFile(const char* file_path);

	std::string version;
	double approch_x, approch_y, approch_z, ant_dh, ant_de, ant_dn;
	int sec;
	std::vector<std::string> satNames;
	std::vector<Epoch*> epoches;

private:
	ObservationManager() = default;
	ObservationManager(const ObservationManager&) = delete;
	ObservationManager& operator=(const ObservationManager&) = delete;

};
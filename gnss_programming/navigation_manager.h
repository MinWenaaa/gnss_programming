#pragma once
#include <string>
#include <fstream>
#include <vector>

const size_t data_of_satellite = 26;

class satelliteData {
public:
	int prn, year, mon, day, hour, minu;
	double sec;
	double bias, drift, drift_rate;
	double orbits[data_of_satellite];

	satelliteData() = delete;
	satelliteData(std::ifstream& file);

};

class navigationManager {
public:
	static navigationManager& instance() {
		static navigationManager instance;
		return instance;
	}

	bool readFile(const char* file_path);

	std::string version;
	double ion_alpha[4], ion_beta[4];
	double a0, a1;
	long int t;
	int w, leapSecond;
	std::vector<satelliteData*> satelilteList;

private:
	navigationManager() = default;
	navigationManager(const navigationManager&) = delete;
	navigationManager& operator=(const navigationManager&) = delete;
};

bool parserFortranDouble(const std::string& s, double& result);


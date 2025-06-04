#pragma once
#include <vector>
#include "observation_manager.h"
#include "navigation_manager.h"

struct satPosition {
	double x, y, z;
};
class solution {
public:
	static solution& instance() {
		static solution instance;
		return instance;
	}

	void cal_all();
	void cal_all_position();
	void set_file_path(std::string& s) {
		file_path = s;
	}

private:
	solution() = default;
	solution(const solution&) = delete;
	solution& operator=(const solution&) = delete;
	std::vector<satPosition> sat_positions;

	void get_position(satelliteData* bestSat, Epoch* epoch, double dSec, double* result);
	void sat_position(int prn, Epoch* epoch, double* result);
	double klobuchar(double x, double y, double z, double UT);
	double saastamoinen(double el, double P = 1013.25, double T = 291.15, double e = 20.0);

	void spp_gps(Epoch* epoch, double* result, Band type);

	std::string file_path;
};
#pragma once
#include <vector>
#include "navigation_manager.h"

struct satPosition {
	double x, y, z;
};
class solution {
public:
	static solution& instance() {
		solution instance;
		return instance;
	}

	void cal_all();

private:
	solution() = default;
	solution(const solution&) = delete;
	solution& operator=(const solution&) = delete;
	std::vector<satPosition> sat_positions;

	void get_position(int prn, int hour, int min, double sec, double* result);
	
};
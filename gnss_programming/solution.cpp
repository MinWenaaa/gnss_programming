#include "pch.h" 
#include <cmath>
#include "solution.h"
#include "spacetime_transform.h"
#include "observation_manager.h"

const double GM = 3.986005e14; // 地球引力常数
const double TOLERANCE = 1e-6; // 误差容限
const double omega_e = 7.2921151467e-5; // 地球自转角速度 (rad/s)
const double c = 299792458.0; // 光速 (m/s)

struct position {
	double coord[3];
};

double solveKepler(double M, double e) {
	double E = M;
	double delta = 1.0;

	while (std::abs(delta) > TOLERANCE) {
		delta = (E - e * std::sin(E) - M) / (1 - e * std::cos(E));
		E -= delta;
	}
	return E;
}

void solution::get_position(int prn, int hour, int min, double sec, double dSec, double* result) {
	navigationManager& navManager = navigationManager::instance();
	timeTransformer& timeManager = timeTransformer::instance();

	// 选择最合适的观测值
	satelliteData* temp = nullptr, *temp2 = nullptr;
	size_t j = 0;
	for (; j < navManager.satelilteList.size(); ++j) {
		temp = navManager.satelilteList[j];
		if (prn != temp->prn) continue;
		if (temp->hour < hour) continue;
		else if (temp->hour == hour && temp->minu < min) continue;
		else if (temp->minu == min && temp->sec < sec) continue;
		else {
			temp2 = temp;
			break;
		}
	}
	if (temp2 && temp->prn == prn) {
		double dTime = temp->hour * 3600 + temp->minu * 60 + temp->sec - (hour * 3600 + min * 60 + sec);
		double dTime2 = hour * 3600 + min * 60 + sec - (temp2->hour * 3600 + temp2->minu * 60 + temp2->sec);
		temp = (dTime > dTime2) ? temp2 : temp;
	}
	else if (temp->prn != prn) {
		temp = temp2;
	}

	// step 1
	double n = sqrt(GM) / (temp->orbits[sqrtA] * temp->orbits[sqrtA] * temp->orbits[sqrtA]) + temp->orbits[Dn];

	// step2
	timeManager.setOrigin(UTM); timeManager.setTarget(GPS);
	timeManager.utm_year = temp->year + 2000; timeManager.utm_mon = temp->mon;
	timeManager.utm_day = temp->day; timeManager.utm_hour = temp->hour;
	timeManager.utm_min = temp->minu; timeManager.utm_sec = temp->sec;
	timeManager.run();
	double t = timeManager.gps_second - dSec;
	//double t = temp->orbits[ToeT];
	double M = temp->orbits[M0] + n * (t - temp->orbits[ToeT]);

	// step3
	double E = solveKepler(M, temp->orbits[Ec]);

	// step4
	double sinE = std::sin(E), cosE = std::cos(E);
	double f = std::atan2(std::sqrt(1 - temp->orbits[Ec] * temp->orbits[Ec]) * sinE, cosE - temp->orbits[Ec]);

	// step5
	double phi = f + temp->orbits[omega];

	// step6
	double delta_u = temp->orbits[Cuc] * std::cos(2 * phi) + temp->orbits[Cus] * std::sin(2 * phi);
	double delta_r = temp->orbits[Crs] * std::cos(2 * phi) + temp->orbits[Crc] * std::sin(2 * phi);
	double delta_i = temp->orbits[Cic] * std::cos(2 * phi) + temp->orbits[Cis] * std::sin(2 * phi);

	// step7
	double u = phi + delta_u;
	double r = temp->orbits[sqrtA] * temp->orbits[sqrtA] * (1 - temp->orbits[Ec] * cosE) + delta_r;
	double i = temp->orbits[i0] + delta_i + temp->orbits[IDOT] * (t - temp->orbits[ToeT]);

	// step8
	double x = r * std::cos(u), y = r * std::sin(u);

	// step9
	double L = temp->orbits[Omega] + (temp->orbits[OmegaDot] - omega_e) * t - temp->orbits[OmegaDot] * temp->orbits[ToeT];

	// step10
	result[0] = x * std::cos(L) - y * std::sin(L) * std::cos(i);
	result[1] = x * std::sin(L) + y * std::cos(L) * std::cos(i);
	result[2] = y * std::sin(i);

	return;
}

void solution::cal_all() {
	std::vector<position> positions;
	positions.resize(300);
	int count = 0;
	ObservationManager& obsManager = ObservationManager::instance();
	for (int i = 0; i < 10; i++) {
		Epoch* temp = obsManager.epoches[i];

		for (const auto& obs : temp->obses) {
			if (obs->satName[0] != 'G') continue;
			int prn = std::atoi(obs->satName + 1); 
			double dSec = 0;
			get_position(prn, temp->h, temp->min, temp->sec, dSec, positions[count].coord);
			double distance = std::sqrt(std::pow(obsManager.approch_x - positions[count].coord[0], 2) +
				std::pow(obsManager.approch_y - positions[count].coord[1], 2) +
				std::pow(obsManager.approch_z - positions[count].coord[2], 2));
			dSec += distance/c;
			get_position(prn, temp->h, temp->min, temp->sec, dSec, positions[count].coord);
			count++;
		}
		
	}
	return;
}
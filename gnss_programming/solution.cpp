#include "pch.h" 
#include <cmath>
#include <fstream>

#include <Eigen/Dense>

#include "solution.h"
#include "spacetime_transform.h"

const double GM = 3.986005e14; // 地球引力常数
const double TOLERANCE = 1e-6; // 误差容限
const double omega_e = 7.2921151467e-5; // 地球自转角速度 (rad/s)
const double c = 299792458.0; // 光速 (m/s)
const double PI = 3.14159265358979323846; // 圆周率
const double phi = 79.93;
const double lambda = 288.04;

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

void solution::get_position(satelliteData* sat, Epoch* epoch, double dSec, double* result) {
	
	// step 1
	double n = sqrt(GM) / (sat->orbits[sqrtA] * sat->orbits[sqrtA] * sat->orbits[sqrtA]) + sat->orbits[Dn];

	// step2
	GPS gpsTime; 
	UTC utcTime(
		epoch->y + 2000, epoch->m, epoch->d, epoch->h, epoch->min, epoch->sec);
	utc2gps(&utcTime, &gpsTime);
	double t = gpsTime.sec - dSec;
	double M = sat->orbits[M0] + n * (t - sat->orbits[ToeT]);

	// step3
	double E = solveKepler(M, sat->orbits[Ec]);

	// step4
	double sinE = std::sin(E), cosE = std::cos(E);
	double f = std::atan2(std::sqrt(1 - sat->orbits[Ec] * sat->orbits[Ec]) * sinE, cosE - sat->orbits[Ec]);

	// step5
	double phi = f + sat->orbits[omega];

	// step6
	double delta_u = sat->orbits[Cuc] * std::cos(2 * phi) + sat->orbits[Cus] * std::sin(2 * phi);
	double delta_r = sat->orbits[Crs] * std::cos(2 * phi) + sat->orbits[Crc] * std::sin(2 * phi);
	double delta_i = sat->orbits[Cic] * std::cos(2 * phi) + sat->orbits[Cis] * std::sin(2 * phi);

	// step7
	double u = phi + delta_u;
	double r = sat->orbits[sqrtA] * sat->orbits[sqrtA] * (1 - sat->orbits[Ec] * cosE) + delta_r;
	double i = sat->orbits[i0] + delta_i + sat->orbits[IDOT] * (t - sat->orbits[ToeT]);

	// step8
	double x = r * std::cos(u), y = r * std::sin(u);

	// step9
	double L = sat->orbits[Omega] + (sat->orbits[OmegaDot] - omega_e) * t - sat->orbits[OmegaDot] * sat->orbits[ToeT];

	// step10
	result[0] = x * std::cos(L) - y * std::sin(L) * std::cos(i);
	result[1] = x * std::sin(L) + y * std::cos(L) * std::cos(i);
	result[2] = y * std::sin(i);

	return;
}

void solution::sat_position(int prn, Epoch* epoch, double* result) {
	ObservationManager& obsManager = ObservationManager::instance();
	navigationManager& navManager = navigationManager::instance();
	int hour = epoch->h, min = epoch->min;
	double sec = epoch->sec;
	// 选择最合适的观测值
	satelliteData* temp = nullptr, * temp2 = nullptr;
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

	double dSec = 0;
	get_position(temp, epoch, dSec, result);
	double distance = std::sqrt(std::pow(obsManager.approch_x - result[0], 2) +
		std::pow(obsManager.approch_y - result[1], 2) +
		std::pow(obsManager.approch_z - result[2], 2));
	dSec += distance / c;
	get_position(temp, epoch, dSec, result);
}

double solution::klobuchar(double x, double y, double z, double UT) {
	// step1
	ObservationManager& obsManager = ObservationManager::instance();
	double dx = x - obsManager.approch_x, dy = y - obsManager.approch_y, dz = z - obsManager.approch_z;
	double b, l, h;
	spaceTransformer::xyz2blh(x, y, z, b, l, h);

	double sin_lat = sin(b), cos_lat = cos(l);
	double sin_lon = sin(l), cos_lon = cos(b);
	double e = -sin_lon * dx + cos_lon * dy;
	double n = -sin_lat * cos_lon * dx - sin_lat * sin_lon * dy + cos_lat * dz;
	double u = cos_lat * cos_lon * dx + cos_lat * sin_lon * dy + sin_lat * dz;

	double el = std::asin(u / sqrt(e * e + n * n + u * u));
	double alpha = atan2(e, n);

	double EA = (445 / (el / PI * 180) + 20) - 4;


	// step2
	spaceTransformer::xyz2blh(obsManager.approch_x, obsManager.approch_y, obsManager.approch_z, b, l, h);
	double phi_P = b * PI / 180.0 + EA * std::cos(alpha);
	double lambda_P = l * PI / 180.0 + EA * std::sin(alpha);

	// step3
	double t = UT + lambda_P / PI * 180 / 15;

	// step4
	double phi_m = phi_P + (90 - phi) * std::cos(lambda_P - lambda) * PI / 180.0;

	navigationManager& navManager = navigationManager::instance();
	double Tg = 5e-9 +(navManager.ion_alpha[0] + navManager.ion_alpha[1] * std::cos(phi_m) + navManager.ion_alpha[2] * std::sin(phi_m) +
		navManager.ion_alpha[3] * std::cos(2 * phi_m)) * std::pow(10, -6);

	return Tg * (1 + 2 * std::pow((96 - el / PI * 180) / 90, 2));
}

double solution::saastamoinen(double el, double P, double T, double e) {
	ObservationManager& obsManager = ObservationManager::instance();
	double b, l, h;
	spaceTransformer::xyz2blh(obsManager.approch_x, obsManager.approch_y, obsManager.approch_z, b, l, h);


	if (P == 1013.25) {
		P = 1013.25 * pow(1 - 0.0000226 * h, 5.225);
	}

	if (T == 291.15) {
		T = 291.15 - 0.0065 * h;
	}

	if (e == 20.0) {
		e = 6.108 * exp((17.15 * (T - 273.15)) / (234.7 + (T - 273.15)));
	}

	double zenith = PI / 2.0 - el; // 天顶距
	double tropo = 0.002277 * P / cos(zenith)
		+ 0.002277 * (1255.0 / T + 0.05) * e / cos(zenith);

	return tropo; // 单位：米
}

void solution::spp_gps(Epoch* epoch, double* result, Band type) {
	int satNum = 0;
	for (const auto& obs : epoch->obses) {
		if (obs->satName[0] != 'G') continue; // 只处理GPS卫星
		if (obs->obs[type] == 0) continue; // 只处理有观测值的卫星
		satNum++;
	}

	Eigen::MatrixXd satPositions(satNum, 3);
	int idx = 0;
	for (const auto& obs : epoch->obses) {
		if (obs->satName[0] != 'G') continue;
		if (obs->obs[type] == 0) continue; // 只处理有观测值的卫星
		int prn = std::atoi(obs->satName + 1); // 提取PRN号
		double pos[3] = { 0 };
		sat_position(prn, epoch, pos);
		satPositions(idx, 0) = pos[0];
		satPositions(idx, 1) = pos[1];
		satPositions(idx, 2) = pos[2];
		idx++;
	}

	Eigen::MatrixXd B(satNum, 4);
	Eigen::MatrixXd L(satNum, 1);
	Eigen::MatrixXd P(satNum, satNum);
	Eigen::MatrixXd X(4, 1);

	ObservationManager& obsManager = ObservationManager::instance();

	X(0, 0) = obsManager.approch_x; X(1, 0) = obsManager.approch_y; X(2, 0) = obsManager.approch_z;

	for (int i = 0; i < satNum; i++) {
		if (epoch->obses[i]->satName[0] != 'G') continue;
		if (epoch->obses[i]->obs[type] == 0) continue; // 只处理有观测值的卫星

		double rho_0 = std::sqrt(std::pow(satPositions(i, 0)-obsManager.approch_x, 2) +
			std::pow(satPositions(i, 1) - obsManager.approch_y, 2) +
			std::pow(satPositions(i, 2) - obsManager.approch_z, 2));

		B(i, 0) = -(satPositions(i, 0) - obsManager.approch_x) / rho_0;
		B(i, 1) = -(satPositions(i, 1) - obsManager.approch_y) / rho_0;
		B(i, 2) = -(satPositions(i, 2) - obsManager.approch_z) / rho_0;
		B(i, 3) = 1;

		double dx = satPositions(i, 0) - obsManager.approch_x, 
			dy = satPositions(i, 1) - obsManager.approch_y, 
			dz = satPositions(i, 2) - obsManager.approch_z;
		double b, l, h;
		spaceTransformer::xyz2blh(satPositions(i, 0), satPositions(i, 1), satPositions(i, 2), b, l, h);

		double sin_lat = sin(b), cos_lat = cos(l);
		double sin_lon = sin(l), cos_lon = cos(b);
		double e = -sin_lon * dx + cos_lon * dy;
		double n = -sin_lat * cos_lon * dx - sin_lat * sin_lon * dy + cos_lat * dz;
		double u = cos_lat * cos_lon * dx + cos_lat * sin_lon * dy + sin_lat * dz;

		double el = std::asin(u / sqrt(e * e + n * n + u * u));

		L(i, 0) = epoch->obses[i]->obs[type] - rho_0
			- klobuchar(satPositions(i, 0), satPositions(i, 1), satPositions(i, 2), epoch->h + epoch->min / 60.0)
			- saastamoinen(el);


		P(i, i) = std::pow(std::sin(el), 2);
	}

	Eigen::MatrixXd BT = B.transpose();
	Eigen::MatrixXd PB = P * B;
	Eigen::MatrixXd N = BT * PB;
	Eigen::MatrixXd W = BT * P * L;
	Eigen::MatrixXd dX = N.inverse() * W;

	X = X + dX;

	result[0] = X(0, 0);
	result[1] = X(1, 0);
	result[2] = X(2, 0);
	result[3] = X(3, 0); // 钟差

}

void solution::cal_all() {
	std::fstream outFile(file_path, 'w');

	outFile << "x, y, z\n";
	ObservationManager& obsManager = ObservationManager::instance();
	for (const auto& epoch : obsManager.epoches) {
		double result[4] = { 0 };
		spp_gps(epoch, result, C1); // 使用C1观测值进行单点定位
		outFile << result[0] << "," << result[1] << ',' << result[2] << '\n';
	}
}

void solution::cal_all_position() {
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
			//get_position(prn, temp, dSec, positions[count].coord);
			double distance = std::sqrt(std::pow(obsManager.approch_x - positions[count].coord[0], 2) +
				std::pow(obsManager.approch_y - positions[count].coord[1], 2) +
				std::pow(obsManager.approch_z - positions[count].coord[2], 2));
			dSec += distance / c;
			//get_position(prn, temp, dSec, positions[count].coord);
			count++;
		}
	}
	return;
}
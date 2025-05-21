#include "pch.h"

#include <iostream>
#include <string>
#include <stdexcept>

#include "navigation_manager.h"

const std::size_t N1 = 22, N2 = 19;
bool parserFortranDouble(const std::string& s, double& result) {
	if (s.find('D') != std::string::npos) {
		std::string s1 = s;
		s1.replace(s1.find('D'), 1, "E");
		result = std::stod(s1);
		return true;
	}
	result = -1;
	return false;

}

bool navigationManager::readFile(const char* file_path) {
	std::ifstream navigationFile(file_path);
	if (!navigationFile.is_open()) {
		std::cerr << "Error opening file: " << file_path << std::endl;
		return false;
	}

	// 文件头读取
	std::string temp1, temp2;
	navigationFile >> version;
	navigationFile >> temp1 >> temp2;
	if (temp1 != "NAVIGATION" || temp2!="DATA") {
		std::cerr << "Error: Expected 'NAVIGATION DATA' but got '" << temp1 << " " << temp2 << "'" << std::endl;
		return false;
	}
	
	std::getline(navigationFile, temp1);
	std::getline(navigationFile, temp1);
	std::getline(navigationFile, temp1);

	// 电离层参数
	bool flag = false;
	for (int i = 0; i < 4; i++) {
		navigationFile >> temp1;
		if (!parserFortranDouble(temp1, ion_alpha[i])) {
			std::cerr << "Error parsing double value from string: " << temp1 << std::endl;
			return false;
		}
	}
	std::getline(navigationFile, temp1);
	for (int i = 0; i < 4; i++) {
		navigationFile >> temp1;
		if (!parserFortranDouble(temp1, ion_beta[i])) {
			std::cerr << "Error parsing double value from string: " << temp1 << std::endl;
			return false;
		}
	}
	std::getline(navigationFile, temp1);

	char buffer1[N1 + 1] = { 0 }, buffer2[N2 + 1] = { 0 };
	navigationFile.read(buffer1, N1);
	if (!parserFortranDouble(buffer1, a0)) {
		std::cerr << "Error parsing double value from string: " << buffer1 << std::endl;
		return false;
	}
	navigationFile.read(buffer2, N2);
	if (!parserFortranDouble(buffer2, a1)) {
		std::cerr << "Error parsing double value from string: " << buffer2 << std::endl;
		return false;
	}
	navigationFile >> t >> w;
	std::getline(navigationFile, temp1);
	navigationFile >> leapSecond;
	std::getline(navigationFile, temp1);
	std::getline(navigationFile, temp1);

	// 读取卫星数据
	satelliteData* temp = nullptr;
	char nextChar;
	while (navigationFile.peek()!=-1) {
		nextChar = navigationFile.peek();
		try{
			temp = new satelliteData(navigationFile);
			satelilteList.push_back(temp);
		} catch (const std::exception& ex) {
			return false;
		}
	}

	std::cout << "read finished" << std::endl;
}

satelliteData::satelliteData(std::ifstream& file) {
	std::string temp;

	file >> prn >> year >> mon >> day >> hour >> minu >> sec;

	bool flag = false;
	char buffer1[N1 + 1] = { 0 }, buffer2[N2 + 1] = { 0 };
	file.read(buffer2, N2);
	if (!parserFortranDouble(buffer2, bias)) throw std::invalid_argument("parser bias eoor");
	file.read(buffer2, N2);
	if (!parserFortranDouble(buffer2, drift)) throw std::invalid_argument("parser bias error");
	file.read(buffer2, N2);
	if (!parserFortranDouble(buffer2, drift_rate)) throw std::invalid_argument("parser bias error");

	for (int i = 0; i < data_of_satellite; i++) {
		if (i % 4 == 0) {
			std::getline(file, temp);
			file.read(buffer1, N1);
			if (!parserFortranDouble(buffer1, orbits[i])) throw std::invalid_argument("parser fortran error");
		}
		else {
			file.read(buffer2, N2);
			if (!parserFortranDouble(buffer2, orbits[i])) throw std::invalid_argument("parser fortran error");
		}
	}

	std::getline(file, temp);
}
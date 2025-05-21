#pragma once
#include <string>

class timeTransformer {
public:
	enum TimeType { DOY, GPS, JD, UTM  };

	static timeTransformer& instance() {
		static timeTransformer instance;
		return instance;
	}

	void setOrigin(TimeType o) {
		origin = o;
	}
	void setTarget(TimeType t) {
		target = t;
	}

	void run();

	std::string getTarget();
	void parserInput(std::string);

private:
	timeTransformer() = default;
	// ½ûÖ¹¿½±´ºÍ¸³Öµ
	timeTransformer(const timeTransformer&) = delete;
	timeTransformer& operator=(const timeTransformer&) = delete;

	TimeType origin = UTM, target = UTM;

	double jd = 0;
	int doy_year = 0, doy_day = 0, gps_week = 0, gps_second = 0;
	int utm_year = 0, utm_mon = 0, utm_day = 0, utm_hour = 0, utm_min = 0, utm_sec = 0;

	double UTM2JD();
	double GPS2JD();
	double DOY2JD();
	void JD2UTM();
	void JD2GPS();
	void JD2DOY();
};

struct LeapSecond {
	int year, month, day;
	int total_leap_seconds;
};

static const LeapSecond leap_seconds[] = {
	{ 1972, 6, 30, 10 },
	{ 1972, 12, 31, 11 },
	{ 1973, 12, 31, 12 },
	{ 1974, 12, 31, 13 },
	{ 1975, 12, 31, 14 },
	{ 1976, 12, 31, 15 },
	{ 1977, 12, 31, 16 },
	{ 1978, 12, 31, 17 },
	{ 1979, 12, 31, 18 },
	{ 1981, 6, 30, 19 },
	{ 1982, 6, 30, 20 },
	{ 1983, 6, 30, 21 },
	{ 1985, 12, 31, 22 },
	{ 1987, 12, 31,23 },
	{ 1989, 12, 31, 24 },
	{ 1990, 12, 31, 25 },
	{ 1992, 6, 30, 26 },
	{ 1993, 6, 30, 27 },
	{ 1994, 6, 30, 28 },
	{ 1995, 12, 31, 29 },
	{ 1997, 12, 31, 30 },
	{ 1998, 12, 31, 31 },
	{ 2005, 12, 31,32 },
	{ 2008, 12, 31,33 },
	{2012 ,6 ,30 ,34},
	{2015 ,6 ,30 ,35},
	{2016 ,12 ,31 ,36},
};

static const int leap_seconds_count = sizeof(leap_seconds) / sizeof(leap_seconds[0]);

int getLeapSeconds(int year, int month, int day);


static constexpr double a = 6378137.0;                // ³¤°ëÖá
static constexpr double f = 1.0 / 298.257223563;      // ±âÂÊ
static constexpr double e2 = 2 * f - f * f;

class spaceTransformer {
public:
	enum spaceType { BLH, ENU, XYZ };
	static spaceTransformer& instance() {
		static spaceTransformer instance;
		return instance;
	}
	void setOrigin(spaceType o) {
		origin = o;
	}
	void setTarget(spaceType t) {
		target = t;
	}
	void run();
	void parserInput(const std::string& input);
	std::string getTarget();


private:
	spaceTransformer() = default;
	// ½ûÖ¹¿½±´ºÍ¸³Öµ
	spaceTransformer(const spaceTransformer&) = delete;
	spaceTransformer& operator=(const spaceTransformer&) = delete;

	double x, y, z, b, l, h, e, n, u;
	spaceType origin = BLH, target = BLH;

	void xyz2blh();
	void xyz2enu();
	void blh2xyz();
	void enu2xyz();
};
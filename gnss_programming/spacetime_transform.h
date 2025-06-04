#pragma once
#include <string>

struct UTC {
	int y, m, d, h, minu;
	double sec;

	UTC(int year, int month, int day, int hour, int minute, double second)
		: y(year), m(month), d(day), h(hour), minu(minute), sec(second) {
	}
};

struct GPS {
	int week;
	double sec;
	GPS() : week(0), sec(0) {}
	GPS(int week, double sec) : week(week), sec(sec) {}
};

void utc2gps(UTC* utcTime, GPS* gpsTime);
void gps2utc(GPS* gpsTime, UTC* utcTime);

struct LeapSecond {
	int year, month, day;
	int total_leap_seconds;
};

static const LeapSecond leap_seconds[] = {
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

static constexpr double a = 6378137.0;                // ≥§∞Î÷·
static constexpr double f = 1.0 / 298.257223563;      // ±‚¬ 
static constexpr double e2 = 2 * f - f * f;

namespace spaceTransformer {
	void xyz2blh(double x, double y, double z, double& b, double& l, double& h);
};
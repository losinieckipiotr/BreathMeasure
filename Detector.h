#ifndef DETECOTR_H
#define DETECOTR_H

#include <chrono>
#include <future>
#include <vector>
#include <iostream>
#include <functional>
#include <thread>

#include <boost/lockfree/spsc_queue.hpp>
#include <boost/atomic.hpp>

#include "DigitalFilter.h"

class Detector
{
	enum
	{
		SAMPLE_FERQ = 2000,
		WINDOW_SIZE = 500,
		ADC_ADDRES = 0x48,
		CONTROL_BYTE = 0x02,
		//HIGH_PASS_ORDER = 2,
		PASS_BAND_ORDER = 2,
		LOW_PASS_ORDER = 3,
		TICK = 1000000 / SAMPLE_FERQ,
		DOWNSAMPLE_FACTOR = 4,
		ANALAYZE_FERQ = SAMPLE_FERQ / DOWNSAMPLE_FACTOR,
		ANALAYZE_SIZE = WINDOW_SIZE / DOWNSAMPLE_FACTOR
	};

public:
	Detector(std::function<void(double)> brHandler);
	~Detector();

	void Init();
	void StartSample();
	void StopSample();

private:
	void AnalyseSamples(unsigned char *buf);
	void AnalyseSamples2();

	//ZMIENNE METODY PROBKUJACEJ/////////////////////////////////////////

	int i2cFd;
	bool sampleFlag = true;

	std::future<void> analyseFuture;

	unsigned char readBuf[2];
	unsigned char sampleBuf1[WINDOW_SIZE];
	unsigned char sampleBuf2[WINDOW_SIZE];

	boost::lockfree::spsc_queue<unsigned char, boost::lockfree::capacity<SAMPLE_FERQ>> lockFreeBuffer;

	/////////////////////////////////////////////////////////////////////

	//ZMIENNE METODY ANALIZUJACEJ PROBKI/////////////////////////////////

	//WSPOLCZYNNIKI B DLA FILTRU GORNOPRZEPUSTOWEGO DLAPORBKOWANIA 500 Hz
	//static const double B_H[HIGH_PASS_ORDER + 1];
	//WSPOLCZYNNIKI A DLA FILTRU GORNOPRZEPUSTOWEGO DLAPORBKOWANIA 500 Hz
	//static const double A_H[HIGH_PASS_ORDER + 1];

	//WSPOLCZYNNIKI B DLA FILTRU DOLNOPRZEPUSTOWEGO DLAPORBKOWANIA 500 Hz
	static const double Low500B[LOW_PASS_ORDER + 1];
	//WSPOLCZYNNIKI A DLA FILTRU DOLNOPRZEPUSTOWEGO DLAPORBKOWANIA 500 Hz
	static const double Low500A[LOW_PASS_ORDER + 1];

	//WSPOLCZYNNIKI B DLA FILTRU PASMOWOPRZEPUSTOWEGO DLAPORBKOWANIA 2000 Hz
	static const double Band2000B[PASS_BAND_ORDER*2 + 1];
	//WSPOLCZYNNIKI A DLA FILTRU PASMOWOPRZEPUSTOWEGO DLAPORBKOWANIA 2000 Hz
	static const double Band2000A[PASS_BAND_ORDER*2 + 1];

	//WSPOLCZYNNIKI B DLA FILTRU DOLNOPRZEPUSTOWEGO DLAPORBKOWANIA 2000 Hz
	//static const double Low2000B[LOW_PASS_ORDER + 1];
	//WSPOLCZYNNIKI A DLA FILTRU DOLNOPRZEPUSTOWEGO DLAPORBKOWANIA 2000 Hz
	//static const double Low2000A[LOW_PASS_ORDER + 1];

	//DigitalFilter filterHigh;

	DigitalFilter filterBand;
	DigitalFilter filterLow;

	std::vector<double> wave;
	std::vector<double> env;

	unsigned long peakStartTime = 0;
	unsigned long peakTime = 0;
	unsigned long peakEndTime = 0;

	double peakVal = 0.0;
	double peakW = 0.0;
	double peakH = 0.0;
	bool peakFound = false;
	const double threshold = 5.0;
	const double minW = 0.5;
	const double minH = 10.0;

	unsigned long offset = 0;

	double previousPeakSecond = 0.0;
	double peakSecond;

	double BPM;

	std::function<void(double)> handler;

	/////////////////////////////////////////////////////////////////////
};

#endif // !DETECOTR_H

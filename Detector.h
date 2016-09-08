#ifndef DETECOTR_H
#define DETECOTR_H

#include <vector>
#include <functional>
#include <thread>
#include <future>

#include <boost/lockfree/spsc_queue.hpp>

#include "DigitalFilter.h"

class Detector
{
	enum
	{
		SAMPLE_FERQ = 2000,//nie zmeniac, zalezne od filtru pasmowo przepustowego
		WINDOW_SIZE = 4,//mozna zmieniac, min 4
		ADC_ADDRES = 0x48,//adres przetwornika
		CONTROL_BYTE = 0x02,//kanal przetwornika
		PASS_BAND_ORDER = 2,
		LOW_PASS_ORDER = 3,
		TICK = 1000000 / SAMPLE_FERQ,
		DOWNSAMPLE_FACTOR = 4,//nie zmieniac, zalezne od filtru dolonoprzepustowego
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
	void AnalyseSamples();

	//ZMIENNE METODY PROBKUJACEJ/////////////////////////////////////////

	int i2cFd;
	bool sampleFlag = true;

	unsigned char readBuf[2];

	boost::lockfree::spsc_queue<unsigned char, boost::lockfree::capacity<SAMPLE_FERQ>> lockFreeBuffer;

	/////////////////////////////////////////////////////////////////////

	//ZMIENNE METODY ANALIZUJACEJ PROBKI/////////////////////////////////

	//WSPOLCZYNNIKI B DLA FILTRU DOLNOPRZEPUSTOWEGO DLAPORBKOWANIA 500 Hz
	static const double Low500B[LOW_PASS_ORDER + 1];
	//WSPOLCZYNNIKI A DLA FILTRU DOLNOPRZEPUSTOWEGO DLAPORBKOWANIA 500 Hz
	static const double Low500A[LOW_PASS_ORDER + 1];

	//WSPOLCZYNNIKI B DLA FILTRU PASMOWOPRZEPUSTOWEGO DLAPORBKOWANIA 2000 Hz
	static const double Band2000B[PASS_BAND_ORDER*2 + 1];
	//WSPOLCZYNNIKI A DLA FILTRU PASMOWOPRZEPUSTOWEGO DLAPORBKOWANIA 2000 Hz
	static const double Band2000A[PASS_BAND_ORDER*2 + 1];

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

#ifndef DETECOTR_H
#define DETECOTR_H

#include <vector>
#include <functional>
#include <thread>
#include <future>

#include <boost/lockfree/spsc_queue.hpp>

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
	Detector(std::function<void(double)>& brHandler);
	~Detector();

	void Init();
	void Sample();
	void StopSample();

private:
	void AnalyseSamples();

	int i2cFd;
	bool sampleFlag;
	boost::lockfree::spsc_queue
        <unsigned char,
        boost::lockfree::capacity<SAMPLE_FERQ>> lockFreeBuffer;

	std::function<void(double)> handler;
};

#endif // !DETECOTR_H

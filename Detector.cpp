#include "Detector.h"

#include <iostream>
#include <exception>

#include <cstdlib>
#include <cstring>
#include <cmath>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>

using namespace std;
using namespace chrono;

//WSPOLCZYNNIKI B DLA FILTRU DOLNOPRZEPUSTOWEGO DLAPORBKOWANIA 500 Hz
const double Detector::Low500B[] =
{
	0.00000030403105645560036,
	0.00000091209316936680107,
	0.00000091209316936680107,
	0.00000030403105645560036
};
//WSPOLCZYNNIKI A DLA FILTRU DOLNOPRZEPUSTOWEGO DLAPORBKOWANIA 500 Hz
const double Detector::Low500A[] =
{
	1.0,
	-2.9729825085787462,
	2.9463287571367061,
	-0.97334381630950817,
};

//WSPOLCZYNNIKI B DLA FILTRU PASMOWOPRZEPUSTOWEGO DLAPORBKOWANIA 2000 Hz
const double Detector::Band2000B[] =
{
	0.292893218813452,
	0.0,
	-0.585786437626905,
	0.0,
	0.292893218813452
};

//WSPOLCZYNNIKI a DLA FILTRU PASMOWOPRZEPUSTOWEGO DLAPORBKOWANIA 2000 Hz
const double Detector::Band2000A[] =
{
	1.0,
	0.442463484164948,
	0.0573408579290252,
	0.0759147321729989,
	0.171572875253810,
};

/*
//WSPOLCZYNNIKI B DLA FILTRU DOLNOPRZEPUSTOWEGO DLAPORBKOWANIA 2000 Hz
const double Detector::Low2000B[] =
{
	4.79865533420387e-09,
	1.43959660026116e-08,
	1.43959660026116e-08,
	4.79865533420387e-09,
};
//WSPOLCZYNNIKI A DLA FILTRU DOLNOPRZEPUSTOWEGO DLAPORBKOWANIA 2000 Hz
const double Detector::Low2000A[] =
{
	1.0,
	-2.99324557900466,
	2.98651394985148,
	-0.993268332457576
};
*/

Detector::Detector(std::function<void(double)> brHandler)
	: //filterHigh(2, B_H, A_H),
	  filterBand(PASS_BAND_ORDER*2, Band2000B, Band2000A),
	  filterLow(LOW_PASS_ORDER, Low500B, Low500A),
	  wave(WINDOW_SIZE),
	  env(ANALAYZE_SIZE),
	  handler(brHandler)
{
    //pusta operacja aby obiekt byl wazny
	analyseFuture = async([](){ ; });
}

Detector::~Detector()
{

}

void Detector::Init()
{
    //inicjalizacja wiringPi wymaga uprawnien root'a
	if (wiringPiSetup() == -1)
		throw new runtime_error("Failed wiringPiSetup!");
    //inicjalizacja I2C
	i2cFd = wiringPiI2CSetup(ADC_ADDRES);
	if (i2cFd == -1)
		throw new runtime_error("Failed wiringPiI2CSetup!");
    //konfiguracja przetowrnika ADC
	wiringPiI2CWrite(i2cFd, CONTROL_BYTE);
}

void Detector::StartSample()
{
    //odczytanie i pominiecie starej probki
    read(i2cFd, readBuf, 2);

    size_t i = 0;
	bool bufFalg = false;
	unsigned char *bufPtr = sampleBuf1;
    unsigned int now;

    //ustawienie maksymalnego priorytetu watku
    //w celu minimalizacji jittera
    piHiPri(99);

   unsigned int nexTime = micros() + TICK;

    while(sampleFlag)
    {
        //odczytanie nowej probki////////////////////////////////////////
        //UWAGA: czytanie 2 probek ktore sa takie same///////////////////
        //jezeli czytalibysmy 1 to otrzymamy zawsze to samo stara probke
        read(i2cFd, readBuf, 2);
		bufPtr[i] = readBuf[0];
		/////////////////////////////////////////////////////////////////
		++i;
		//jezeli bufor napelniony przeslij do drugiego watka do analizy
		if (i == WINDOW_SIZE)
		{
            //wyzerowanie licznika
			i = 0;
			//synchronizacja (czeka az poprzednie probki zostaly przeanalizowane)
			analyseFuture.wait();
			//uruchomienie analizy aktualnego bufora
			analyseFuture = async(launch::async, [this, bufPtr]{ AnalyseSamples(bufPtr); });
			//zmiana bufora aby nie kopiowac danych w tym watku
			bufFalg = !bufFalg;
			if (bufFalg)
				bufPtr = sampleBuf2;
			else
				bufPtr = sampleBuf1;
		}
		//pobranie aktualnego czasu
		now = micros();
		//obliczenie nastepnego czasu
		nexTime += TICK;
		//opoznienie jezeli zdazylo sie wszystko wykonac
		//jezeli wystapi opoznienie zobaczymy komunikat
		if((nexTime > now))
            delayMicroseconds(nexTime - now);
        //else
            //cout << "To slow!\t" << (now - nexTime) << endl;
    }
    //synchronizacja przy wyjsciu
    analyseFuture.wait();
}

void Detector::StopSample()
{
    sampleFlag = false;
}

void Detector::AnalyseSamples(unsigned char *buf)
{
    //kopiowanie danych konieczne aby wykonac
    //konwersje na double
	for (size_t i = 0; i < WINDOW_SIZE; ++i)
	{
		wave[i] = static_cast<double>(buf[i]);
		//skalowanie
		wave[i] -= 128;
		//wave[i] /= 127;
	}

	//filtr gornoprzepustowy fc=5Hz - usuwa skladowa stala
	//filterHigh.Filter(wave.data(), wave.size());

	//filtr pasmowo przepustowy 300-800Hz - filtrue np. sygnal mowy
	filterBand.Filter(wave.data(), wave.size());

	//downsampling i obliczenie wartosci bezwzglednej
	//oblicznie wartosci bezwzglednej dla kazdej probki
	int j = 0;
	for(size_t i = 0; i < ANALAYZE_SIZE; ++i)
	{
		env[i] = abs(wave[j]);
		j += DOWNSAMPLE_FACTOR;
	}

	//filtr dolnoprzepustowy fc=1,075Hz - wykrywanie obwiedni
	filterLow.Filter(env.data(), ANALAYZE_SIZE);

	//wykrywanie wydechow
	for (size_t i = 0; i < ANALAYZE_SIZE; ++i)
	{
        //jezeli wczesniej nie znaleziono piku
		if (peakFound == false)
		{
            //czy wartosc probki jest wieksza od progu
			if (env[i] >= threshold)
			{
				//zapisanie czasu wystapienia probki
				peakTime = peakStartTime = i + offset;
				//zapisanie wartosci probki
				peakVal = env[i];
				//ustawienie flagi oznacza poczatek piku
				peakFound = true;
			}
		}
		//jezeli znaleziono pik
		else
		{
            //jezeli probka jest wieksza od popzedniej
			if (env[i] > peakVal)
			{
                //zapisz jej wartosc (szukanie maksimum)
				peakVal = env[i];
				//zapisz czas
				peakTime = i + offset;
			}
			//jezeli probka jest mniejsza od progu, oznacza to ze pik sie skonczyl
			else if (env[i] < threshold)
			{
				//wylacznie flagi
				peakFound = false;
				//zapisanie czasu ostatniej probki w piku
				peakEndTime = i + offset;
				//obliczenie szerokosci piku
				peakW = (peakEndTime - peakStartTime) / (double)ANALAYZE_FERQ;
				//obliczenie wysokosci piku
				peakH = peakVal - threshold;
				//podjecie decyzji o wykryciu wydechu
				if (peakW > minW && peakH > minH)
				{
					//przeliczenie numeru probek na sekundy
					peakSecond = peakTime / (double)ANALAYZE_FERQ;
					//obliczenie czestosci oddechu
					BPM = 1 / (peakSecond - previousPeakSecond) * 60;
					//zapisanie czasu piku (probka o najwyzszej wartosci)
					previousPeakSecond = peakSecond;

					//wyswietlenie i przeslanie wyniku pomiaru (wywolanie handlera)
					async(launch::async, handler, BPM);
				}
			}
		}
	}
	//inkrementacjia offsetu
	offset += ANALAYZE_SIZE;
}


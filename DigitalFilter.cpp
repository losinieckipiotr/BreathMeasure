#include "DigitalFilter.h"
#include <cstring>

DigitalFilter::DigitalFilter(const unsigned int order, const double B[], const double A[])
	: order(order)
{
    //alokacja pamieci na wspolczynniki i rejestry
	b = new double[order + 1];
	a = new double[order + 1];
	x = new double[order + 1];
	y = new double[order + 1];
    //wypelnienie tablicy wspolczynnikow B
	for (size_t i = 0; i < order + 1; i++)
	{
		b[i] = B[i];
	}
    //wypelnienie tablicy wspolczynnikow A
	a[0] = A[0];
	for (size_t i = 1; i < order + 1; i++)
	{
		a[i] = -A[i];
	}
    //wyzerowanie rejestrow
	Reset();
}


DigitalFilter::~DigitalFilter()
{
	delete[]b;
	delete[]a;
	delete[]x;
	delete[]y;
}

void DigitalFilter::Filter(double data[], const size_t size)
{
	for (size_t i = 0; i < size; ++i)
	{
		//przesuniecie danych w rejestrach
		for (size_t j = order; j > 0; --j)
		{
			x[j] = x[j - 1];
			y[j] = y[j - 1];
		}
		x[0] = data[i];
		//obliczanie wartosci przefiltrowanej probki
		t = x[0] * b[0];
		for (size_t j = 1; j < order + 1; j++)
		{
			t += x[j] * b[j];
			t += y[j] * a[j];
		}
		//zazwyczaj mnozenie razy 1
		y[0] = t*a[0];
		//zapisanie wartosci przefiltrowanej probki
		data[i] = y[0];
	}
}

void DigitalFilter::Filter(const double data[], double out[], const size_t size)
{
	for (size_t i = 0; i < size; ++i)
	{
        //przesuniecie danych w rejestrach
		for (size_t j = order; j > 0; --j)
		{
			x[j] = x[j - 1];
			y[j] = y[j - 1];
		}
		x[0] = data[i];
        //obliczanie wartosci przefiltrowanej probki
		t = x[0] * b[0];
		for (size_t j = 1; j < order + 1; j++)
		{
			t += x[j] * b[j];
			t += y[j] * a[j];
		}
        //zazwyczaj mnozenie razy 1
		y[0] = t*a[0];
		//zapisanie wartosci przefiltrowanej probki
		out[i] = y[0];
	}
}

void DigitalFilter::Reset()
{
	memset(x, 0, sizeof(double)*(order + 1));
	memset(y, 0, sizeof(double)*(order + 1));
}


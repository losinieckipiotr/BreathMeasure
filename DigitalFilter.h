#ifndef DIGITAL_FILTER_H
#define DIGITAL_FILTER_H
#include <cstdlib>

class DigitalFilter
{
public:
	DigitalFilter(const unsigned int order, const double B[], const double A[]);
	~DigitalFilter();

	void Filter(double data[], const size_t size);
	void Filter(const double data[], double out[], const size_t size);
	void Reset();

private:
	const unsigned int order;

	double *buf;

	double *x;
	double *y;

	double *b;
	double *a;

	double t;
};

#endif // !DIGITAL_FILTER_H


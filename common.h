#ifndef LIB_COMMON_H_
#define LIB_COMMON_H_

void reseed();
double randf();
double randf(double min, double max);
double gaussrandf(double mean, double stddev);

extern const double E_NUM;
extern long randseed;

#endif
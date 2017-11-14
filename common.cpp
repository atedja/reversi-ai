#include "lib.h"
#include "common.h"

const double E_NUM = 2.71828183;

#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define NTAB 32
#define NDIV (1+(IM-1)/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)

long randseed = -1;

// Minimal Standard of Park and Miller with Bays-Durham shuffle
// From: Numerical Recipes in C
double ran1()
{
   int j;
   long k;
   static long iy=0;
   static long iv[NTAB];
   double temp;
   if (randseed <= 0 || !iy)
   {
      if (-(randseed) < 1) randseed=1;
      else randseed = -(randseed);
      for (j=NTAB+7;j>=0;j--)
      {
         k=(randseed)/IQ;
         randseed=IA*(randseed-k*IQ)-IR*k;
         if (randseed < 0) randseed += IM;
         if (j < NTAB) iv[j] = randseed;
      }
      iy=iv[0];
   }
   k=(randseed)/IQ;
   randseed=IA*(randseed-k*IQ)-IR*k;
   if (randseed < 0) randseed += IM;
   j=iy/NDIV;
   iy=iv[j];
   iv[j] = randseed;
   if ((temp=AM*iy) > RNMX) return RNMX;
   else return temp;
}

void reseed()
{
   randseed = -(long)time(0);
   srand( (unsigned)time(0) );
}

// randf()
// Produces a random number (0.0,1.0)
// WARNING: System-supplied rand() is not a good uniform generator.
double randf()
{
   //return rand()/(RAND_MAX+1.0);
   return ran1();
}

// randf()
// Produces a random number (min,max)
double randf(double min, double max)
{
   double range = max-min;
   return range*randf()+min;
}


// Returns a normally distributed deviate with zero mean and unit variance, using ran1()
// as the source of uniform deviates.
// From: Numerical Recipes in C.
double gasdev()
{
   static int iset=0;
   static double gset;
   double fac,rsq,v1,v2;
   if (randseed < 0) iset=0;
   if (iset == 0)
   {
      do
      {
         v1=2.0*ran1()-1.0;
         v2=2.0*ran1()-1.0; 
         rsq=v1*v1+v2*v2;
      } while (rsq >= 1.0 || rsq == 0.0);
      fac=sqrt(-2.0*log(rsq)/rsq);
      gset=v1*fac;
      iset=1;
      return v2*fac;
   }
   else
   {
      iset=0;
      return gset;
   }
}


// gaussrandf()
// Produces a Gaussian random number with mean 'mean' and standard deviation 'stddev'.
// Using a sample of 10 uniform distributions.
// Using Central Limit Theorem.
double gaussrandf(double mean, double stddev)
{
   //double x=0.0;
   //for( int i=0; i<10; ++i )
   //   x += randf();
   //x = stddev*(x-5.0)+mean;
   //return x;

   return gasdev();
}



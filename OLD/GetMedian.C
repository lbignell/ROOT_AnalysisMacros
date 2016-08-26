/******************************************************************************
* macro to test median                                                        *
*                                                                             *
* Author: Dr. Christian Stratowa, Vienna, Austria.                            *
* Created: 23 Sep 2003                             Last modified: 24 Sep 2004 *
******************************************************************************/

#include "TMath.h"
#include "TRandom.h"

#include <math.h>
#include <Riostream.h>

/**********************************************************
 ** Source code from BioConductor package "affy"
 **
 ** int sort_double(const void *a1,const void *a2)
 ** 
 ** a comparison function used when sorting doubles.
 **
 **********************************************************/

int sort_double(const double *a1,const double *a2){
  if (*a1 < *a2)
    return (-1);
  if (*a1 > *a2)
    return (1);
  return 0;
}

/**************************************************************************
 ** Source code from BioConductor package "affy"
 **
 ** double median(double *x, int length)
 **
 ** double *x - vector
 ** int length - length of *x
 **
 ** returns the median of *x
 **
 *************************************************************************/

double  median(double *x, int length)
{
  int i;
  int half;
  double med;
  double *buffer = (double *)calloc(length,sizeof(double));
  
  for (i = 0; i < length; i++)
    buffer[i] = x[i];
  
  qsort(buffer,length,sizeof(double), (int(*)(const void*, const void*))sort_double);
  half = (length + 1)/2;
  if (length % 2 == 1){
    med = buffer[half - 1];
  } else {
    med = (buffer[half] + buffer[half-1])/2.0;
  }
  
  free(buffer);
  return med;
}

//______________________________________________________________________________
Double_t XMedian(Int_t n, const Double_t *arr)
{
   // Calculate median (source code C. Stratowa)

   if (n == 1) return arr[0];

// Create index and sort array
   Int_t *index = 0;
   if (!(index = new Int_t[n])) {return 1;} //return not ok
   TMath::Sort(n, arr, index);

// Find median
   Int_t k;
   Double_t median = 0;
   if ((n % 2) == 0){
      k = (Int_t)TMath::Floor(n / 2.0) - 1;
      median = (arr[index[k]] + arr[index[k+1]])/2;
   } else {
      k = (Int_t)TMath::Floor(n / 2.0);
      median = arr[index[k]];
   }//if

   delete [] index;

   return median;
}//Median

//______________________________________________________________________________
void macroMedian()
{
   Int_t n = 1000;
   Double_t *x = new Double_t[n];

   for (Int_t i=0; i<n; i++) x[i] = gRandom->Rndm();

   cout << "Median (ROOT) = " << TMath::Median(n, x) << endl; 
   cout << "Median (C.S.) = " << XMedian(n, x) << endl; 
   cout << "Median (CRAN) = " << median(x, n) << endl; 

   delete [] x;
}//macroMedian

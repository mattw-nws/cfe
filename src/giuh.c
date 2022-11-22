#ifndef _GIUH_C

#define _GIUH_C

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "../include/giuh.h"


//##############################################################
//############### GIUH CONVOLUTION INTEGRAL   ##################
//##############################################################
extern double giuh_convolution_integral(double runoff_m,int num_giuh_ordinates, 
					double *giuh_ordinates, double *runoff_queue_m_per_timestep)
{
  //##############################################################
  // This function solves the convolution integral involving N
  //  GIUH ordinates.
  //##############################################################
  double runoff_m_now;
  int N,i;
  
  N=num_giuh_ordinates;
  runoff_queue_m_per_timestep[N]=0.0;
  
  for(i=0;i<N;i++)
    {
      runoff_queue_m_per_timestep[i]+=giuh_ordinates[i]*runoff_m;
    }
  
  runoff_m_now=runoff_queue_m_per_timestep[0];
  
  for(i=1;i<N;i++)  // shift all the entries in preperation for the next timestep
    {
      runoff_queue_m_per_timestep[i-1]=runoff_queue_m_per_timestep[i];
    }
  runoff_queue_m_per_timestep[N-1]=0.0;
  
  return runoff_m_now;
}

extern int giuh_get_num_ordinates_scaled(int num_giuh_ordinates, double factor)
{
  // Naive... sufficient?
  return ceil(num_giuh_ordinates / factor);
}

extern double *giuh_get_ordinates_scaled(int num_giuh_ordinates, double *giuh_ordinates, double factor)
{
  int num_ordinates_scaled = giuh_get_num_ordinates_scaled(num_giuh_ordinates, factor);
  double *scaled_ordinates = malloc(num_ordinates_scaled * sizeof(double));
  int err = giuh_scale_ordinates(num_giuh_ordinates, giuh_ordinates, scaled_ordinates, factor);
  //TODO: If error (of some kind??), issue warning and copy original ordinates into the new array.
  return scaled_ordinates;
}

// scaled_ordinates is a preallocated buffer of the correct size!
extern int giuh_scale_ordinates(int num_giuh_ordinates, double *giuh_ordinates, double *scaled_ordinates, double factor)
{
  int num_ordinates_scaled = giuh_get_num_ordinates_scaled(num_giuh_ordinates, factor);
  for(int i=0; i<num_ordinates_scaled; i++) scaled_ordinates[i] = 0.0; // init scaled ordinates to zero

  double index_f = 0.0;
  for(int i=0; i<num_ordinates_scaled; i++)
    {
      double index_end_f = index_f + factor;
      int lbound = floor(index_f);
      int ubound = fmin(ceil(index_end_f)-1, num_giuh_ordinates-1);
      //int ubound = ceil(index_end_f)-1;
      for(int j = lbound; j <= ubound; j++){
        double ij_part = 1.0;
        if(j > lbound && j < ubound){
          scaled_ordinates[i] += giuh_ordinates[j];
          continue; // ij_part was 1.0!
        }
        // Only if we are on an edge, proceed...
        if(j == lbound){
          // e.g. index_f == 2.4 and j == 2, deduct 0.4
          ij_part -= index_f - j;
        }
        if(j == ubound && index_end_f < num_ordinates_scaled){
          // e.g. index_f == 2.4, factor == 0.6 (index_end_f == 3.2) and j == 3, deduct 0.8
          ij_part -= (j+1) - (index_end_f);
        }
        scaled_ordinates[i] += ij_part * giuh_ordinates[j];
        //printf("\n  For i: %d, j; %d, ij_part is %.3f, so adding %.4f    (lbound: %d, ubound: %d, index_f: %f, index_end_f: %f, num_ordinates_scaled: %d)\n", i, j, ij_part, ij_part * giuh_ordinates[j], lbound, ubound, index_f, index_end_f, num_ordinates_scaled);
      }
      index_f += factor;
    }
    return 0;
}



#endif

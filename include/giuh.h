#ifndef _GIUH_H

#define _GIUH_H


extern double giuh_convolution_integral(double runoff_m, int num_giuh_ordinates, 
                                   double *giuh_ordinates, double *runoff_queue_m_per_timestep);

extern int giuh_get_num_ordinates_scaled(int num_giuh_ordinates, double factor);

extern double *giuh_get_ordinates_scaled(int num_giuh_ordinates, double *giuh_ordinates, double factor);

extern int giuh_scale_ordinates(int num_giuh_ordinates, double *giuh_ordinates, double *scaled_ordinates, double factor);

#endif

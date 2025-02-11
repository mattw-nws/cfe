#ifndef CFE_CFE_H
#define CFE_CFE_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include <assert.h>

#include "giuh.h"

#define TRUE 1
#define FALSE 0
#define MAX_NUM_GIUH_ORDINATES 10
#define MAX_NUM_NASH_CASCADE    3
#define MAX_NUM_RAIN_DATA 720

// t-shirt approximation of the hydrologic routing funtionality of the National Water Model v 1.2, 2.0, and 2.1
// This code was developed to test the hypothesis that the National Water Model runoff generation, vadose zone
// dynamics, and conceptual groundwater model can be greatly simplified by acknowledging that it is truly a 
// conceptual model.  The hypothesis is supported by a number of observations made during a 2017-2018 deep dive
// into the NWM code.  Thesed are:
//
// 1. Rainfall/throughfall/melt partitioning in the NWM is based on a simple curve-number like approach that
//    was developed by Schaake et al. (1996) and which is very similar to the Probability Distributed Moisture (PDM)
//    function by Moore, 1985.   The Schaake function is a single valued function of soil moisture deficit,
//    predicts 100% runoff when the soil is saturated, like the curve-number method, and is fundamentally simple.
// 2. Run-on infiltration is strictly not calculated.  Overland flow routing applies the Schaake function repeatedly
//    to predict this phenomenon, which violates the underlying assumption of the PDM method that only rainfall 
//    inputs affect soil moisture.
// 3. The water-content based Richards' equation, applied using a coarse-discretization, can be replaced with a simple
//    conceptual reservoir because it never allows saturation or infiltration-excess runoff unless deactivated by
//    assuming no-flow lower boundary condition.  Since this form of Richards' equation cannot simulate heterogeneous
//    soil layers, it can be replaced with a conceptual reservoir.
// 4. The lateral flow routing function in the NWM is purely conceptual.  It is activated whenever the soil water
//    content in one or more of the four Richards-equation discretizations reaches the wilting point water content.
//    This activation threshold is physically unrealistic, because in most soils lateral subsurface flow is not
//    active until pore water pressures become positive at some point in the soil profile.  Furthermore, the lateral
//    flow hydraulic conductivity is assumed to be the vertical hydraulic conductivity multiplied by a calibration
//    factor "LKSATFAC" which is allowed to vary between 10 and 10,000 during calibration, resulting in an anisotropy
//    ratio that varies over the same range, without correlation with physiographic characteristics or other support.
//
//    This code implements these assumptions using pure conceptualizations.  The formulation consists of the following:
//
//    1. Rainfall is partitioned into direct runoff and soil moisture using the Schaake function.
//    2. Rainfall that becomes direct runoff is routed to the catchment outlet using a geomorphological instantanteous
//       unit hydrograph (GIUH) approach, eliminating the 250 m NWM routing grid, and the incorrect use of the Schaake
//       function to simulate run-on infiltration.
//    3. Water partitioned by the Schaake function to be soil moisture is placed into a conceptual linear reservoir
//       that consists of two outlets that apply a minimum storage activation threshold.   This activation threshold
//       is identical for both outlets, and is based on an integral solution of the storage in the soil assuming
//       Clapp-Hornberger parameters equal to those used in the NWM to determine that storage corresponding to a
//       soil water content 0.5 m above the soil column bottom that produces a soil suction head equal to -1/3 atm,
//       which is a commonly applied assumption used to estimate the field capacity water content.
//       The first outlet calculates vertical percolation of water to deep groundwater using the saturated hydraulic
//       conductivity of the soil multiplied by the NWM "slope" parameter, which when 1.0 indicates free drainage and
//       when 0.0 indicates a no-flow lower boundary condition.   The second outlet is used to calculate the flux to
//       the soil lateral flow path, using a conceptual LKSATFAC-like calibration parameter.
//    4. The lateral flow is routed to the catchment outlet using a Nash-cascade of reservoirs to produce a mass-
//       conserving delayed response, and elminates the need for the 250 m lateral flow routing grid.
//    5. The groundwater contribution to base flow is modeled using either (a) an exponential nonlinear reservoir
//       identical to the one in the NWM formulation, or (b) a nonlinear reservoir forumulation, which can also be
//       made linear by assuming an exponent value equal to 1.0.
//
//    This code was written entirely by Fred L. Ogden, May 22-24, 2020, in the service of the NOAA-NWS Office of Water
//    Prediction, in Tuscaloosa, Alabama.

// define data structures
//--------------------------

struct conceptual_reservoir
{
// this data structure describes a nonlinear reservoir having two outlets, one primary with an activation
// threshold that may be zero, and a secondary outlet with a threshold that may be zero
// this will also simulate a linear reservoir by setting the exponent parameter to 1.0 iff is_exponential==FALSE
// iff is_exponential==TRUE, then it uses the exponential discharge function from the NWM V2.0 forumulation
// as the primary discharge with a zero threshold, and does not calculate a secondary discharge.
//--------------------------------------------------------------------------------------------------
int    is_exponential;  // set this true TRUE to use the exponential form of the discharge equation
double gw_storage;   // Initial Storage - LKC: added since I need to keep track of it when changing parameters
double storage_max_m;   // maximum storage in this reservoir
double storage_m;       // state variable.
double storage_change_m; // storage change in the current step
double coeff_primary;    // the primary outlet
double exponent_primary;
double storage_threshold_primary_m;
double storage_threshold_secondary_m;
double coeff_secondary;
double exponent_secondary;
double ice_fraction_schaake, ice_fraction_xinan;
int   is_sft_coupled; // boolean - true if SFT is ON otherwise OFF (default is OFF)

//---Root zone adjusted AET development -rlm -ahmad -------------
double *smc_profile; //soil moisture content profile
int n_soil_layers; // number of soil layers
double *soil_layer_depths_m; // soil layer depths defined in the config file in units of [m]
int aet_root_zone; // boolean - true if aet_root_zone is ON otherwise OFF (default is OFF)
int max_root_zone_layer;  // maximum root zone layer is used to identify the maximum layer to remove water from for AET
double *delta_soil_layer_depth_m; // used to calculate the total soil moisture in each layer
double soil_water_content_field_capacity;  // water content [m/m] at field capacity.  Used in AET routine 

//---------------------------------------------------------------
};

struct NWM_soil_parameters
{
// using same variable names as used in NWM.  <sorry>
double smcmax;  // effective porosity [V/V]
double wltsmc;  // wilting point soil moisture content [V/V]
double satdk;   // saturated hydraulic conductivity [m s-1]
double satpsi;	// saturated capillary head [m]
double bb;      // beta exponent on Clapp-Hornberger (1978) soil water relations [-]
double mult;    // the multiplier applied to satdk to route water rapidly downslope
double slop;   // this factor (0-1) modifies the gradient of the hydraulic head at the soil bottom.  0=no-flow.
double D;       // soil depth [m]
double wilting_point_m;
// LKC: Add this two parameters since they belong to soils. Makes the parameter specification consistent
double alpha_fc;
double refkdt;
double soil_storage;
};

struct evapotranspiration_structure {
    double potential_et_m_per_s;
    double potential_et_m_per_timestep;
    double reduced_potential_et_m_per_timestep;
    double actual_et_from_rain_m_per_timestep;
    double actual_et_from_soil_m_per_timestep;
    double actual_et_m_per_timestep;
};
typedef struct evapotranspiration_structure evapotranspiration_structure;

struct massbal
{
    double volstart            ;
    double vol_runoff          ;   
    double vol_infilt          ;   
    double vol_out_giuh        ;
    double vol_end_giuh        ;
    double vol_to_gw           ;
    double vol_in_gw_start     ;
    double vol_in_gw_end       ;
    double vol_from_gw         ;
    double vol_in_nash         ;
    double vol_in_nash_end     ;  // note the nash cascade is empty at start of simulation.
    double vol_out_nash        ;
    double vol_soil_start      ;
    double vol_to_soil         ;
    double vol_soil_to_lat_flow;
    double vol_soil_to_gw      ;  // this should equal vol_to_gw
    double vol_soil_end        ;
    double vol_et_from_soil    ;
    double vol_et_from_rain    ; 
    double vol_et_to_atm       ;   
    double volin               ;
    double volout              ;
    double volend              ;
};
typedef struct massbal massbal;

// define data types
//--------------------------
typedef enum {Schaake=1, Xinanjiang=2} surface_water_partition_type;

/* xinanjiang_dev*/
struct direct_runoff_parameters_structure{
    surface_water_partition_type surface_partitioning_scheme;
    double Schaake_adjusted_magic_constant_by_soil_type;
    double a_Xinanjiang_inflection_point_parameter;
    double b_Xinanjiang_shape_parameter;
    double x_Xinanjiang_shape_parameter;
    double urban_decimal_fraction;
    double ice_content_threshold; // ice content above which soil is impermeable
};
typedef struct direct_runoff_parameters_structure direct_runoff_parameters_structure;


// function prototypes
// --------------------------------
extern void Schaake_partitioning_scheme(double dt, double field_capacity_m, double magic_number, double deficit, double qinsur,
					double smcmax, double soil_depth, double *runsrf, double *pddum, double ice_fraction_schaake, double ice_content_threshold);

// xinanjiang_dev: XinJiang function written by Rachel adapted by Jmframe and FLO, 
extern void Xinanjiang_partitioning_scheme(double water_input_depth_m, double field_capacity_m,
					   double max_soil_moisture_storage_m, double column_total_soil_water_m,
					   struct direct_runoff_parameters_structure *parms, double *surface_runoff_depth_m,
					   double *infiltration_depth_m, double ice_fraction_xinan);

extern void conceptual_reservoir_flux_calc(struct conceptual_reservoir *da_reservoir,
                                           double *primary_flux_m, double *secondary_flux_m);

extern double convolution_integral(double runoff_m, int num_giuh_ordinates, 
                                   double *giuh_ordinates, double *runoff_queue_m_per_timestep);
                                   
extern double nash_cascade(double flux_lat_m,int num_lateral_flow_nash_reservoirs,
                           double K_nash,double *nash_storage_arr);

extern void et_from_rainfall(double *timestep_rainfall_input_m, struct evapotranspiration_structure *et_struct);

extern void et_from_soil(struct conceptual_reservoir *soil_res, struct evapotranspiration_structure *et_struct, struct NWM_soil_parameters *soil_parms);

extern int is_fabs_less_than_epsilon(double a,double epsilon);

extern void cfe(
        double *soil_reservoir_storage_deficit_m_ptr,
        struct NWM_soil_parameters NWM_soil_params_struct,
        struct conceptual_reservoir *soil_reservoir_struct,
        double timestep_h,

        /* xinanjiang_dev: since we are doing the option for Schaake and XinJiang, 
                           instead of passing in the constants
                           pass in a structure with the constants for both subroutines.
        //double Schaake_adjusted_magic_constant_by_soil_type,*/
        struct direct_runoff_parameters_structure direct_runoff_param_struct,

        double timestep_rainfall_input_m,

        /* xinanjiang_dev:
        double *Schaake_output_runoff_m_ptr,*/
        double *flux_output_direct_runoff_m,

        double *infiltration_depth_m_ptr,
        double *flux_perc_m_ptr,
        double *flux_lat_m_ptr,
        double *gw_reservoir_storage_deficit_m_ptr,
        struct conceptual_reservoir *gw_reservoir_struct,
        double *flux_from_deep_gw_to_chan_m_ptr,
        double *giuh_runoff_m_ptr,
        int num_giuh_ordinates,
        double *giuh_ordinates_arr,
        double *runoff_queue_m_per_timestep_arr,
        double *nash_lateral_runoff_m_ptr,
        int num_lateral_flow_nash_reservoirs,
        double K_nash,
        double *nash_storage_arr,
        struct evapotranspiration_structure *evap_struct,
        double *Qout_m_ptr,
        struct massbal *massbal_struct,
        double time_step_size
    );

#endif //CFE_CFE_H

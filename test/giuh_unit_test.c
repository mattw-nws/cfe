#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include "../include/giuh.h" 

double compare_arrays_cumulative_delta(double a[], double b[], int n) {
  double delta = 0.0;
  for(int i = 0; i < n; i++) {
    delta += fabs(a[i] - b[i]);
  }
  return delta;
}

void print_array(double a[], int n) {
  for(int i = 0; i < n; i++) {
    printf("%f", a[i]);
    if(i < n-1) printf(", ");
  }
}

double total_array(double a[], int n) {
  double t;
  for(int i = 0; i < n; i++) {
    t += a[i];
  }
  return t;
}

int main(int argc, const char *argv[]){

    double delta = 0.0;
    double factor = 1.0;
    int num_ordinates = 0;
    int num_ordinates_scaled = 0;
    double *scaled;

    printf("\nBEGIN GIUH UNIT TEST\n*******************\n");

    double test_giuh_initial_1[] = {0.1, 0.3, 0.4, 0.2};
    num_ordinates = 4;

    double test_giuh_expected_1_1[] = {0.05, 0.05, 0.15, 0.15, 0.2, 0.2, 0.1, 0.1};
    factor = 0.5;
    num_ordinates_scaled = giuh_get_num_ordinates_scaled(num_ordinates, factor);
    scaled = giuh_get_ordinates_scaled(num_ordinates, test_giuh_initial_1, factor);
    delta = compare_arrays_cumulative_delta(scaled, test_giuh_expected_1_1, num_ordinates_scaled);
    printf("num_ordinates_scaled: %d, delta: %f, scaled: ", num_ordinates_scaled, delta); print_array(scaled, num_ordinates_scaled); printf(" (total: %f)\n", total_array(scaled, num_ordinates_scaled));
    if(delta >= 1e-8){
        printf("FAIL: Delta from test_giuh_expected_1_1 is %.8f\n", delta);
        printf(" (expected ");
        print_array(test_giuh_expected_1_1, num_ordinates_scaled);
        printf(")\n");
    }
    free(scaled);

    double test_giuh_expected_1_2[] = {0.4, 0.6};
    factor = 2.0;
    num_ordinates_scaled = giuh_get_num_ordinates_scaled(num_ordinates, factor);
    scaled = giuh_get_ordinates_scaled(num_ordinates, test_giuh_initial_1, factor);
    delta = compare_arrays_cumulative_delta(scaled, test_giuh_expected_1_2, num_ordinates_scaled);
    printf("num_ordinates_scaled: %d, delta: %f, scaled: ", num_ordinates_scaled, delta); print_array(scaled, num_ordinates_scaled); printf(" (total: %f)\n", total_array(scaled, num_ordinates_scaled));
    if(delta >= 1e-8){
        printf("FAIL: Delta from test_giuh_expected_1_2 is %.8f\n", delta);
        printf(" (expected ");
        print_array(test_giuh_expected_1_2, num_ordinates_scaled);
        printf(")\n");
    }
    free(scaled);

    double test_giuh_expected_1_3[] = {0.066666666667, 0.133333333333, 0.2, 0.266666666667, 0.2, 0.13333333333333};
    factor = 0.666666666667;
    num_ordinates_scaled = giuh_get_num_ordinates_scaled(num_ordinates, factor);
    scaled = giuh_get_ordinates_scaled(num_ordinates, test_giuh_initial_1, factor);
    delta = compare_arrays_cumulative_delta(scaled, test_giuh_expected_1_3, num_ordinates_scaled);
    printf("num_ordinates_scaled: %d, delta: %f, scaled: ", num_ordinates_scaled, delta); print_array(scaled, num_ordinates_scaled); printf(" (total: %f)\n", total_array(scaled, num_ordinates_scaled));
    if(delta >= 1e-8){
        printf("FAIL: Delta from test_giuh_expected_1_3 is %.8f\n", delta);
        printf(" (expected ");
        print_array(test_giuh_expected_1_3, num_ordinates_scaled);
        printf(")\n");
    }
    free(scaled);

    double test_giuh_expected_1_4[] = {0.175000, 0.425000, 0.350000, 0.050000};
    factor = 1.25;
    num_ordinates_scaled = giuh_get_num_ordinates_scaled(num_ordinates, factor);
    scaled = giuh_get_ordinates_scaled(num_ordinates, test_giuh_initial_1, factor);
    delta = compare_arrays_cumulative_delta(scaled, test_giuh_expected_1_4, num_ordinates_scaled);
    printf("num_ordinates_scaled: %d, delta: %f, scaled: ", num_ordinates_scaled, delta); print_array(scaled, num_ordinates_scaled); printf(" (total: %f)\n", total_array(scaled, num_ordinates_scaled));
    if(delta >= 1e-8){
        printf("FAIL: Delta from test_giuh_expected_1_4 is %.8f\n", delta);
        printf(" (expected ");
        print_array(test_giuh_expected_1_4, num_ordinates_scaled);
        printf(")\n");
    }
    free(scaled);

    double test_giuh_expected_1_5[] = {0.199900, 0.466500, 0.333400, 0.000200};
    factor = 1.333;
    num_ordinates_scaled = giuh_get_num_ordinates_scaled(num_ordinates, factor);
    scaled = giuh_get_ordinates_scaled(num_ordinates, test_giuh_initial_1, factor);
    delta = compare_arrays_cumulative_delta(scaled, test_giuh_expected_1_5, num_ordinates_scaled);
    printf("num_ordinates_scaled: %d, delta: %f, scaled: ", num_ordinates_scaled, delta); print_array(scaled, num_ordinates_scaled); printf(" (total: %f)\n", total_array(scaled, num_ordinates_scaled));
    if(delta >= 1e-8){
        printf("FAIL: Delta from test_giuh_expected_1_5 is %.8f!", delta);
        printf(" (expected ");
        print_array(test_giuh_expected_1_5, num_ordinates_scaled);
        printf(")\n");
    }
    free(scaled);

    return 0;
}

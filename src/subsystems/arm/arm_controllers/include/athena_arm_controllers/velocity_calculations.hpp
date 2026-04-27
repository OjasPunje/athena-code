#ifndef VELOCITY_KINEMATIC_CALCULATIONS_H
#define VELOCITY_KINEMATIC_CALCULATIONS_H

#include <vector>

void velocity_kinematics_calculations(
  std::vector<double> &, std::vector<double> &, std::vector<double>, std::vector<double>);
void calculate_jacobian(double, double, double, double**, double*);
double calculate_determinant(double**);
void calculate_inverse(double**, double);
void init_matrix(double**&, int);

#endif
#include <vector>
#include <cmath>
#include <cstdio>

void init_matrix(double**& mat, int size){
	mat = new double*[size];
	for(int i = 0; i < size; i++){
		mat[i] = new double[size];
	}
}

void calculate_jacobian(double theta1, double theta2, double theta3, double** J, double* link_lengths){
	double alpha = theta1 + theta2;
	double beta = alpha + theta3;
	// printf("Link lengths: %f", J[0][0]);

	J[0][0] = -(link_lengths[0] * std::sin(theta1) + link_lengths[1] * std::sin(alpha) + link_lengths[2] * std::sin(beta));
	J[0][1] = -(link_lengths[1] * std::sin(alpha)  + link_lengths[2] * std::sin(beta));
	J[0][2] = -(link_lengths[2] * std::sin(beta));
	J[1][0] = link_lengths[0] * std::cos(theta1) + link_lengths[1] * std::cos(alpha) + link_lengths[2] * std::cos(beta);
	J[1][1] = link_lengths[1] * std::cos(alpha)  + link_lengths[2] * std::cos(beta);
	J[1][2] = link_lengths[2] * std::cos(beta);
	J[2][0] = J[2][1] = J[2][2] = 1;
}

double calculate_determinant(double** mat){
	double det;
	double term1 = mat[0][0] * (mat[1][1]*mat[2][2] - mat[1][2]*mat[2][1]);
	double term2 = -(mat[0][1] * (mat[1][0]*mat[2][2] - mat[1][2]*mat[2][0]));
	double term3 = mat[0][2] * (mat[1][0]*mat[2][1] - mat[1][1]*mat[2][0]);
	det = term1+term2+term3;
	return det;
}

void calculate_inverse(double** J, double det){
	double** Ji;
	init_matrix(Ji, 3);
	if(det == 0) det = 1;

	// printf("Matrix val: %f", Ji[0][0]);

	Ji[0][0] = (J[1][1]*J[2][2] - J[1][2]*J[2][1]) / det;
	Ji[0][1] = (J[0][2]*J[2][1] - J[0][1]*J[2][2]) / det;
	Ji[0][2] = (J[0][1]*J[1][2] - J[0][2]*J[1][1]) / det;

	Ji[1][0] = (J[1][2]*J[2][0] - J[1][0]*J[2][2]) / det;
	Ji[1][1] = (J[0][0]*J[2][2] - J[0][2]*J[2][0]) / det;
	Ji[1][2] = (J[0][2]*J[1][0] - J[0][0]*J[1][2]) / det;

	Ji[2][0] = (J[1][0]*J[2][1] - J[1][1]*J[2][0]) / det;
	Ji[2][1] = (J[0][1]*J[2][0] - J[0][0]*J[2][1]) / det;
	Ji[2][2] = (J[0][0]*J[1][1] - J[0][1]*J[1][0]) / det;

	J = Ji;
}

void velocity_kinematics_calculations(
  std::vector<double> & command_velocities, std::vector<double> & joint_velocities, std::vector<double> link_lengths, std::vector<double> joint_positions)
{
	double** J;
	init_matrix(J, 3);
	double vy = command_velocities[1];
	double vz = command_velocities[2];
	double thetadot = command_velocities[3];
	double theta1 = joint_positions[0];
	double theta2 = joint_positions[1];
	double theta3 = joint_positions[2];
	double* arm = link_lengths.data();


	// printf("Arm val: %f", J[0][0]);

	// Calculate Jacobian 
	// TODO Define theta1, theta2, theta3, link lengths
	calculate_jacobian(theta1, theta2, theta3, J, arm);
	calculate_inverse(J, calculate_determinant(J));


	// Base yaw
	joint_velocities[0] = command_velocities[0];

	// Shoulder
	joint_velocities[1] = vy * J[0][0] + vz * J[0][1] + thetadot * J[0][2];

	// Elbow
	joint_velocities[2] = vy * J[1][0] + vz * J[1][1] + thetadot * J[1][2];

	// Wrist Pitch
	joint_velocities[3] = vy * J[2][0] + vz * J[2][1] + thetadot * J[2][2];

	// Wrist Roll
	joint_velocities[4] = command_velocities[4];

	// Open Gripper Claw
	joint_velocities[5] = command_velocities[5];

	// Close Gripper Claw
	joint_velocities[6] = command_velocities[6];
}
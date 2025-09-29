#pragma once

#include <string>
#include <Eigen/Dense>
class SNIRF {
public:
	SNIRF(const std::string& filepath);
	~SNIRF();

	void Print();

	Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> channel_data = {};
};
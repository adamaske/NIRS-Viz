#include "Snirf.h"

#include <highfive/highfive.hpp>
#include <spdlog/spdlog.h>
namespace h5 = HighFive;
SNIRF::SNIRF(const std::string& filepath)
{
	h5::File snirf(filepath, h5::File::AccessMode::ReadOnly);

	snirf.getName();

	auto info = snirf.getInfo();

	auto adress = info.getAddress()
		spdlog::info("SNIRF file opened: {}", adress);
}

SNIRF::~SNIRF()
{

}

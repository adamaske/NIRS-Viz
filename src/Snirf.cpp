#include "Snirf.h"
#include <highfive/H5File.hpp>
#include <highfive/H5Group.hpp>
#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5Easy.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <Eigen/Dense>
#include <spdlog/spdlog.h>

using namespace HighFive;

// Function to get a string representation of the dataset shape
std::string get_dataset_shape(const DataSet& dataset) {
    try {
        std::vector<size_t> dims = dataset.getDimensions();
        std::string shape_str = "(";
        for (size_t i = 0; i < dims.size(); ++i) {
            shape_str += std::to_string(dims[i]);
            if (i < dims.size() - 1) {
                shape_str += ", ";
            }
        }
        shape_str += ")";
        return shape_str;
    }
    catch (const Exception& e) {
        return "(Error getting shape)";
    }
}


// Recursive function to traverse the HDF5 structure
void parse_group(const Group& current_group, const std::string& path) {
    // 1. Get all object names in the current group
    std::vector<std::string> object_names = current_group.listObjectNames();

    // 2. Iterate through objects
    for (const auto& name : object_names) {
        std::string current_path = path + "/" + name;
        ObjectType type = current_group.getObjectType(name);

        switch (type) {
        case ObjectType::Group: {
            // It's a Group: print info and recurse
            std::cout << "  [Group]  " << current_path << std::endl;
            Group next_group = current_group.getGroup(name);
            parse_group(next_group, current_path);
            break;
        }
        case ObjectType::Dataset: {
            // It's a Dataset: print info including shape
            DataSet dataset = current_group.getDataSet(name);
            std::string shape = get_dataset_shape(dataset);
            std::cout << "  [Dataset] " << current_path << " - Shape: " << shape << std::endl;
            break;
        }
        default: {
            // Other types (e.g., named DataType)
            std::cout << "  [Other]  " << current_path << std::endl;
            break;
        }
        }
    }
}

// Main parsing function
void parse_hdf5_file(const std::string& filename) {
    try {
        // Open the file in read-only mode
        File file(filename, File::ReadOnly);

        std::cout << "--- Parsing HDF5 File: " << filename << " ---" << std::endl;

        // Start the recursive parsing from the root group (which is the File object itself)
        Group root_group = file.getGroup("/");
        parse_group(root_group, "");

        std::cout << "--- End of File Structure ---" << std::endl;

    }
    catch (const Exception& err) {
        std::cerr << "Error opening or parsing HDF5 file: " << err.what() << std::endl;
    }
}

SNIRF::SNIRF(const std::string& filepath)
{
    parse_hdf5_file(filepath);
    File file(filepath, File::ReadOnly);
    Group root_group = file.getGroup("/");

    //DataSet format = root_group.getDataSet("/formatVersion");

	Group nirs = root_group.getGroup("/nirs");
    Group data1 = nirs.getGroup("data1");

    DataSet time_series = data1.getDataSet("dataTimeSeries");

    // Print me the info about the time_series dataset
    std::vector<size_t> dims = time_series.getDimensions(); 
    if (dims.size() != 2) {
        spdlog::error("dataTimeSeries is not 2D!");
        return;
    }
	spdlog::info("dataTimeSeries : {} x {}", dims[0], dims[1]);


    auto nd_array = std::vector<double>(dims[0] * dims[1]);
    time_series.read_raw<double>(nd_array.data());

    using Map_RM = Eigen::Map<const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>;
    channel_data = Map_RM(nd_array.data(), dims[0], dims[1]).transpose();

    Print();
}

SNIRF::~SNIRF()
{

}

void SNIRF::Print()
{

	spdlog::info("channel_data : ({}, {})", channel_data.rows(), channel_data.cols());

}

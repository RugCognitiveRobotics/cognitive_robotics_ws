/**
 * @addtogroup colormap_utils
 * @file 
 * @brief header file for colormap class
 *@{
 */
#ifndef _colormap_utils_H_
#define _colormap_utils_H_

//####################################################################
// Includes:
//####################################################################
#include <ros/ros.h>
#include <sensor_msgs/PointCloud2.h>
#include <sensor_msgs/PointCloud.h>
#include <visualization_msgs/Marker.h>
#include <tf/transform_broadcaster.h>      

#include <opencv2/core/core.hpp>


class class_colormap_utils
{
	public:
		class_colormap_utils(std::string name, int total, float alfa, bool reverse=false);
		~class_colormap_utils(void);

		std_msgs::ColorRGBA color(int i);
		cv::Scalar cv_color(int i);
		
	private:
		std::vector<std_msgs::ColorRGBA> cm;
		int max_index;



	int setup_colormap(int total, float alfa, bool reverse, float* r, float* g, float* b);

	int init_colormap_jet(int total, float alfa, bool reverse);
	int init_colormap_hsv(int total, float alfa, bool reverse);
	int init_colormap_hot(int total, float alfa, bool reverse);
	int init_colormap_cool(int total, float alfa, bool reverse);
	int init_colormap_spring(int total, float alfa, bool reverse);
	int init_colormap_summer(int total, float alfa, bool reverse);
	int init_colormap_autumn(int total, float alfa, bool reverse);
	int init_colormap_winter(int total, float alfa, bool reverse);
	int init_colormap_gray(int total, float alfa, bool reverse);
	int init_colormap_bone(int total, float alfa, bool reverse);
	int init_colormap_copper(int total, float alfa, bool reverse);
	int init_colormap_pink(int total, float alfa, bool reverse);
	int init_colormap_lines(int total, float alfa, bool reverse);
};

#endif
/**
 *@}
*/

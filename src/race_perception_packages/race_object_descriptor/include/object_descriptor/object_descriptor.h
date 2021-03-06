#ifndef _OBJECT_DESCRIPTOR_H_
#define _OBJECT_DESCRIPTOR_H_


//roslaunch race_feature_extraction test_pdbnodelet_node.launch
//Emulate race_object_tracking by : "rosrun race_feature_extraction test_feature_extraction"

/* _________________________________
   |                                 |
   |           INCLUDES              |
   |_________________________________| */

//system includes
#include <stdio.h>
#include <stdlib.h>

//ROS includes
#include <ros/ros.h>
#include <sensor_msgs/PointCloud2.h>
#include <std_msgs/String.h>
#include <nodelet/nodelet.h>
#include <pluginlib/class_list_macros.h>   
#include <pcl/filters/uniform_sampling.h>
#include <pcl/visualization/cloud_viewer.h>
#include <ros/package.h>

//package includes
#include <object_descriptor/object_descriptor_functionality.h>
#include <race_perception_msgs/perception_msgs.h>
#include <race_perception_msgs/CompleteRTOV.h>

//Eigen includes
#include <Eigen/Core>

//PCL includes
#include <pcl/io/pcd_io.h>
#include <pcl/features/normal_3d.h>
#include <pcl/features/normal_3d_omp.h>
#include <pcl/filters/voxel_grid.h>
#include <CGAL/Plane_3.h>
#include <pcl/filters/uniform_sampling.h>
#include <pcl_conversions/pcl_conversions.h>
#include <pcl/point_types.h>
#include <pcl/common/common_headers.h>

////new added
#include <pcl/tracking/kld_adaptive_particle_filter_omp.h>
#include <pcl/tracking/distance_coherence.h>
#include <pcl/tracking/hsv_color_coherence.h>
#include <pcl/tracking/approx_nearest_pair_point_cloud_coherence.h>
#include <pcl/tracking/nearest_pair_point_cloud_coherence.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/common/centroid.h>
#include <pcl/common/pca.h>
#include <pcl/filters/passthrough.h>
#include <pcl/filters/project_inliers.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/common/transforms.h>
#include <pcl/filters/conditional_removal.h>
#include <pcl/io/pcd_io.h>
//new includes from preprocessing
//ROS includes
#include <ros/ros.h>
//#include "pcl_ros/impl/transforms.hpp"
#include <pcl/conversions.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <sensor_msgs/PointCloud2.h>
#include <pcl/filters/conditional_removal.h>
#include <pcl/sample_consensus/ransac.h>
#include <pcl/sample_consensus/sac_model_plane.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/ModelCoefficients.h>
#include <pcl/io/pcd_io.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/kdtree/kdtree.h>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/segmentation/extract_clusters.h>
#include <pcl/filters/project_inliers.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/surface/convex_hull.h>
#include <pcl/segmentation/extract_polygonal_prism_data.h>
//#include <pcl/common/impl/transforms.hpp>
#include <pcl/common/transforms.h>
#include <tf/transform_broadcaster.h>
#include <tf/transform_listener.h>
#include <tf_conversions/tf_eigen.h>
//#include "/opt/ros/fuerte/stacks/geometry/tf_conversions/include/tf_conversions/tf_eigen.h"
#include <visualization_msgs/Marker.h>
#include <pcl/segmentation/conditional_euclidean_clustering.h>

#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>
#include <race_perception_utils/print.h>
#include <pcl_conversions/pcl_conversions.h>


//raceua includes
#include <race_perception_db/perception_db.h>
#include <race_perception_db/perception_db_serializer.h>
#include <race_perception_db/MsgTest.h>
#include <race_perception_utils/cycle.h>
#include <race_perception_utils/print.h>

#include <race_perception_msgs/TOVI.h>

#include <colormap_utils/colormap_utils.h>


/* _________________________________
   |                                 |
   |        Class definition         |
   |_________________________________| */

namespace ros {class Publisher;}

/* _________________________________
|                                 |
|            NameSpace            |
|_________________________________| */

using namespace std;
using namespace pcl;
using namespace ros;
using namespace race_perception_db;
using namespace race_perception_msgs;
using namespace race_perception_utils;
using namespace tf;


/* _________________________________
|                                 |
|        Global Parameters        |
|_________________________________| */

//spin images parameters
int    spin_image_width = 8 ;
double spin_image_support_lenght = 0.1;
int    subsample_spinimages = 10;
double uniform_sampling_size = 0.06;

int  off_line_flag  = 1;
int  number_of_bins = 5;
int  adaptive_support_lenght = 0;
double global_image_width =0.5;
    
/* _________________________________
   |                                 |
   |        GLOBAL VARIABLES         |
   |_________________________________| */
            
//
// typedef pcl::PointXYZRGB PointT;
typedef PointCloud<PointT> PointCloudT;
typedef boost::shared_ptr<PointCloudT> PointCloudPtrT;



  
bool signDisambiguationFlag = false;
int sign = 1;
int threshold = 150;	 
boost::shared_ptr<tf::TransformListener> _p_transform_listener;


PointCloudPtrT cloud_reference;
PointCloudPtrT initial_cloud_ref;

boost::shared_ptr<TransformBroadcaster> _br; //a transform broadcaster


int AddingGaussianNoise (boost::shared_ptr<PointCloud<PointT> > input_pc, 
			 double standard_deviation,
			 boost::shared_ptr<PointCloud<PointT> > output_pc)

{
      ROS_INFO ("Adding Gaussian noise with mean 0.0 and standard deviation %f\n", standard_deviation);

      *output_pc = *input_pc;

      boost::mt19937 rng; rng.seed (static_cast<unsigned int> (time (0)));
      boost::normal_distribution<> nd (0, standard_deviation);
      boost::variate_generator<boost::mt19937&, boost::normal_distribution<> > var_nor (rng, nd);

      for (size_t point_i = 0; point_i < input_pc->points.size (); ++point_i)
      {
	output_pc->points[point_i].x = input_pc->points[point_i].x + static_cast<float> (var_nor ());
	output_pc->points[point_i].y = input_pc->points[point_i].y + static_cast<float> (var_nor ());
	output_pc->points[point_i].z = input_pc->points[point_i].z + static_cast<float> (var_nor ());
      }

    return 0;
 
}
int downSampling ( boost::shared_ptr<PointCloud<PointT> > cloud, 		
		  float downsampling_voxel_size, 
		  boost::shared_ptr<PointCloud<PointT> > downsampled_pc)
{
  
	// Downsample the input point cloud using downsampling voxel size
	PointCloud<int> sampled_indices;
	UniformSampling<PointT> uniform_sampling;
	uniform_sampling.setInputCloud (cloud);
	uniform_sampling.setRadiusSearch (downsampling_voxel_size/*0.01f*/);
	//uniform_sampling.compute (sampled_indices);////PCL 1.7;
	//copyPointCloud (*cloud, sampled_indices.points, *downsampled_pc);////PCL 1.7;

	uniform_sampling.filter (*downsampled_pc);//PCL 1.8;
	
  return 0;
}


namespace race_object_descriptor
{
    template <class PointT>
        class ObjectDescriptor: public nodelet::Nodelet 
    {
        public:
            //Type defs

            //local variables
            string _name;
            bool _verb;
            ros::NodeHandle* _p_nh; // The pointer to the node handle
            ros::NodeHandle _nh; // The node handle
            ros::NodeHandle _priv_nh; // The node handle
            ros::NodeHandle* _p_priv_nh; // The node handle
            bool _flg_is_nodelet; //a flag to check if this code is running as a node or a nodelet
            boost::shared_ptr<ros::Subscriber> _p_pcin_subscriber;
            boost::shared_ptr<ros::Subscriber> _p_pcin_subscriber2;

            //boost::shared_ptr<ros::Publisher> _p_pcin_publisher;
            boost::shared_ptr<ros::Publisher> _p_crtov_publisher;
            boost::shared_ptr<ros::Publisher> _p_projected_object_point_cloud_to_table_publisher;
            boost::shared_ptr<ros::Publisher> _p_projected_object_point_cloud_to_table_publisher2;

            boost::shared_ptr<class_colormap_utils> cm; //a colormap class
            
            Publisher marker_publisher;
            Publisher neat_marker_publisher;

            std::string _id_name;
            PerceptionDB* _pdb; //initialize the class
            boost::shared_ptr<CycleDebug> cd;

            tf::StampedTransform stf;

            tf::StampedTransform transf_object;


            /* _________________________________
               |                                 |
               |           PARAMETERS			|
               |_________________________________| */

            //double _param1;
            //string _param2;

            /* _________________________________
               |                                 |
               |           CONSTRUCTORS          |
               |_________________________________| */

            ObjectDescriptor(){_flg_is_nodelet = true;};

            ObjectDescriptor(ros::NodeHandle* n)
            {
                _flg_is_nodelet = false; 
                _p_nh = n; //if this is a node set both the nodehandle and private node handle to n
                _p_priv_nh = n; 
                onInit();
            };

            /**
             * @brief Destructor
             */
            ~ObjectDescriptor()
            {
                PrettyPrint pp(_name);
                pp.info(std::ostringstream().flush() << _name.c_str() << ": Destructor called");
                pp.info(std::ostringstream().flush() << _name.c_str() << ": Finished destructor");
                pp.printCallback();


            };

	    /* _________________________________
               |                                 |
               |           CLASS METHODS         |
               |_________________________________| */

 	    /* _________________________________
               |                                 |
               |           CLASS METHODS         |
               |_________________________________| */

            void onInit(void)
            {
                //create a node handle in internal nh_ variable, and point p_nh_
                //to it. Only done if we are using a nodelet.
                if (_flg_is_nodelet == true)
                {
                    _nh = getNodeHandle(); 
                    _p_nh = &_nh;
                    _priv_nh = getPrivateNodeHandle(); 
                    _p_priv_nh = &_priv_nh;
                }

                /////////////////////////////////
                /* ______________________________
                   |                             |
                   |  working area for grasping  |
                   |_____________________________| */
		
		//init listener
		_p_transform_listener = (boost::shared_ptr<tf::TransformListener>) new tf::TransformListener;
		ros::Duration(0.5).sleep();
		ROS_INFO(" a tf lisener has been created" )  ;	
		//init broadcaster
		_br = (boost::shared_ptr<TransformBroadcaster>) new TransformBroadcaster;

		ROS_INFO(" a tf broadcaster has been created" )  ;	

                _id_name = "_ObjID_";
                //Initialize tf stuff

                //initialize parameters
                _name = _p_priv_nh->getNamespace();

                PrettyPrint pp(_name);

                _p_priv_nh->param<bool>("verbose", _verb , false);
                

		//read spin images parameters
		_p_priv_nh->param<int>("/perception/spin_image_width", spin_image_width, spin_image_width);
		_p_priv_nh->param<double>("/perception/spin_image_support_lenght", spin_image_support_lenght, spin_image_support_lenght);
		_p_priv_nh->param<int>("/perception/subsample_spinimages", subsample_spinimages, subsample_spinimages);
		_p_priv_nh->param<double>("/perception/uniform_sampling_size", uniform_sampling_size, uniform_sampling_size);
	
		//new object descriptor parameters
		_p_priv_nh->param<int>("/perception/number_of_bins", number_of_bins, number_of_bins);
		_p_priv_nh->param<double>("/perception/global_image_width", global_image_width, global_image_width);
		_p_priv_nh->param<int>("/perception/adaptive_support_lenght", adaptive_support_lenght, adaptive_support_lenght);


		signDisambiguationFlag = false;
		
		//Create a new colormap	
		cm = (boost::shared_ptr<class_colormap_utils>) new class_colormap_utils(std::string("hot"),5, 1, false);
		
                //create a cycle debug
                cd = (boost::shared_ptr<CycleDebug>) new CycleDebug(_p_nh, _name);

                //initialize the subscriber
                //topic name is computed by getting the namespace path and then adding
		unsigned found = _name.find_last_of("/\\");
                std::string pcin_topic = _name.substr(0,found) + "/tracker/tracked_object_point_cloud";
		ROS_INFO("pcin");
                _p_pcin_subscriber = (boost::shared_ptr<ros::Subscriber>) new ros::Subscriber;
                *_p_pcin_subscriber = _p_nh->subscribe (pcin_topic, 1, &ObjectDescriptor::callback, this);

                //initialize the Publisher
		_p_crtov_publisher = (boost::shared_ptr<ros::Publisher>) new ros::Publisher;
                *_p_crtov_publisher = _p_priv_nh->advertise<race_perception_msgs::CompleteRTOV> ("histogram_tracked_object_view", 1000);
		
		//initialize the publishers
		neat_marker_publisher = _p_nh->advertise<visualization_msgs::MarkerArray>("/perception/object_descriptor/neat_markers", 100);
			
		
		_p_crtov_publisher = (boost::shared_ptr<ros::Publisher>) new ros::Publisher;
                *_p_crtov_publisher = _p_priv_nh->advertise<race_perception_msgs::CompleteRTOV> ("new_histogram_tracked_object_view", 100);

		_p_projected_object_point_cloud_to_table_publisher = (boost::shared_ptr<ros::Publisher>) new ros::Publisher;
		*_p_projected_object_point_cloud_to_table_publisher = _p_nh->advertise<sensor_msgs::PointCloud2>("/projected_object_point_clouds", 100);
	
		_p_projected_object_point_cloud_to_table_publisher2 = (boost::shared_ptr<ros::Publisher>) new ros::Publisher;
		*_p_projected_object_point_cloud_to_table_publisher2 = _p_nh->advertise<sensor_msgs::PointCloud2>("/projected_object_point_clouds_after_transform", 100);
	
		
                //initialize the database
                _pdb = race_perception_db::PerceptionDB::getPerceptionDB(_p_priv_nh, _flg_is_nodelet); //initialize the database class

                //Output initialization information
                pp.printInitialization();				

            };

	    
            /**
             * @brief This is the callback.
             * @param msg
             */
            void callback(const race_perception_msgs::PCTOV::ConstPtr& msg)
            {
	      //NOTE : there is a bug in global one 
	      
	      //std::reverse(myvector.begin(),myvector.end());
                cd->tic();//cycle debug tic
                PrettyPrint pp(_name);
		ROS_INFO("//////////////////////////////// TID = %i ///////////////////////////////////////////", msg-> track_id);


		ROS_INFO("//////////////////////////////// dimensions (%2.2f, %2.2f, %2.2f)  ///////////////////////////////////////////", 
			  msg-> dimensions.x , msg-> dimensions.y, msg-> dimensions.z	);
		ROS_INFO("//////////////////////////////// pose of the object = %2.2f, %2.2f, %2.2f ///////////////////////////////////////////", 
			  msg-> pose_stamped.pose.position.x, msg-> pose_stamped.pose.position.y, msg-> pose_stamped.pose.position.z	);

		
		//read spin images parameters
		_p_priv_nh->param<int>("/perception/spin_image_width", spin_image_width, spin_image_width);
		_p_priv_nh->param<double>("/perception/spin_image_support_lenght", spin_image_support_lenght, spin_image_support_lenght);
		_p_priv_nh->param<int>("/perception/subsample_spinimages", subsample_spinimages, subsample_spinimages);
        _p_priv_nh->param<double>("/perception/uniform_sampling_size", uniform_sampling_size, uniform_sampling_size);



		//parameters of new object descriptor
		_p_priv_nh->param<int>("/perception/number_of_bins", number_of_bins, number_of_bins);
		_p_priv_nh->param<double>("/perception/global_image_width", global_image_width, global_image_width);		
		_p_priv_nh->param<int>("/perception/adaptive_support_lenght", adaptive_support_lenght, adaptive_support_lenght);
		_p_priv_nh->param<int>("/perception/sign", sign, sign);
		_p_priv_nh->param<int>("/perception/off_line_flag", off_line_flag, off_line_flag);
		//TODO threshold must be a parametes 
		
		pp.info(std::ostringstream().flush()<<"\t\t[-] spin_image_width :"<< spin_image_width);
		pp.info(std::ostringstream().flush()<<"\t\t[-] spin_image_support_lenght :"<< spin_image_support_lenght);
		pp.info(std::ostringstream().flush()<<"\t\t[-] subsample_spinimages :"<< subsample_spinimages);
		
		//initializ 3 plane histogram for the given object
						
		ros::Time beginProc = ros::Time::now(); //start tic
		
		//Declare a boost share ptr to the pointCloud
                boost::shared_ptr<PointCloud<PointT> > target_pc (new PointCloud<PointT>); 
                pcl::fromROSMsg(msg->point_cloud,*target_pc ); //Convert the pointcloud msg to a pcl point cloud
		
		ROS_INFO("given point cloud has %d points ", target_pc->points.size());
		
	
	      //NOTE : Rotation matrix can be formed in several ways. 
	      // One way is to get it from rotation axis and rotation angle:
	      // 
	      // 1 - Get the plane coefficients of your ground plane from RANSAC. First three coefficients correspond to your ground planes normal (table). 
	      // 2 - Generate the normal vector for your desired plane. If it is xy plane, since in xy plane z is 0, its normal is x=0,y=0 and z=1. 
	      // 3 - Calculate cross product of normal of ground plane and normal of xy plane to get rotation vector (axis) which is a unit vector. 
	      // 4 - Calculate the rotation angle. Angle between the planes is equal to angle between the normals. From the definition of the dot product, 
	      //     you can extract the angle between two normal vectors. In case of XY plane, it is equal to theta=arccos(C/sqrt(A^2+B^2+C^2) where A, B, C are first three coefficients of ground plane. 
	      // 5 - You now have rotation vector and rotation angle. You can generate the rotation matrix (3x3) or quaternion. Look for the formula in Wikipedia. 
	      // 6 - Generate the complete transformation matrix. If you do not perform any translation or scaling, your third row and column will have zero except (4,4) entry which is 1. 
	      // 7 - Apply the transformation simply with transformPointCloud(cloud,transformed,transformationMatrix) 
				
		 /* ______________________________________________________________
		|                     				       	    |
		|  compute projection of the given object to three main axes   |
		|______________________________________________________________| */

		string _fixed_frame_id_tmp = "/odom_combined";
		string _table_frame_id = "/perception/tabletop_segmentation/table";
		
		std::string tracker_frame_id = "/perception/pipeline" + boost::lexical_cast<std::string>(msg->track_id) + "/tracker";
// 		std::string tracker_frame_id = "/perception/pipeline" + boost::lexical_cast<std::string>(7) + "/tracker";

		      
// 		std::string _arm_frame_id = "/perception/tabletop_segmentation/arm";
// 		transf_arm.setOrigin( tf::Vector3(transf_.getOrigin().x()+ arm_reference_frame_offset_x_, 
// 						    transf_.getOrigin().y()+ arm_reference_frame_offset_y_,
// 						    transf_.getOrigin().z()+ arm_reference_frame_offset_z_) );
// 
		
		
		
		/* ___________________
		|                     |
		|  Offline descriptor |
		|_____________________| */
		
		// transf_object.setOrigin( tf::Vector3(0, 0, 0) );
		// tf::Quaternion rotation;
		// //rotation.setRPY(0, 0, 1.57);
		// rotation.setRPY(0, 0, 0);	
		// transf_object.setRotation(rotation);
		
		// _br->sendTransform(StampedTransform(transf_object, Time::now(), "/odom_combined", tracker_frame_id));

		
		if (1)
		{
			boost::shared_ptr<pcl::PointCloud<T> > pca_object_view (new PointCloud<PointT>);
			boost::shared_ptr<PointCloud<PointT> > pca_pc (new PointCloud<PointT>); 
			vector < boost::shared_ptr<pcl::PointCloud<PointT> > > vector_of_projected_views;
			double largest_side = 0;
			int  sign = 1;
			vector <float> view_point_entropy;
			string std_name_of_sorted_projected_plane;
			vector< float > object_description;
			vector< float > object_description1Dvariance;

			Eigen::Vector3f center_of_bbox ;
			center_of_bbox(0,0) = msg->dimensions.x; center_of_bbox(0,1) = msg->dimensions.y;center_of_bbox(0,2) = msg->dimensions.z;
			
			
			
			///offline //TODO : put a parameter for online and offline
			// compuet_object_description( target_pc,
			// 							adaptive_support_lenght,
			// 							global_image_width,
			// 							threshold,
			// 							number_of_bins,
			// 							pca_object_view,
			// 							center_of_bbox,
			// 							vector_of_projected_views, 
			// 							largest_side, 
			// 							sign,
			// 							view_point_entropy,
			// 							std_name_of_sorted_projected_plane,
			// 							object_description
			// 						);

			// set_neat_visualization_marker_array_object_descriptor_vector_offline( target_pc,
			// 																	/*output_pc*/pca_object_view,
			// 																	center_of_bbox,
			// 																	vector_of_projected_views,
			// 																	msg->header.frame_id,
			// 																	msg -> track_id, 
			// 																	largest_side, 
			// 																	sign,
			// 																	view_point_entropy,
			// 																	std_name_of_sorted_projected_plane
			// 																	);	


			
			///real_demo //TODO : put a parameter for online and offline
			compuet_object_description_real_demo (  target_pc,
													adaptive_support_lenght,
													global_image_width,
													threshold,
													number_of_bins,
													msg->dimensions,
													vector_of_projected_views, 
													largest_side, 
													sign,
													view_point_entropy,
													std_name_of_sorted_projected_plane,
													object_description );

			ROS_INFO("**********************************************************************************************************");
			ROS_INFO("**********************************************************************************************************");
			ROS_INFO("**********************************************************************************************************");
			ROS_INFO("**********************************************************************************************************");
			ROS_INFO("**********************************************************************************************************");
			ROS_INFO("**********************************************************************************************************");
			ROS_INFO("**********************************************************************************************************");
			set_neat_visualization_marker_array_object_descriptor_vector(   target_pc,
																			vector_of_projected_views,
																			msg->header.frame_id,
																			msg -> track_id, 
																			largest_side, 
																			sign,
																			view_point_entropy,
																			std_name_of_sorted_projected_plane );	
		

		
			/// NOTE: for ModelNet dataset, since point clouds are algined, we do not need to perform PCA
			/// So we use, real_demo function and offline visualization
			// compuet_object_description_real_demo(   target_pc,
			// 					adaptive_support_lenght,
			// 					global_image_width,
			// 					threshold,
			// 					number_of_bins,
			// 					msg->dimensions,
			// 					vector_of_projected_views, 
			// 					largest_side, 
			// 					sign,
			// 					view_point_entropy,
			// 					std_name_of_sorted_projected_plane,
			// 					object_description )	;

			// /// NOTE: for ModelNet dataset, since point clouds are algined, we do not need to perform PCA
			// /// we input target_pc as pca_object_view
			// set_neat_visualization_marker_array_object_descriptor_vector_offline( target_pc,
			// 															/*pca_object_view*/target_pc /* special case for modelNet*/ ,
			// 															center_of_bbox,
			// 															vector_of_projected_views,
			// 															msg->header.frame_id,
			// 															msg -> track_id, 
			// 															largest_side, 
			// 															sign,
			// 															view_point_entropy,
			// 															std_name_of_sorted_projected_plane
			// 															);	
		
		
		
		
		
		// set_neat_visualization_marker_array_object_descriptor_vector(   target_pc,
		// 																vector_of_projected_views,
		// 																msg->header.frame_id,
		// 																msg -> track_id, 
		// 																largest_side, 
		// 																sign,
		// 																view_point_entropy,
		// 																std_name_of_sorted_projected_plane
		// 																);	

			// // 			  
// 		      cout<< "\n GOOD description for the given object = [ ";
// 		      SITOV object_representation;
// 		      for (size_t i = 0; i < object_description.size(); i++)
// 		      {
// 			  object_representation.spin_image.push_back(object_description.at(i));
// 			  cout<< object_description.at(i)<<",";
// 		      }
// 		      cout<< "];\n";
			
		 
		      
// 		    #define SORT_BY_ENTROPY_AND_1D_VARIANCE			0
// 		    #define SORT_BY_ENTROPY_AND_2D_STDDEV_SUM			1
// 		    #define SORT_BY_DISTINCTIVENESS				2
// 		    #define SORT_BY_ENTROPY_AND_2D_VARIANCE_SUM		3
// 		    #define SORT_BY_ENTROPY_AND_AVERAGE_DISTANCE		4 
/*		   compuet_object_description_2D_variance( target_pc,
					      adaptive_support_lenght,
					      global_image_width,
					      threshold,
					      number_of_bins,
					      pca_object_view,
					      center_of_bbox,
					      vector_of_projected_views, 
					      largest_side, 
					      sign,
					      view_point_entropy,
					      std_name_of_sorted_projected_plane,
					      object_description, 
					      SORT_BY_ENTROPY_AND_2D_VARIANCE_SUM); */  
		      
// 		      cout<< "\n GOOD description based on 2D variance for the given object = [ ";
		      SITOV object_representation;
		      for (size_t i = 0; i < object_description.size(); i++)
		      {
			  object_representation.spin_image.push_back(object_description.at(i));
// 			  cout<< object_description.at(i)<<",";
		      }
// 		      cout<< "];\n";
// 		      
		      
		      
		/* _____________________________________________
        |                                            |
        |  Write features to DB based on TID and VID |
        |____________________________________________| */

                beginProc = ros::Time::now();

                //Declare SITOV (Spin Images of Tracked Object View)
                SITOV _sitov;

                //Declare RTOV (Representation of Tracked Object View)
                RTOV _rtov;
                _rtov.track_id = msg->track_id;
                _rtov.view_id = msg->view_id;

		ROS_INFO( "Track_id = %d,\tView_id = %d",  msg->track_id, msg->view_id );
		
		
                //declare the RTOV complete variable
                race_perception_msgs::CompleteRTOV _crtov;
                _crtov.track_id = msg->track_id;
                _crtov.view_id = msg->view_id;
                _crtov.ground_truth_name = msg->ground_truth_name.c_str();
		_crtov.pose_stamped = msg -> pose_stamped;
		_crtov.dimensions = msg -> dimensions;
		pp.info(std::ostringstream().flush() << "object_pose.x = %f " << msg -> pose_stamped.pose.position.x);
		pp.info(std::ostringstream().flush() << "object_pose.y = %f " << msg -> pose_stamped.pose.position.y);
		pp.info(std::ostringstream().flush() << "object_pose.z = %f " << msg -> pose_stamped.pose.position.z);

		ROS_INFO("ground_truth_name = %s", msg->ground_truth_name.c_str());

                //Add the object view representation in msg_out to put in the DB
		_sitov = object_representation; //copy spin images
		_sitov.track_id = msg->track_id; //copy track_id
		_sitov.view_id = msg->view_id; //copy view_id
		_sitov.spin_img_id = 1; //copy spin image id

		//Addd sitov to completertov sitov list
		_crtov.sitov.push_back(_sitov);
        _crtov.is_key_view = msg->is_key_view;
		if (msg->is_key_view) //add sitovs to the DB only if this is a key view
		{
		    //Serialize to add to DB
		    uint32_t serial_size = ros::serialization::serializationLength(_sitov);
		    boost::shared_array<uint8_t> buffer(new uint8_t[serial_size]);
		    PerceptionDBSerializer<boost::shared_array<uint8_t>, SITOV>::serialize(buffer, _sitov, serial_size);	
		    leveldb::Slice s((char*)buffer.get(), serial_size);
		    std::string key = _pdb->makeSIKey(key::SI, msg->track_id, msg->view_id, 1 );

		    //Put slice to the DB
		    _pdb->put(key, s); 

		    //Add to the list of SITOV keys for this RTOV
		    _rtov.sitov_keys.push_back(key);
		    buffer.reset();
		}

        //Add RTOV to the DB (only if this is a key view)
        if (msg->is_key_view)                 
        {
            uint32_t serial_size = ros::serialization::serializationLength(_rtov);
            boost::shared_array<uint8_t> buffer(new uint8_t[serial_size]);
            PerceptionDBSerializer<boost::shared_array<uint8_t>, RTOV>::serialize(buffer, _rtov, serial_size);	
            leveldb::Slice s((char*)buffer.get(), serial_size);
            std::string key = _pdb->makeKey(key::RV, msg->track_id, msg->view_id);

            //Put slice to the db
            _pdb->put(key, s);
            buffer.reset();
        }

        //Publish the CompleteRTOV to recognition
        _p_crtov_publisher->publish (_crtov);

        //Toc
        ros::Duration duration = (ros::Time::now() - beginProc);
        double duration_sec = duration.toSec();
        pp.info(std::ostringstream().flush() << "Write the features to DB took " << duration_sec << " secs");
		
					
		//add guassian noise 
// 		double standard_deviation=0.003;
// 		boost::shared_ptr<PointCloud<PointT> > output_pc (new PointCloud<PointT>);
// 		
// 		AddingGaussianNoise (	target_pc, 
// 					standard_deviation,
// 					output_pc);
// 			      
		
		
		//downSampling
// 		float downsampling_voxel_size= 0.01;
// 		boost::shared_ptr<PointCloud<PointT> > output_pc (new PointCloud<PointT>);
// 		downSampling ( target_pc, 		
// 				downsampling_voxel_size,
// 				output_pc);	
    
// 		set_neat_visualization_marker_array_object_descriptor_vector_offline( target_pc,
// 											/*output_pc*/pca_object_view,
// 											center_of_bbox,
// 											vector_of_projected_views,
// 											msg->header.frame_id,
// 											msg -> track_id, 
// 											largest_side, 
// 											sign,
// 											view_point_entropy,
// 											std_name_of_sorted_projected_plane
// 											);	
		
		//ROS_INFO("before visualization dimension of largest_side =  %f ", largest_side);

		// set_neat_visualization_marker_array_object_descriptor_vector(   target_pc,
		// 																vector_of_projected_views,
		// 																msg->header.frame_id,
		// 																msg -> track_id, 
		// 																largest_side, 
		// 																sign,
		// 																view_point_entropy,
		// 																std_name_of_sorted_projected_plane );	
    
        pp.printCallback();

// 		
// 		set_neat_visualization_marker_array_object_descriptor( vector_of_projected_views,
// 									   msg->header.frame_id,
// 									    msg -> track_id);

		}
		
		
		/* ________________________________________________________
		 |    				    	                   |
		|  compute bounding box dimensions fo the given object  |
		|_______________________________________________________| */

// 		beginProc = ros::Time::now();
// 		//initial_cloud_ref = cloud_reference;
// 		geometry_msgs::Vector3 dimensions;
// 		compute_bounding_box_dimensions(target_pc, dimensions);
// 		ROS_INFO("box dimensions (x, y, z) = (%f, %f, %f) ", dimensions.x, dimensions.y, dimensions.z);
// 		
//                 //get toc
//                 duration = ros::Time::now() - beginProc;
//                 duration_sec = duration.toSec();
//                 ROS_INFO("Compute bounding box for given object took %f secs", duration_sec );

            }


            /* _________________________________
               |                                 |
               |           ACCESSORS             |
               |_________________________________| */
	    
	    template <typename T>
	    int set_neat_visualization_marker_array_object_descriptor_vector_offline( boost::shared_ptr<pcl::PointCloud<T> > object_view,
											boost::shared_ptr<pcl::PointCloud<T> > pca_object_view,
											Eigen::Vector3f center_of_bbox,
											vector < boost::shared_ptr<pcl::PointCloud<T> > >  all_projected_views, 
											string object_frame_id, 
											unsigned int TID , 
											double largest_side, 
											int sign,
											vector <float> view_point_entropy,
											string std_name_of_sorted_projected_plane )
	    {


    
	      
		std::string tracker_frame_id = "/perception/pipeline" + boost::lexical_cast<std::string>(TID) + "/tracker";
		ROS_INFO("tracker_frame_id = %s", tracker_frame_id.c_str());
		visualization_msgs::MarkerArray marker_array; 
		//geometry_msgs::Point p;
		double duration =1000;
		bool locked = true;
		bool finish= true;
		//float center_of_projected_cloud_x=0 , center_of_projected_cloud_y=0 , center_of_projected_cloud_z=0 ;
		//geometry_msgs::Point center_of_projected_cloud;
		//geometry_msgs::Vector3 dimensions;
		//geometry_msgs::Point p;
		// int view_i = 1;
		
		
		
		//////////////////////////// working with a dataset /////////////////////////////
		
		geometry_msgs::Point center_of_boundingbox_cloud;
// 		center_of_boundingbox_cloud.x=center_of_bbox(0,0);
// 		center_of_boundingbox_cloud.y=center_of_bbox(0,1);
// 		center_of_boundingbox_cloud.z=center_of_bbox(0,2);

			
		geometry_msgs::Point center_of_gravity_cloud;
		geometry_msgs::Vector3 dimensions;
		geometry_msgs::Point p;
		float center_of_gravity_cloud_x=0 , center_of_gravity_cloud_y=0 , center_of_gravity_cloud_z=0 ;
							
		compute_bounding_box_dimensions(pca_object_view, dimensions);
		center_of_boundingbox_cloud.x=dimensions.x/2;
		center_of_boundingbox_cloud.y=dimensions.y/2;
		center_of_boundingbox_cloud.z=dimensions.z/2;
		/* ____________________________________
		|                                     |
		|   Draw  PCA Point Cloud of object   |
		|_____________________________________| */
	      
		if (1)
		{
		      visualization_msgs::Marker marker;
		      //marker.header.frame_id = object_frame_id;
		      marker.header.stamp = ros::Time();		
		      marker.frame_locked = locked;
		      marker.header.frame_id = tracker_frame_id;

		      marker.ns = "pca object view";
		      marker.id = TID*20;
		      marker.type = visualization_msgs::Marker::POINTS;
		      marker.lifetime = Duration(duration);
		      marker.action = visualization_msgs::Marker::ADD;
		      marker.pose.position.x = 0;	marker.pose.position.y = 0;	marker.pose.position.z = 0;
		      marker.pose.orientation.x = 0.0; marker.pose.orientation.y = 0.0; marker.pose.orientation.z = 0.0; marker.pose.orientation.w = 1.0;
		      marker.color.r = 0.5;  marker.color.g = 0.5; marker.color.b = 0.5; 	marker.color.a = 0.8;
		      marker.color.a = 1;
		      marker.scale.x = 0.002; marker.scale.y = 0.002; marker.scale.z = 0.002; 
		      
		      for (size_t i=0; i< pca_object_view->points.size(); i++)
		      {
			  //added recently
// 			  p.x = sign * pca_object_view->points.at(i).x;
// 			  p.y = sign * pca_object_view->points.at(i).y;
// 			  
			  p.x = pca_object_view->points.at(i).x;
			  p.y = pca_object_view->points.at(i).y;
			  p.z = pca_object_view->points.at(i).z;

			  center_of_gravity_cloud_x += p.x;
			  center_of_gravity_cloud_y += p.y;
			  center_of_gravity_cloud_z += p.z;

			  marker.points.push_back(p);
		      }

		      center_of_gravity_cloud.x= center_of_gravity_cloud_x/object_view->points.size();
		      center_of_gravity_cloud.y= center_of_gravity_cloud_y/object_view->points.size();
		      center_of_gravity_cloud.z= center_of_gravity_cloud_z/object_view->points.size();
		      marker_array.markers.push_back(marker);
		  
		}    

		/* _________________________________
		|                                 |
		|   Draw  Point Cloud of object   |
		|_________________________________| */
	      
		if (1)
		{
	  
	    
		  visualization_msgs::Marker marker;
		  //marker.header.frame_id = object_frame_id;
		  marker.header.stamp = ros::Time();		
		  marker.frame_locked = locked;
		  marker.header.frame_id = tracker_frame_id;

		  marker.ns = "object view";
		  marker.id = TID*1000;
		  marker.type = visualization_msgs::Marker::POINTS;
		  marker.lifetime = Duration(duration);
		  marker.action = visualization_msgs::Marker::ADD;
		  marker.pose.position.x = 0;	marker.pose.position.y = 0;	marker.pose.position.z = 0;

		  marker.pose.orientation.x = 0.0; marker.pose.orientation.y = 0.0; marker.pose.orientation.z = 0.0; marker.pose.orientation.w = 1.0;

		  marker.color.r = 0.5;  marker.color.g = 0.5; marker.color.b = 0.0; 	marker.color.a = 1;
		  marker.scale.x = 0.002; marker.scale.y = 0.002; marker.scale.z = 0.002; marker.color.a = 1;
		  //marker.color.r = 1.0; marker.color.g = 1.0;	marker.color.b = 1.0; marker.color.a = 1.0; 
		  
		  for (size_t i=0; i< object_view->points.size(); i++)
		  {
		      p.x = object_view->points.at(i).x;
		      p.y = object_view->points.at(i).y;
		      p.z = object_view->points.at(i).z;
		      
		      marker.points.push_back(p);
		  }

		  marker_array.markers.push_back(marker);
		  
		  
			    
		}    
			    
			    
		
		 /* _________________________________
	      	   |                                 |
	      	   |          DRAW BB WIREFRAME      |
	      	   |_________________________________| */
	      	if (1)
	      	{
	      		visualization_msgs::Marker marker;
	      		marker.header.frame_id = tracker_frame_id;
	      		marker.header.stamp = ros::Time();
	      
	      		marker.ns = "wireframe_boundingbox";
	      		marker.id = TID*100;
	      		marker.frame_locked = locked;
	      		marker.type = visualization_msgs::Marker::LINE_LIST;
	      		//if (finish)
	      		//marker.action = visualization_msgs::Marker::DELETE;
	      		//else
	      		marker.action = visualization_msgs::Marker::ADD;
	      		marker.lifetime = Duration(duration);
	      
			marker.pose.position.x = 0;	marker.pose.position.y = 0;	marker.pose.position.z = 0;

// 		      
			
// 			marker.pose.position = center_of_boundingbox_cloud;
			marker.pose.orientation.x = 0.0; marker.pose.orientation.y = 0.0; marker.pose.orientation.z = 0.0; marker.pose.orientation.w = 1.0;
			    				
			
	      		marker.scale.x = 0.005; 
	      		double x = dimensions.x/2; 
	      		double y = dimensions.y/2; 
	      		double z = dimensions.z/2; 
	      
	      		marker.color = cm->color(TID);
	      		marker.color.r = 0.5;
	      		marker.color.g = 0.5;
	      		marker.color.b = 0.5;
			marker.color.a = 0.1;
	      		//marker
	      		if (finish)
	      		{
	      			marker.color.r = 0.1;
	      			marker.color.g = 0.1;
	      			marker.color.b = 0.1;
	      		}

	      		geometry_msgs::Point p;
	      		p.x =  x; p.y =  y; p.z =  z; marker.points.push_back(p);
	      		p.x = -x; p.y =  y; p.z =  z; marker.points.push_back(p);
	      		p.x =  x; p.y =  y; p.z = -z; marker.points.push_back(p);
	      		p.x = -x; p.y =  y; p.z = -z; marker.points.push_back(p);
	      		p.x =  x; p.y =  y; p.z = -z; marker.points.push_back(p);
	      		p.x =  x; p.y =  y; p.z =  z; marker.points.push_back(p);
	      		p.x = -x; p.y =  y; p.z = -z; marker.points.push_back(p);
	      		p.x = -x; p.y =  y; p.z =  z; marker.points.push_back(p);
	      
	      		p.x =  x; p.y =  -y; p.z =  z; marker.points.push_back(p);
	      		p.x = -x; p.y =  -y; p.z =  z; marker.points.push_back(p);
	      		p.x =  x; p.y =  -y; p.z = -z; marker.points.push_back(p);
	      		p.x = -x; p.y =  -y; p.z = -z; marker.points.push_back(p);
	      		p.x =  x; p.y =  -y; p.z = -z; marker.points.push_back(p);
	      		p.x =  x; p.y =  -y; p.z =  z; marker.points.push_back(p);
	      		p.x = -x; p.y =  -y; p.z = -z; marker.points.push_back(p);
	      		p.x = -x; p.y =  -y; p.z =  z; marker.points.push_back(p);
	      
	      		p.x =  x; p.y =  y; p.z =  z; marker.points.push_back(p);
	      		p.x =  x; p.y = -y; p.z =  z; marker.points.push_back(p);
	      		p.x =  x; p.y =  y; p.z = -z; marker.points.push_back(p);
	      		p.x =  x; p.y = -y; p.z = -z; marker.points.push_back(p);
	      
	      		p.x = -x; p.y =  y; p.z =  z; marker.points.push_back(p);
	      		p.x = -x; p.y = -y; p.z =  z; marker.points.push_back(p);
	      		p.x = -x; p.y =  y; p.z = -z; marker.points.push_back(p);
	      		p.x = -x; p.y = -y; p.z = -z; marker.points.push_back(p);
	      
	      		marker_array.markers.push_back(marker);
	      	}
	      
	      
	      	/* _________________________________
	      	   |                                 |
	      	   |             DRAW BBOX           |
	      	   |_________________________________| */
	      	if (1)
	      	{
	      		visualization_msgs::Marker marker;
	      		marker.header.frame_id = tracker_frame_id;
	      		marker.header.stamp = ros::Time();
	      
	      		marker.ns = "boundingbox";
	      		marker.id = TID*290;
	      		marker.type = visualization_msgs::Marker::CUBE;
	      		marker.frame_locked = locked;
	      		//if (finish)
	      		//marker.action = visualization_msgs::Marker::DELETE;
	      		//else
	      		marker.action = visualization_msgs::Marker::ADD;
	      		marker.lifetime = Duration(duration);
	      
// 			marker.pose.position = center_of_boundingbox_cloud;
// 			marker.pose.position = center_of_gravity_cloud;

			marker.pose.position.x = 0;	marker.pose.position.y = 0;	marker.pose.position.z = 0;

			marker.pose.orientation.x = 0.0; marker.pose.orientation.y = 0.0; marker.pose.orientation.z = 0.0; marker.pose.orientation.w = 1.0;
	      		
			marker.scale.x = dimensions.x; 
	      		marker.scale.y = dimensions.y; 
	      		marker.scale.z = dimensions.z; 
	      

			marker.color = cm->color(TID);
			marker.color.a = 0.2;
			
			if (finish)
	      		{
	      			marker.color.r = 0.9;
	      			marker.color.g = 0.0;
	      			marker.color.b = 0.0;
	      		}
	      
	      		marker_array.markers.push_back(marker);
	      	}
		
		
		
		
		for (int view_i = 0; view_i< all_projected_views.size(); view_i++)			
		{
			geometry_msgs::Point center_of_projected_cloud;
			geometry_msgs::Vector3 dimensions;
			geometry_msgs::Point p;
			float center_of_projected_cloud_x=0 , center_of_projected_cloud_y=0 , center_of_projected_cloud_z=0 ;
			/* _________________________________
			  |                                 |
			  |   Draw Projected Point Cloud    |
			  |_________________________________| */
			if (1)
			{
		    
		      
			    visualization_msgs::Marker marker;
			    //marker.header.frame_id = object_frame_id;
			    marker.header.stamp = ros::Time();		
			    marker.frame_locked = locked;
			    marker.header.frame_id = tracker_frame_id;

			    marker.ns = "projected views";
			    marker.id = TID*520+view_i;
			    marker.type = visualization_msgs::Marker::POINTS;
			    marker.lifetime = Duration(duration);
			    marker.action = visualization_msgs::Marker::ADD;
			    marker.pose.position.x = 0;	marker.pose.position.y = 0;	marker.pose.position.z = 0;
			    marker.pose.orientation.x = 0.0; marker.pose.orientation.y = 0.0; marker.pose.orientation.z = 0.0; marker.pose.orientation.w = 1.0;
			    marker.color = cm->color(TID);
			    marker.scale.x = 0.005; marker.scale.y = 0.005; marker.scale.z = 0.005; marker.color.a = 1;
			    //marker.color.r = 1.0; marker.color.g = 1.0;	marker.color.b = 1.0; marker.color.a = 1.0; 
			    
			    //marker.points.erase(marker.points.begin(), marker.points.end());
			    geometry_msgs::Point p;

			    for (size_t i=0; i<all_projected_views.at(view_i)-> points.size(); i++)
			    {
// 				p.x = sign * all_projected_views.at(view_i)->points.at(i).x;
// 				p.y = sign * all_projected_views.at(view_i)->points.at(i).y;
// 				p.z = all_projected_views.at(view_i)->points.at(i).z;
// 				marker.points.push_back(p);

				//all_projected_views.at(view_i)->points.at(i).x = sign * all_projected_views.at(view_i)->points.at(i).x;
				//all_projected_views.at(view_i)->points.at(i).y = sign * all_projected_views.at(view_i)->points.at(i).y;
				
				
// 				if (view_i == 0)
// 				{
// 				    all_projected_views.at(view_i)->points.at(i).x = all_projected_views.at(view_i)->points.at(i).x;
// 				    all_projected_views.at(view_i)->points.at(i).y = sign * all_projected_views.at(view_i)->points.at(i).y;
// 				    //all_projected_views.at(view_i)->points.at(i).x =  all_projected_views.at(view_i)->points.at(i).x;
// 				    //all_projected_views.at(view_i)->points.at(i).y =  all_projected_views.at(view_i)->points.at(i).y;
// 				}
// 				else if (view_i == 1)
// 				{
// 				    all_projected_views.at(view_i)->points.at(i).x = sign * all_projected_views.at(view_i)->points.at(i).x;
// 				    all_projected_views.at(view_i)->points.at(i).y = all_projected_views.at(view_i)->points.at(i).y;
// 				  
// 				}
// 				else
// 				{
// 				    all_projected_views.at(view_i)->points.at(i).x = sign * all_projected_views.at(view_i)->points.at(i).x;
// 				    all_projected_views.at(view_i)->points.at(i).y = sign * all_projected_views.at(view_i)->points.at(i).y;
// 				  
// 				}
    
				all_projected_views.at(view_i)->points.at(i).z = sign * all_projected_views.at(view_i)->points.at(i).z;
				p.x = all_projected_views.at(view_i)->points.at(i).x;
				p.y = all_projected_views.at(view_i)->points.at(i).y;
				//p.z = sign * all_projected_views.at(view_i)->points.at(i).z;
								
				p.z = all_projected_views.at(view_i)->points.at(i).z;

								
				marker.points.push_back(p);
				center_of_projected_cloud_x += p.x;
				center_of_projected_cloud_y += p.y;
				center_of_projected_cloud_z += p.z;
			    }

			    marker_array.markers.push_back(marker);
			    
			    center_of_projected_cloud.x= center_of_projected_cloud_x/all_projected_views.at(view_i)->points.size();
			    center_of_projected_cloud.y= center_of_projected_cloud_y/all_projected_views.at(view_i)->points.size();
			    center_of_projected_cloud.z= center_of_projected_cloud_z/all_projected_views.at(view_i)->points.size();
			    
			}
			
			  /* _________________________________
			    |                                 |
			    |           Draw Plane            |
			    |_________________________________| */
			  if (1)
			  {
				  visualization_msgs::Marker marker;
				  marker.header.frame_id = tracker_frame_id;
				  marker.header.stamp = ros::Time();
			
				  marker.ns = "Plane";
				  marker.id = TID*150+view_i;
				  marker.type = visualization_msgs::Marker::CUBE;
				  marker.frame_locked = locked;
				  //if (finish)
				  //marker.action = visualization_msgs::Marker::DELETE;
				  //else
				  marker.action = visualization_msgs::Marker::ADD;
				  marker.lifetime = Duration(duration);
			
				  marker.pose.position =center_of_projected_cloud;

				  //marker.pose.position.x = 0;	marker.pose.position.y = 0;	marker.pose.position.z = 0;

				  //compute_bounding_box_dimensions(all_projected_views.at(view_i), dimensions);
				  //ROS_INFO("Visuzlize box dimensions (x, y, z) = (%f, %f, %f) ", dimensions.x, dimensions.y, dimensions.z);
				  
				  if (view_i == 0)
				  {
				  	marker.scale.x = 0.0001;
					marker.scale.y = largest_side; 
					marker.scale.z = largest_side;
					
					dimensions.x = 0.0001; 
					dimensions.y = largest_side; 
					dimensions.z = largest_side;
				  }
				  else if (view_i ==1)
				  {
					marker.scale.x = largest_side; 
					marker.scale.y =  0.0001;
					marker.scale.z = largest_side; 
					
					dimensions.x = largest_side ; 
					dimensions.y = 0.0001;
					dimensions.z = largest_side; 
			
				  } 
				  else
				  {
				      marker.scale.x = largest_side; 
				      marker.scale.y = largest_side; 
				      marker.scale.z = 0.0001;
				      
				      dimensions.x = largest_side; 
				      dimensions.y = largest_side; 
				      dimensions.z = 0.0001;
				  }
				  			  
				  marker.color = cm->color(TID);		  
				  marker.color.a = 0.3;
						    
		
				  marker_array.markers.push_back(marker);
			  }
			  

				  
				  /* _________________________________
				    |                                 |
				    |         DRAW WIREFRAME          |
				    |_________________________________| */
				  if (1)
				  {
				    
					  visualization_msgs::Marker marker;
					  marker.header.frame_id = tracker_frame_id;
					  marker.header.stamp = ros::Time();
				
					  marker.ns = "wireframe";
					  marker.id = TID*170+view_i;
					  marker.frame_locked = locked;
					  marker.type = visualization_msgs::Marker::LINE_LIST;
					  //if (finish)
					  //marker.action = visualization_msgs::Marker::DELETE;
					  //else
					  marker.action = visualization_msgs::Marker::ADD;
					  marker.lifetime = Duration(duration);
							  
					 marker.pose.position =center_of_projected_cloud;
					 // marker.pose.position.x = 0;	marker.pose.position.y = 0;	marker.pose.position.z = 0;

					  //marker.pose = _tracked_object_msg.bounding_box.pose_stamped.pose;
				
					  marker.scale.x = 0.003; 

					  marker.color.a = 0.5;
					  marker.color.r = 0.5;
					  marker.color.g = 0.5;
					  marker.color.b = 0.5;
					  //marker
					  if (finish)
					  {
						  marker.color.r = 0.1;
						  marker.color.g = 0.1;
						  marker.color.b = 0.1;
					  }

					  
					  
					double x = dimensions.x/2; 
					double y = dimensions.y/2; 
					double z = dimensions.z/2; 
// 					double x = largest_side/2; 
// 					double y = largest_side/2; 
// 					double z = largest_side/2; 
					
					int k =number_of_bins;
					
					double interval_x = dimensions.x/k; 
					double interval_y = dimensions.y/k; 
					double interval_z = dimensions.z/k; 

// 					double interval_x = largest_side/k; 
// 					double interval_y = largest_side/k; 
// 					double interval_z = largest_side/k; 
				    
					if (view_i == 0)//for x
					{
					  //ROS_INFO("YOZ WIREFRAME plane size (x, y, z) = (%f, %f, %f) ", x, y, z);   
					  // Page YOZ for Y axis
					      for (int i=0; i<= k ; i++)  
					      {	
						    p.x = x; p.y = -y + i*interval_y; p.z = -z; 
						    marker.points.push_back(p);
						    p.x = x; p.y = -y + i*interval_y ; p.z =  z; 
						    marker.points.push_back(p);	
					      }
					      
					      // Page YOZ for Z axis
					      for (int i=0; i<= k ; i++)  
					      {
						    p.x = x; p.y = y; p.z = -z + i*interval_z; 
						    marker.points.push_back(p);
						    p.x = x; p.y = -y ; p.z = -z + i*interval_z; ; 
						    marker.points.push_back(p);
					      }
					      marker_array.markers.push_back(marker);
					      
					}
					
					else if (view_i == 1)//for y
					{
					  //ROS_INFO("XOZ WIREFRAME plane size (x, y, z) = (%f, %f, %f) ", x, y, z);   

					      // Page XOZ for X axis
					      for (int i=0; i<= k ; i++)  
					      {	
						    p.x = -x+ i*interval_x; p.y = y; p.z = -z; 
						    marker.points.push_back(p);
						    p.x = -x+ i*interval_x; p.y = y ; p.z =  z; 
						    marker.points.push_back(p);	
					      }
					      
					      // Page XOZ for Z axis
					      for (int i=0; i<= k ; i++)  
					      {
						    p.x = x; p.y = y; p.z = -z + i*interval_z; 
						    marker.points.push_back(p);
						    p.x = -x; p.y = y ; p.z = -z + i*interval_z; ; 
						    marker.points.push_back(p);
					      }
					      marker_array.markers.push_back(marker);

					}
					
					else //for z
					{
					  //ROS_INFO("XOY WIREFRAME plane size (x, y, z) = (%f, %f, %f) ", x, y, z);   
					      // Page XOY for X axis
					      for (int i=0; i<= k ; i++)  
					      {	
						    p.x = -x+ i*interval_x; p.y = -y; p.z = z; 
						    marker.points.push_back(p);
						    p.x = -x+ i*interval_x; p.y = y ; p.z = z; 
						    marker.points.push_back(p);	
					      }
					      
					      // Page XOY for Y axis
					      for (int i=0; i<= k ; i++)  
					      {	
						    p.x = -x; p.y = -y + i*interval_y; p.z = z; 
						    marker.points.push_back(p);
						    p.x = x; p.y = -y + i*interval_y ; p.z = z; 
						    marker.points.push_back(p);	
					      }
					      marker_array.markers.push_back(marker);
					      
					}
	      		      } 
	      		     

				
	      		     
				    /* _______________________
				    |                         |
				    |         histogram       |
				    |_________________________| */
				  if (1)
				  {
				    
				        visualization_msgs::Marker marker;
					//marker.header.frame_id = object_frame_id;
					marker.header.stamp = ros::Time();		
					marker.frame_locked = locked;
					marker.header.frame_id = tracker_frame_id;

					marker.ns = "histogram of projected views";
					marker.id = TID*10+view_i;
					marker.type = visualization_msgs::Marker::POINTS;
					marker.lifetime = Duration(duration);
					marker.action = visualization_msgs::Marker::ADD;
// 					marker.pose.position.x = 0; marker.pose.position.y = 0;	marker.pose.position.z = 0;
// 					marker.pose.orientation.x = 0.0; marker.pose.orientation.y = 0.0; marker.pose.orientation.z = 0.0; marker.pose.orientation.w = 1.0;
					marker.color = cm->color(TID);
					marker.scale.x = 0.01; marker.scale.y = 0.01; marker.scale.z = 0.01; marker.color.a = 1;
					marker.color.r = 0.0; marker.color.g = 0.0;	marker.color.b = 1.0; marker.color.a = 1.0; 
										  
					  
					double x = dimensions.x/2; 
					double y = dimensions.y/2; 
					double z = dimensions.z/2; 
					    
					int k =number_of_bins;
					
					double interval_x = dimensions.x/k; 
					double interval_y = dimensions.y/k; 
					double interval_z = dimensions.z/k; 

// 					double interval_x = largest_side/k; 
// 					double interval_y = largest_side/k; 
// 					double interval_z = largest_side/k; 
				    
					if (view_i == 0)//for x
					{
					  //ROS_INFO("YOZ histogram");   
					  for (int i=0; i<all_projected_views.at(view_i)-> points.size(); i++)
					  {
						geometry_msgs::Point p;
						//p.x = all_projected_views.at(view_i)->points.at(i).x + x;
						// 1/2 dimention is added to the real data to put all the data in positive scale between [0,2y][0-2z]
						p.y = all_projected_views.at(view_i)->points.at(i).y + sign * y;
						//p.y = all_projected_views.at(view_i)->points.at(i).y + y;
						p.z = all_projected_views.at(view_i)->points.at(i).z + z;
						//ROS_INFO("YOZ: P(y).(%i)= %f --- dy= %f", all_projected_views.at(view_i)->points.at(i).y, y );
						//ROS_INFO("(trunc(p.y / interval_y)= %f and (trunc(p.z / interval_z) =%f", trunc(p.y / interval_y), trunc(p.z / interval_z));
						if ((trunc(p.y / interval_y) == 0.0) and ((trunc(p.z / interval_z) == 0.0)))
						{
						    geometry_msgs::Point p;
						    p.x=all_projected_views.at(view_i)->points.at(i).x;
						    p.y=all_projected_views.at(view_i)->points.at(i).y;
						    p.z=all_projected_views.at(view_i)->points.at(i).z;
						    marker.points.push_back(p);
			
// 						    p.x=all_projected_views.at(view_i)->points.at(i).x;
// 						    p.y= - all_projected_views.at(view_i)->points.at(i).y;
// 						    p.z=all_projected_views.at(view_i)->points.at(i).z;
// 						    marker.points.push_back(p);
// 						    
						}
					    }
					    marker_array.markers.push_back(marker);
					      
					}
					
					else if (view_i == 1)//for y
					{
					  //ROS_INFO("XOZ histogram");   
					  for (int i=0; i<all_projected_views.at(view_i)-> points.size(); i++)
					  {					    
						geometry_msgs::Point p;
						// 1/2 dimention is added to the real data to put all the data in positive scale between [0,2x][0-2z]
						p.x = all_projected_views.at(view_i)->points.at(i).x + sign * x;
						//p.x = all_projected_views.at(view_i)->points.at(i).x + x;
						//p.y = all_projected_views.at(view_i)->points.at(i).y;
						p.z = all_projected_views.at(view_i)->points.at(i).z + z;
						//ROS_INFO("XOZ: P(x).(%i)= %f --- dx= %f", all_projected_views.at(view_i)->points.at(i).x, x );
						//ROS_INFO("(trunc(p.x / interval_x)= %f and (trunc(p.z / interval_z) =%f", trunc(p.x / interval_x), trunc(p.z / interval_z));
						
						if ((trunc(p.x / interval_x) == 0.0) and ((trunc(p.z / interval_z) == 0.0)))
						{
						    geometry_msgs::Point p;
						    p.x=all_projected_views.at(view_i)->points.at(i).x;
						    p.y=all_projected_views.at(view_i)->points.at(i).y;
						    p.z=all_projected_views.at(view_i)->points.at(i).z;
						    marker.points.push_back(p);
						    
// 						    p.x= - all_projected_views.at(view_i)->points.at(i).x;
// 						    p.y=all_projected_views.at(view_i)->points.at(i).y;
// 						    p.z=all_projected_views.at(view_i)->points.at(i).z;
// 						    marker.points.push_back(p);
						}
					    }
					    marker_array.markers.push_back(marker);

					}
					
					else //for z
					{
					  //ROS_INFO("XOY histogram");   					       
					  for (int i=0; i<all_projected_views.at(view_i)-> points.size(); i++)
					  {
						geometry_msgs::Point p;
						// 1/2 dimention is added to the real data to put all the data in positive scale between [0,2x][0-2y]
						p.x = all_projected_views.at(view_i)->points.at(i).x + sign * x;						
						p.y = all_projected_views.at(view_i)->points.at(i).y + sign * y;
						//p.x = all_projected_views.at(view_i)->points.at(i).x +  x;						
						//p.y = all_projected_views.at(view_i)->points.at(i).y +  y;
						//p.z = all_projected_views.at(view_i)->points.at(i).z + z;
						//ROS_INFO("XOY: P(x).(%i)= %f --- dx= %f", all_projected_views.at(view_i)->points.at(i).x, x );
						//ROS_INFO("(trunc(p.x / interval_x)= %f and (trunc(p.y / interval_y) =%f", trunc(p.x / interval_x), trunc(p.y / interval_y));
						if ((trunc(p.x / interval_x) == 0.0) and ((trunc(p.y / interval_y) == 0.0)))
						{
						    geometry_msgs::Point p;
						    p.x=all_projected_views.at(view_i)->points.at(i).x;
						    p.y=all_projected_views.at(view_i)->points.at(i).y;
						    p.z=all_projected_views.at(view_i)->points.at(i).z;
						    marker.points.push_back(p);
						    
// 						    p.x= - all_projected_views.at(view_i)->points.at(i).x;
// 						    p.y= - all_projected_views.at(view_i)->points.at(i).y;
// 						    p.z=all_projected_views.at(view_i)->points.at(i).z;
// 						    marker.points.push_back(p);
						}
					    }
					    marker_array.markers.push_back(marker);
					      
					}
	      		      } 

	      		      
	      		      /* _________________________________
				|                                 |
				|         DRAW TEXT INFO          |
				|_________________________________| */
				if (1)
				{
					
					visualization_msgs::Marker marker;
					//marker.header.frame_id = object_frame_id;
					marker.header.stamp = ros::Time();		
					marker.frame_locked = locked;
					marker.header.frame_id = tracker_frame_id;

//					marker.pose.position =center_of_boundingbox_cloud;
					marker.pose.position.x = 0;	marker.pose.position.y = 0;	marker.pose.position.z = 0;

					marker.ns = "sign";
					marker.id = TID*10+view_i;
					marker.type = visualization_msgs::Marker::TEXT_VIEW_FACING;
					marker.lifetime = Duration(duration);
					marker.action = visualization_msgs::Marker::ADD;
					//marker.pose.position.x = 0 + marker.pose.position.x*0.9 + 0;
					//marker.pose.position.y = 0 + marker.pose.position.y*0.9 + 0.1;
					marker.pose.position.z = largest_side + 0.1;
					marker.scale.z = 0.02; 
					marker.color.r  = 0; marker.color.g  = 0; marker.color.b  = 0; marker.color.a = 1;
					marker.text = "Sign = " + boost::lexical_cast<std::string>(sign);
					marker_array.markers.push_back(marker);
				}

				if (1)
				{
					
					visualization_msgs::Marker marker;
					//marker.header.frame_id = object_frame_id;
					marker.header.stamp = ros::Time();		
					marker.frame_locked = locked;
					marker.header.frame_id = tracker_frame_id;

					//marker.pose.position =center_of_gravity_cloud;
					marker.pose.position.x = 0;	marker.pose.position.y = 0;	marker.pose.position.z = 0;


					marker.ns = "entropy";
					marker.id = TID*100+view_i;
					marker.type = visualization_msgs::Marker::TEXT_VIEW_FACING;
					marker.lifetime = Duration(duration);
					marker.action = visualization_msgs::Marker::ADD;
					//marker.pose.position.x = 0 + marker.pose.position.x*0.9 + 0;
					//marker.pose.position.y = 0 + marker.pose.position.y*0.9 + 0.1;
					marker.pose.position.z = largest_side + 0.25;
					marker.scale.z = 0.02; 
					marker.color.r  = 1; marker.color.g  = 0; marker.color.b  = 0; marker.color.a = 1;
					marker.text = "H(YoZ) = " + boost::lexical_cast<std::string>(view_point_entropy.at(0))+
						      "\nH(XoZ) = " + boost::lexical_cast<std::string>(view_point_entropy.at(1))+
						      "\nH(XoY) = " + boost::lexical_cast<std::string>(view_point_entropy.at(2));
					marker_array.markers.push_back(marker);
				}

				if (1)
				{
					
					visualization_msgs::Marker marker;
					//marker.header.frame_id = object_frame_id;
					marker.header.stamp = ros::Time();		
					marker.frame_locked = locked;
					marker.header.frame_id = tracker_frame_id;

					//marker.pose.position =center_of_gravity_cloud;
					marker.pose.position.x = 0;	marker.pose.position.y = 0;	marker.pose.position.z = 0;


					marker.ns = "sorted projections";
					marker.id = TID*200+view_i;
					marker.type = visualization_msgs::Marker::TEXT_VIEW_FACING;
					marker.lifetime = Duration(duration);
					marker.action = visualization_msgs::Marker::ADD;
					//marker.pose.position.x = 0 + marker.pose.position.x*0.9 + 0;
					//marker.pose.position.y = 0 + marker.pose.position.y*0.9 + 0.1;
					marker.pose.position.z = largest_side + 0.3;
					marker.scale.z = 0.02; 
					marker.color.r  = 0; marker.color.g  = 0; marker.color.b  = 0; marker.color.a = 1;
					marker.text = std_name_of_sorted_projected_plane;
					marker_array.markers.push_back(marker);
				}

				
				
			      /* _______________________________
			      |                                 |
			      |           DRAW XYZ AXES         |
			      |_________________________________| */
				if (1)
				{	
					visualization_msgs::Marker marker;
					double axis_dimension = sign * 0.2;
					marker.header.frame_id = tracker_frame_id;
					marker.header.stamp = ros::Time();

					marker.frame_locked = locked;
					marker.type = visualization_msgs::Marker::LINE_STRIP;
					marker.action = visualization_msgs::Marker::ADD;
					marker.lifetime = Duration(duration);

					//marker.pose.position =center_of_boundingbox_cloud;
					marker.pose.position.x = 0;	marker.pose.position.y = 0;	marker.pose.position.z = 0;

					marker.scale.x = 0.01; marker.scale.y = 0.5; marker.scale.z = 4;

					//X axis
					marker.ns = "axes_x";
					marker.id = TID*10+view_i;;
					marker.color.r = 1.0; marker.color.g = 0.0;	marker.color.b = 0.0; marker.color.a = 1.0; //red color
					marker.points.erase(marker.points.begin(), marker.points.end());
					p.x = 0; p.y = 0; p.z = 0; 
					marker.points.push_back(p);
					p.x = 1 * axis_dimension; p.y = 0; p.z = 0; 
					marker.points.push_back(p);
					marker_array.markers.push_back(marker);

					//Y axis
					marker.ns = "axes_y";
					marker.id = TID*10+view_i;
					marker.color.r = 0.0; marker.color.g = 1.0;	marker.color.b = 0.0; marker.color.a = 1.0; //green color
					marker.points.erase(marker.points.begin(), marker.points.end());
					p.x = 0; p.y = 0; p.z = 0; 
					marker.points.push_back(p);
					p.x = 0; p.y = 1 * axis_dimension; p.z = 0; 
					marker.points.push_back(p);
					marker_array.markers.push_back(marker);

					//Z axis
					marker.ns = "axes_z";
					marker.id = TID*10+view_i;
					marker.color.r = 0.0; marker.color.g = 0.0;	marker.color.b = 1.0; marker.color.a = 1.0; //blue color
					marker.points.erase(marker.points.begin(), marker.points.end());
					p.x = 0; p.y = 0; p.z = 0; 

					marker.points.push_back(p);

					p.x = 0; p.y =0 ; p.z = 1 * sign * axis_dimension;

					
// 					if (sign > 0 )
// 					{
// 					  p.x = 0; p.y = 0; p.z = 1 * axis_dimension;  
// 					}
// 					else 
// 					{
// 					  p.x = 0; p.y = 0; p.z = 1 * -axis_dimension;
// 					}
					 
					marker.points.push_back(p);
					marker_array.markers.push_back(marker);
				}
				
				}
				


		      neat_marker_publisher.publish(marker_array);
		      return 1;


		  }
		
		template <typename T>
	    int set_neat_visualization_marker_array_object_descriptor_vector( boost::shared_ptr<pcl::PointCloud<T> > object_view,
																		vector < boost::shared_ptr<pcl::PointCloud<T> > >  all_projected_views, 
																		string object_frame_id, 
																		unsigned int TID , 
																		double largest_side, 
																		int sign,
																		vector <float> view_point_entropy,
																		string std_name_of_sorted_projected_plane										
																		)
	    {
	      
		std::string tracker_frame_id = "/perception/pipeline" + boost::lexical_cast<std::string>(TID) + "/tracker";

		ROS_INFO("dimension of largest_side =  %f ", largest_side);

		visualization_msgs::MarkerArray marker_array; 
		//geometry_msgs::Point p;
		//TODO: duration should be a parameter
		//double duration =2;// online
		double duration =30;// offline
		bool locked = true;
		bool finish= true;
		//float center_of_projected_cloud_x=0 , center_of_projected_cloud_y=0 , center_of_projected_cloud_z=0 ;
		//geometry_msgs::Point center_of_projected_cloud;
		//geometry_msgs::Vector3 dimensions;
		//geometry_msgs::Point p;
		// int view_i = 1;
		
		
		
		//////////////////////////// working with a dataset /////////////////////////////
		geometry_msgs::Point center_of_boundingbox_cloud;
		geometry_msgs::Vector3 dimensions;
		geometry_msgs::Point p;
		float center_of_gravity_cloud_x=0 , center_of_gravity_cloud_y=0 , center_of_gravity_cloud_z=0 ;
// 		/* _________________________________
// 		|                                 |
// 		|   Draw  Point Cloud of object   |
// 		|_________________________________| */
// 	      
// 		if (1)
// 		{
// 	  
// 	    
// 		  visualization_msgs::Marker marker;
// 		  //marker.header.frame_id = object_frame_id;
// 		  marker.header.stamp = ros::Time();		
// 		  marker.frame_locked = locked;
// 		  marker.header.frame_id = tracker_frame_id;
// 
// 		  marker.ns = "object view";
// 		  marker.id = TID*100000000;
// 		  marker.type = visualization_msgs::Marker::POINTS;
// 		  marker.lifetime = Duration(duration);
// 		  marker.action = visualization_msgs::Marker::ADD;
// 		  marker.pose.position.x = 0;	marker.pose.position.y = 0;	marker.pose.position.z = 0;
// 		  marker.pose.orientation.x = 0.0; marker.pose.orientation.y = 0.0; marker.pose.orientation.z = 0.0; marker.pose.orientation.w = 1.0;
// // 		  marker.color = cm->color(TID);
// 		  marker.color.r = 0;
// 		  marker.color.g = 0;
// 		  marker.color.b = 0;
// 		  marker.color.a = 1;
// 		  
// 		  marker.scale.x = 0.005; marker.scale.y = 0.005; marker.scale.z = 0.005; marker.color.a = 1;
// 		  //marker.color.r = 1.0; marker.color.g = 1.0;	marker.color.b = 1.0; marker.color.a = 1.0; 
// 		  
// 		  for (size_t i=0; i< object_view->points.size(); i++)
// 		  {
// 		      p.x = object_view->points.at(i).x;
// 		      p.y = object_view->points.at(i).y;
// 		      p.z = object_view->points.at(i).z;
// 		      
// 		      marker.points.push_back(p);
// 		      center_of_gravity_cloud_x += p.x;
// 		      center_of_gravity_cloud_y += p.y;
// 		      center_of_gravity_cloud_z += p.z;
// 		  }
// 
// 		  marker_array.markers.push_back(marker);
// 		  
// // 		  center_of_gravity_cloud.x= center_of_gravity_cloud_x/object_view->points.size();
// // 		  center_of_gravity_cloud.y= center_of_gravity_cloud_y/object_view->points.size();
// // 		  center_of_gravity_cloud.z= center_of_gravity_cloud_z/object_view->points.size();
// 			    
// 		}    
// 			    
			    
		
// 		 /* _________________________________
// 	      	   |                                 |
// 	      	   |          DRAW BB WIREFRAME      |
// 	      	   |_________________________________| */
// 	      	if (1)
// 	      	{
// 	      		visualization_msgs::Marker marker;
// 	      		marker.header.frame_id = tracker_frame_id;
// 	      		marker.header.stamp = ros::Time();
// 	      
// 	      		marker.ns = "wireframe";
// 	      		marker.id = TID*10000;
// 	      		marker.frame_locked = locked;
// 	      		marker.type = visualization_msgs::Marker::LINE_LIST;
// 	      		//if (finish)
// 	      		//marker.action = visualization_msgs::Marker::DELETE;
// 	      		//else
// 	      		marker.action = visualization_msgs::Marker::ADD;
// 	      		marker.lifetime = Duration(duration);
// 	      
// 			compute_bounding_box_dimensions(object_view, dimensions);
// 			center_of_boundingbox_cloud.x=dimensions.x/2;
// 			center_of_boundingbox_cloud.y=dimensions.y/2;
// 			center_of_boundingbox_cloud.z=dimensions.z/2;
// 
// 			marker.pose.position = center_of_boundingbox_cloud;	/*marker.pose.position.y = 0;	marker.pose.position.z = 0;*/
// 			marker.pose.orientation.x = 0.0; marker.pose.orientation.y = 0.0; marker.pose.orientation.z = 0.0; marker.pose.orientation.w = 1.0;
// 			    				
// 			
// 	      		marker.scale.x = 0.005; 
// 	      		double x = dimensions.x/2; 
// 	      		double y = dimensions.y/2; 
// 	      		double z = dimensions.z/2; 
// 	      
// 	      		marker.color = cm->color(TID);
// 	      		marker.color.r = 0.5;
// 	      		marker.color.g = 0.5;
// 	      		marker.color.b = 0.5;
// 			marker.color.a = 0.1;
// 	      		//marker
// 	      		if (finish)
// 	      		{
// 	      			marker.color.r = 0.1;
// 	      			marker.color.g = 0.1;
// 	      			marker.color.b = 0.1;
// 	      		}
// 
// 	      		geometry_msgs::Point p;
// 	      		p.x =  x; p.y =  y; p.z =  z; marker.points.push_back(p);
// 	      		p.x = -x; p.y =  y; p.z =  z; marker.points.push_back(p);
// 	      		p.x =  x; p.y =  y; p.z = -z; marker.points.push_back(p);
// 	      		p.x = -x; p.y =  y; p.z = -z; marker.points.push_back(p);
// 	      		p.x =  x; p.y =  y; p.z = -z; marker.points.push_back(p);
// 	      		p.x =  x; p.y =  y; p.z =  z; marker.points.push_back(p);
// 	      		p.x = -x; p.y =  y; p.z = -z; marker.points.push_back(p);
// 	      		p.x = -x; p.y =  y; p.z =  z; marker.points.push_back(p);
// 	      
// 	      		p.x =  x; p.y =  -y; p.z =  z; marker.points.push_back(p);
// 	      		p.x = -x; p.y =  -y; p.z =  z; marker.points.push_back(p);
// 	      		p.x =  x; p.y =  -y; p.z = -z; marker.points.push_back(p);
// 	      		p.x = -x; p.y =  -y; p.z = -z; marker.points.push_back(p);
// 	      		p.x =  x; p.y =  -y; p.z = -z; marker.points.push_back(p);
// 	      		p.x =  x; p.y =  -y; p.z =  z; marker.points.push_back(p);
// 	      		p.x = -x; p.y =  -y; p.z = -z; marker.points.push_back(p);
// 	      		p.x = -x; p.y =  -y; p.z =  z; marker.points.push_back(p);
// 	      
// 	      		p.x =  x; p.y =  y; p.z =  z; marker.points.push_back(p);
// 	      		p.x =  x; p.y = -y; p.z =  z; marker.points.push_back(p);
// 	      		p.x =  x; p.y =  y; p.z = -z; marker.points.push_back(p);
// 	      		p.x =  x; p.y = -y; p.z = -z; marker.points.push_back(p);
// 	      
// 	      		p.x = -x; p.y =  y; p.z =  z; marker.points.push_back(p);
// 	      		p.x = -x; p.y = -y; p.z =  z; marker.points.push_back(p);
// 	      		p.x = -x; p.y =  y; p.z = -z; marker.points.push_back(p);
// 	      		p.x = -x; p.y = -y; p.z = -z; marker.points.push_back(p);
// 	      
// 	      		marker_array.markers.push_back(marker);
// 	      	}
// 	      
// 	      
// 	      	/* _________________________________
// 	      	   |                                 |
// 	      	   |             DRAW BBOX           |
// 	      	   |_________________________________| */
// 	      	if (1)
// 	      	{
// 	      		visualization_msgs::Marker marker;
// 	      		marker.header.frame_id = tracker_frame_id;
// 	      		marker.header.stamp = ros::Time();
// 	      
// 	      		marker.ns = "boundingbox";
// 	      		marker.id = TID*200;
// 	      		marker.type = visualization_msgs::Marker::CUBE;
// 	      		marker.frame_locked = locked;
// 	      		//if (finish)
// 	      		//marker.action = visualization_msgs::Marker::DELETE;
// 	      		//else
// 	      		marker.action = visualization_msgs::Marker::ADD;
// 	      		marker.lifetime = Duration(duration);
// 	      
// 			marker.pose.position = center_of_boundingbox_cloud;
// 			marker.pose.orientation.x = 0.0; marker.pose.orientation.y = 0.0; marker.pose.orientation.z = 0.0; marker.pose.orientation.w = 1.0;
// 	      		
// 			marker.scale.x = dimensions.x; 
// 	      		marker.scale.y = dimensions.y; 
// 	      		marker.scale.z = dimensions.z; 
// 	      
// 
// 			marker.color = cm->color(TID);
// 			marker.color.a = 0.1;
// 			
// 			if (finish)
// 	      		{
// 	      			marker.color.r = 0.9;
// 	      			marker.color.g = 0.0;
// 	      			marker.color.b = 0.0;
// 	      		}
// 	      
// 	      		marker_array.markers.push_back(marker);
// 	      	}
		
		
		
		/* _________________________________
		|                                 |
		|   Draw Projected Point Cloud    |
		|_________________________________| */
			      
		for (int view_i = 0; view_i< all_projected_views.size(); view_i++)			
		{
			geometry_msgs::Point center_of_projected_cloud;
			geometry_msgs::Vector3 dimensions;
			geometry_msgs::Point p;
			float center_of_projected_cloud_x=0 , center_of_projected_cloud_y=0 , center_of_projected_cloud_z=0 ;

			if (1)
			{
		    
		      
			    visualization_msgs::Marker marker;
			    //marker.header.frame_id = object_frame_id;
			    marker.header.stamp = ros::Time();		
			    marker.frame_locked = locked;
			    marker.header.frame_id = tracker_frame_id;

			    marker.ns = "projected views";
			    marker.id = TID*32+view_i;
			    marker.type = visualization_msgs::Marker::POINTS;
			    marker.lifetime = Duration(duration);
			    marker.action = visualization_msgs::Marker::ADD;
// 			    marker.pose.position.x = 0;	marker.pose.position.y = 0;	marker.pose.position.z = 0;
// 			    marker.pose.orientation.x = 0.0; marker.pose.orientation.y = 0.0; marker.pose.orientation.z = 0.0; marker.pose.orientation.w = 1.0;
			    marker.color = cm->color(TID);
			    marker.scale.x = 0.0035; marker.scale.y = 0.0035; marker.scale.z = 0.0035; marker.color.a = 1;
			    //marker.color.r = 1.0; marker.color.g = 1.0;	marker.color.b = 1.0; marker.color.a = 1.0; 
			    
			    //marker.points.erase(marker.points.begin(), marker.points.end());
			    geometry_msgs::Point p;

			    for (size_t i=0; i<all_projected_views.at(view_i)-> points.size(); i++)
			    {
// 				p.x = sign * all_projected_views.at(view_i)->points.at(i).x;
// 				p.y = sign * all_projected_views.at(view_i)->points.at(i).y;
// 				p.z = all_projected_views.at(view_i)->points.at(i).z;
// 				marker.points.push_back(p);

				all_projected_views.at(view_i)->points.at(i).x = sign * all_projected_views.at(view_i)->points.at(i).x;
				all_projected_views.at(view_i)->points.at(i).y = sign * all_projected_views.at(view_i)->points.at(i).y;
				p.x = all_projected_views.at(view_i)->points.at(i).x;
				p.y = all_projected_views.at(view_i)->points.at(i).y;
				p.z = all_projected_views.at(view_i)->points.at(i).z;
				
				marker.points.push_back(p);
				center_of_projected_cloud_x += p.x;
				center_of_projected_cloud_y += p.y;
				center_of_projected_cloud_z += p.z;
			    }

			    marker_array.markers.push_back(marker);
			    
// 			    center_of_projected_cloud.x= center_of_projected_cloud_x/all_projected_views.at(view_i)->points.size();
// 			    center_of_projected_cloud.y= center_of_projected_cloud_y/all_projected_views.at(view_i)->points.size();
// 			    center_of_projected_cloud.z= center_of_projected_cloud_z/all_projected_views.at(view_i)->points.size();

			    center_of_projected_cloud.x= 0.3;
			    center_of_projected_cloud.y= 0.3;
			    center_of_projected_cloud.z= 0.3;
			    
			}
			
			  /* _________________________________
			    |                                 |
			    |           Draw Plane            |
			    |_________________________________| */
			  if (1)
			  {
				  visualization_msgs::Marker marker;
				  marker.header.frame_id = tracker_frame_id;
				  marker.header.stamp = ros::Time();
			
				  marker.ns = "Plane";
				  marker.id = TID*33+view_i;
				  marker.type = visualization_msgs::Marker::CUBE;
				  marker.frame_locked = locked;
				  //if (finish)
				  //marker.action = visualization_msgs::Marker::DELETE;
				  //else
				  marker.action = visualization_msgs::Marker::ADD;
				  marker.lifetime = Duration(duration);
			
//  				  marker.pose.position =center_of_projected_cloud;
				  //compute_bounding_box_dimensions(all_projected_views.at(view_i), dimensions);
				  //ROS_INFO("Visuzlize box dimensions (x, y, z) = (%f, %f, %f) ", dimensions.x, dimensions.y, dimensions.z);
				  
				  if (view_i == 0)
				  {
					marker.pose.position.x = sign *  0.3;
					marker.scale.x = 0.0001;
					marker.scale.y = largest_side; 
					marker.scale.z = largest_side;
					
					dimensions.x = 0.0001; 
					dimensions.y = largest_side; 
					dimensions.z = largest_side;
				  }
				  else if (view_i ==1)
				  {
					marker.pose.position.y = sign *  0.3;
					marker.scale.x = largest_side; 
					marker.scale.y =  0.0001;
					marker.scale.z = largest_side; 
					
					dimensions.x = largest_side ; 
					dimensions.y = 0.0001;
					dimensions.z = largest_side; 
			
				  } 
				  else
				  {

				      marker.pose.position.z =  0.3;
				      marker.scale.x = largest_side; 
				      marker.scale.y = largest_side; 
				      marker.scale.z = 0.0001;
				      
				      dimensions.x = largest_side; 
				      dimensions.y = largest_side; 
				      dimensions.z = 0.0001;
				  }
				  			  
				  marker.color = cm->color(TID);		  
				  marker.color.a = 0.3;
						    
		
				  marker_array.markers.push_back(marker);
			  }
			  

				  
				  /* _________________________________
				    |                                 |
				    |         DRAW WIREFRAME          |
				    |_________________________________| */
				  if (1)
				  {
				    
					  visualization_msgs::Marker marker;
					  marker.header.frame_id = tracker_frame_id;
					  marker.header.stamp = ros::Time();
				
					  marker.ns = "wireframe";
					  marker.id = TID*44+view_i;
					  marker.frame_locked = locked;
					  marker.type = visualization_msgs::Marker::LINE_LIST;
					  //if (finish)
					  //marker.action = visualization_msgs::Marker::DELETE;
					  //else
					  marker.action = visualization_msgs::Marker::ADD;
					  marker.lifetime = Duration(duration);
							  
					  //marker.pose.position =center_of_projected_cloud;

					  //marker.pose = _tracked_object_msg.bounding_box.pose_stamped.pose;
				
					  marker.scale.x = 0.003; 
					  
				
					  marker.color.a = 0.5;
					  marker.color.r = 0.5;
					  marker.color.g = 0.5;
					  marker.color.b = 0.5;
					  //marker
					  if (finish)
					  {
						  marker.color.r = 0.1;
						  marker.color.g = 0.1;
						  marker.color.b = 0.1;
					  }

					  
					  
					double x = dimensions.x/2; 
					double y = dimensions.y/2; 
					double z = dimensions.z/2; 
// 					double x = largest_side/2; 
// 					double y = largest_side/2; 
// 					double z = largest_side/2; 
					

					    
					int k =number_of_bins;
					
					double interval_x = dimensions.x/k; 
					double interval_y = dimensions.y/k; 
					double interval_z = dimensions.z/k; 

// 					double interval_x = largest_side/k; 
// 					double interval_y = largest_side/k; 
// 					double interval_z = largest_side/k; 
				    
					if (view_i == 0)//for x
					{

					    marker.pose.position.x = sign * 0.3;

					  //ROS_INFO("YOZ WIREFRAME plane size (x, y, z) = (%f, %f, %f) ", x, y, z);   
					  // Page YOZ for Y axis
					      for (int i=0; i<= k ; i++)  
					      {	
						    p.x = x; p.y = -y + i*interval_y; p.z = -z; 
						    marker.points.push_back(p);
						    p.x = x; p.y = -y + i*interval_y ; p.z =  z; 
						    marker.points.push_back(p);	
					      }
					      
					      // Page YOZ for Z axis
					      for (int i=0; i<= k ; i++)  
					      {
						    p.x = x; p.y = y; p.z = -z + i*interval_z; 
						    marker.points.push_back(p);
						    p.x = x; p.y = -y ; p.z = -z + i*interval_z; ; 
						    marker.points.push_back(p);
					      }
					      marker_array.markers.push_back(marker);
					      
					}
					
					else if (view_i == 1)//for y
					{
					  //ROS_INFO("XOZ WIREFRAME plane size (x, y, z) = (%f, %f, %f) ", x, y, z);   
					    marker.pose.position.y = sign * 0.3;

					      // Page XOZ for X axis
					      for (int i=0; i<= k ; i++)  
					      {	
						    p.x = -x+ i*interval_x; p.y = y; p.z = -z; 
						    marker.points.push_back(p);
						    p.x = -x+ i*interval_x; p.y = y ; p.z =  z; 
						    marker.points.push_back(p);	
					      }
					      
					      // Page XOZ for Z axis
					      for (int i=0; i<= k ; i++)  
					      {
						    p.x = x; p.y = y; p.z = -z + i*interval_z; 
						    marker.points.push_back(p);
						    p.x = -x; p.y = y ; p.z = -z + i*interval_z; ; 
						    marker.points.push_back(p);
					      }
					      marker_array.markers.push_back(marker);

					}
					
					else //for z
					{
						marker.pose.position.z = 0.3;

					  //ROS_INFO("XOY WIREFRAME plane size (x, y, z) = (%f, %f, %f) ", x, y, z);   
					      // Page XOY for X axis
					      for (int i=0; i<= k ; i++)  
					      {	
						    p.x = -x+ i*interval_x; p.y = -y; p.z = z; 
						    marker.points.push_back(p);
						    p.x = -x+ i*interval_x; p.y = y ; p.z = z; 
						    marker.points.push_back(p);	
					      }
					      
					      // Page XOY for Y axis
					      for (int i=0; i<= k ; i++)  
					      {	
						    p.x = -x; p.y = -y + i*interval_y; p.z = z; 
						    marker.points.push_back(p);
						    p.x = x; p.y = -y + i*interval_y ; p.z = z; 
						    marker.points.push_back(p);	
					      }
					      marker_array.markers.push_back(marker);
					      
					}
	      		      } 
	      		     

				
	      		     
				    /* _______________________
				    |                         |
				    |         histogram       |
				    |_________________________| */
				  if (1)
				  {
				    
				        visualization_msgs::Marker marker;
					//marker.header.frame_id = object_frame_id;
					marker.header.stamp = ros::Time();		
					marker.frame_locked = locked;
					marker.header.frame_id = tracker_frame_id;

					marker.ns = "histogram of projected views";
					marker.id = TID*55+view_i;
					marker.type = visualization_msgs::Marker::POINTS;
					marker.lifetime = Duration(duration);
					marker.action = visualization_msgs::Marker::ADD;
// 					marker.pose.position.x = 0; marker.pose.position.y = 0;	marker.pose.position.z = 0;
// 					marker.pose.orientation.x = 0.0; marker.pose.orientation.y = 0.0; marker.pose.orientation.z = 0.0; marker.pose.orientation.w = 1.0;
					marker.color = cm->color(TID);
					marker.scale.x = 0.01; marker.scale.y = 0.01; marker.scale.z = 0.01; marker.color.a = 1;
					marker.color.r = 0.0; marker.color.g = 0.0;	marker.color.b = 1.0; marker.color.a = 1.0; 
										  
					  
					double x = dimensions.x/2; 
					double y = dimensions.y/2; 
					double z = dimensions.z/2; 
					    
					int k =number_of_bins;
					
					double interval_x = dimensions.x/k; 
					double interval_y = dimensions.y/k; 
					double interval_z = dimensions.z/k; 

// 					double interval_x = largest_side/k; 
// 					double interval_y = largest_side/k; 
// 					double interval_z = largest_side/k; 
				    
					if (view_i == 0)//for x
					{
					  //ROS_INFO("YOZ histogram");   
					  for (int i=0; i<all_projected_views.at(view_i)-> points.size(); i++)
					  {
						geometry_msgs::Point p;
						//p.x = all_projected_views.at(view_i)->points.at(i).x + x;
						// 1/2 dimention is added to the real data to put all the data in positive scale between [0,2y][0-2z]
						p.y = all_projected_views.at(view_i)->points.at(i).y + sign * y;
						//p.y = all_projected_views.at(view_i)->points.at(i).y + y;
						p.z = all_projected_views.at(view_i)->points.at(i).z + z;
						//ROS_INFO("YOZ: P(y).(%i)= %f --- dy= %f", all_projected_views.at(view_i)->points.at(i).y, y );
						//ROS_INFO("(trunc(p.y / interval_y)= %f and (trunc(p.z / interval_z) =%f", trunc(p.y / interval_y), trunc(p.z / interval_z));
						if ((trunc(p.y / interval_y) == 0.0) and ((trunc(p.z / interval_z) == 0.0)))
						{
						    geometry_msgs::Point p;
						    p.x=all_projected_views.at(view_i)->points.at(i).x;
						    p.y=all_projected_views.at(view_i)->points.at(i).y;
						    p.z=all_projected_views.at(view_i)->points.at(i).z;
						    marker.points.push_back(p);
			
// 						    p.x=all_projected_views.at(view_i)->points.at(i).x;
// 						    p.y= - all_projected_views.at(view_i)->points.at(i).y;
// 						    p.z=all_projected_views.at(view_i)->points.at(i).z;
// 						    marker.points.push_back(p);
// 						    
						}
					    }
					    marker_array.markers.push_back(marker);
					      
					}
					
					else if (view_i == 1)//for y
					{
					  //ROS_INFO("XOZ histogram");   
					  for (int i=0; i<all_projected_views.at(view_i)-> points.size(); i++)
					  {					    
						geometry_msgs::Point p;
						// 1/2 dimention is added to the real data to put all the data in positive scale between [0,2x][0-2z]
						p.x = all_projected_views.at(view_i)->points.at(i).x + sign * x;
						//p.x = all_projected_views.at(view_i)->points.at(i).x + x;
						//p.y = all_projected_views.at(view_i)->points.at(i).y;
						p.z = all_projected_views.at(view_i)->points.at(i).z + z;
						//ROS_INFO("XOZ: P(x).(%i)= %f --- dx= %f", all_projected_views.at(view_i)->points.at(i).x, x );
						//ROS_INFO("(trunc(p.x / interval_x)= %f and (trunc(p.z / interval_z) =%f", trunc(p.x / interval_x), trunc(p.z / interval_z));
						
						if ((trunc(p.x / interval_x) == 0.0) and ((trunc(p.z / interval_z) == 0.0)))
						{
						    geometry_msgs::Point p;
						    p.x=all_projected_views.at(view_i)->points.at(i).x;
						    p.y=all_projected_views.at(view_i)->points.at(i).y;
						    p.z=all_projected_views.at(view_i)->points.at(i).z;
						    marker.points.push_back(p);
						    
// 						    p.x= - all_projected_views.at(view_i)->points.at(i).x;
// 						    p.y=all_projected_views.at(view_i)->points.at(i).y;
// 						    p.z=all_projected_views.at(view_i)->points.at(i).z;
// 						    marker.points.push_back(p);
						}
					    }
					    marker_array.markers.push_back(marker);

					}
					
					else //for z
					{
					  //ROS_INFO("XOY histogram");   					       
					  for (int i=0; i<all_projected_views.at(view_i)-> points.size(); i++)
					  {
						geometry_msgs::Point p;
						// 1/2 dimention is added to the real data to put all the data in positive scale between [0,2x][0-2y]
						p.x = all_projected_views.at(view_i)->points.at(i).x + sign * x;						
						p.y = all_projected_views.at(view_i)->points.at(i).y + sign * y;
						//p.x = all_projected_views.at(view_i)->points.at(i).x +  x;						
						//p.y = all_projected_views.at(view_i)->points.at(i).y +  y;
						//p.z = all_projected_views.at(view_i)->points.at(i).z + z;
						//ROS_INFO("XOY: P(x).(%i)= %f --- dx= %f", all_projected_views.at(view_i)->points.at(i).x, x );
						//ROS_INFO("(trunc(p.x / interval_x)= %f and (trunc(p.y / interval_y) =%f", trunc(p.x / interval_x), trunc(p.y / interval_y));
						if ((trunc(p.x / interval_x) == 0.0) and ((trunc(p.y / interval_y) == 0.0)))
						{
						    geometry_msgs::Point p;
						    p.x=all_projected_views.at(view_i)->points.at(i).x;
						    p.y=all_projected_views.at(view_i)->points.at(i).y;
						    p.z=all_projected_views.at(view_i)->points.at(i).z;
						    marker.points.push_back(p);
						    
// 						    p.x= - all_projected_views.at(view_i)->points.at(i).x;
// 						    p.y= - all_projected_views.at(view_i)->points.at(i).y;
// 						    p.z=all_projected_views.at(view_i)->points.at(i).z;
// 						    marker.points.push_back(p);
						}
					    }
					    marker_array.markers.push_back(marker);
					      
					}
	      		      } 

	      		      
	      		      /* _________________________________
				|                                 |
				|         DRAW TEXT INFO          |
				|_________________________________| */
				if (1)
				{
					
					visualization_msgs::Marker marker;
					//marker.header.frame_id = object_frame_id;
					marker.header.stamp = ros::Time();		
					marker.frame_locked = locked;
					marker.header.frame_id = tracker_frame_id;

					//marker.pose.position =center_of_boundingbox_cloud;

					marker.ns = "sign";
					marker.id = TID*10+view_i;
					marker.type = visualization_msgs::Marker::TEXT_VIEW_FACING;
					marker.lifetime = Duration(duration);
					marker.action = visualization_msgs::Marker::ADD;
					//marker.pose.position.x = 0 + marker.pose.position.x*0.9 + 0;
					//marker.pose.position.y = 0 + marker.pose.position.y*0.9 + 0.1;
					marker.pose.position.z = largest_side + 0.15;
					marker.scale.z = 0.02; 
					marker.color.r  = 1; marker.color.g  = 0; marker.color.b  = 0; marker.color.a = 1;
					marker.text = "Sign = " + boost::lexical_cast<std::string>(sign);
					marker_array.markers.push_back(marker);
				}

				if (1)
				{
					
// 					visualization_msgs::Marker marker;
// 					//marker.header.frame_id = object_frame_id;
// 					marker.header.stamp = ros::Time();		
// 					marker.frame_locked = locked;
// 					marker.header.frame_id = tracker_frame_id;
// 
// 					//marker.pose.position =center_of_projected_cloud;
// 
// 					marker.ns = "entropy";
// 					marker.id = TID*100+view_i;
// 					marker.type = visualization_msgs::Marker::TEXT_VIEW_FACING;
// 					marker.lifetime = Duration(duration);
// 					marker.action = visualization_msgs::Marker::ADD;
// 					//marker.pose.position.x = 0 + marker.pose.position.x*0.9 + 0;
// 					//marker.pose.position.y = 0 + marker.pose.position.y*0.9 + 0.1;
// 					marker.pose.position.z = largest_side + 0.25;
// 					marker.scale.z = 0.02; 
// 					marker.color.r  = 1; marker.color.g  = 0; marker.color.b  = 0; marker.color.a = 1;
// 					marker.text = "H(YoZ) = " + boost::lexical_cast<std::string>(view_point_entropy.at(0))+
// 						      "\nH(XoZ) = " + boost::lexical_cast<std::string>(view_point_entropy.at(1))+
// 						      "\nH(XoY) = " + boost::lexical_cast<std::string>(view_point_entropy.at(2));
// 					marker_array.markers.push_back(marker);
				}

				if (1)
				{
					
// 					visualization_msgs::Marker marker;
// 					//marker.header.frame_id = object_frame_id;
// 					marker.header.stamp = ros::Time();		
// 					marker.frame_locked = locked;
// 					marker.header.frame_id = tracker_frame_id;
// 
// 					//marker.pose.position =center_of_projected_cloud;
// 
// 					marker.ns = "sorted projections";
// 					marker.id = TID*200+view_i;
// 					marker.type = visualization_msgs::Marker::TEXT_VIEW_FACING;
// 					marker.lifetime = Duration(duration);
// 					marker.action = visualization_msgs::Marker::ADD;
// 					//marker.pose.position.x = 0 + marker.pose.position.x*0.9 + 0;
// 					//marker.pose.position.y = 0 + marker.pose.position.y*0.9 + 0.1;
// 					marker.pose.position.z = largest_side + 0.3;
// 					marker.scale.z = 0.02; 
// 					marker.color.r  = 0; marker.color.g  = 0; marker.color.b  = 0; marker.color.a = 1;
// 					marker.text = std_name_of_sorted_projected_plane;
// 					marker_array.markers.push_back(marker);
				}

				
				
			      /* _______________________________
			      |                                 |
			      |           DRAW XYZ AXES         |
			      |_________________________________| */
				if (1)
				{	
					visualization_msgs::Marker marker;
					double axis_dimension = sign * 0.2;
					marker.header.frame_id = tracker_frame_id;
					marker.header.stamp = ros::Time();

					marker.frame_locked = locked;
					marker.type = visualization_msgs::Marker::LINE_STRIP;
					marker.action = visualization_msgs::Marker::ADD;
					marker.lifetime = Duration(duration);

					//marker.pose.position =center_of_projected_cloud;

					marker.scale.x = 0.01; marker.scale.y = 0.5; marker.scale.z = 4;

					//X axis
					marker.ns = "axes_x";
					marker.id = TID*1780+view_i;;
					marker.color.r = 1.0; marker.color.g = 0.0;	marker.color.b = 0.0; marker.color.a = 1.0; //red color
					marker.points.erase(marker.points.begin(), marker.points.end());
					p.x = 0; p.y = 0; p.z = 0; 
					marker.points.push_back(p);
					p.x = 1 * axis_dimension; p.y = 0; p.z = 0; 
					marker.points.push_back(p);
					marker_array.markers.push_back(marker);

					//Y axis
					marker.ns = "axes_y";
					marker.id = TID*10+view_i;
					marker.color.r = 0.0; marker.color.g = 1.0;	marker.color.b = 0.0; marker.color.a = 1.0; //green color
					marker.points.erase(marker.points.begin(), marker.points.end());
					p.x = 0; p.y = 0; p.z = 0; 
					marker.points.push_back(p);
					p.x = 0; p.y = 1 * axis_dimension; p.z = 0; 
					marker.points.push_back(p);
					marker_array.markers.push_back(marker);

					//Z axis
					marker.ns = "axes_z";
					marker.id = TID*10+view_i;
					marker.color.r = 0.0; marker.color.g = 0.0;	marker.color.b = 1.0; marker.color.a = 1.0; //blue color
					marker.points.erase(marker.points.begin(), marker.points.end());
					p.x = 0; p.y = 0; p.z = 0; 
					marker.points.push_back(p);
					
					if (sign > 0 )
					{
					  p.x = 0; p.y = 0; p.z = 1 * axis_dimension;  
					}
					else 
					{
					  p.x = 0; p.y = 0; p.z = 1 * -axis_dimension;
					}
					 
					marker.points.push_back(p);
					marker_array.markers.push_back(marker);
					ROS_INFO("axes added...");
				}
				
				}
				

		      neat_marker_publisher.publish(marker_array);
		      return 1;
		  }
		
	    	    
	    	    
	    	    
	    	    
	    template <typename T>
	    int set_neat_visualization_marker_array_object_descriptor( boost::shared_ptr<pcl::PointCloud<T> > all_projected_views, string object_frame_id, unsigned int TID )
	      {

		//STEP 1: need to get the position of the object so we can draw
		//text nearby
		//   tf::StampedTransform stf; //the transform
		//   //std::string tracker_frame_id =  msg->header.frame_id;
		//   std::string tracker_frame_id = "/perception/pipeline" + boost::lexical_cast<std::string>(msg->track_id) + "/tracker";

		std::string tracker_frame_id = "/perception/pipeline" + boost::lexical_cast<std::string>(TID) + "/tracker";
// 		ROS_INFO ("set_neat_visualization_marker_array_object_descriptor: object_frame_id = %s", object_frame_id.c_str());
		
		visualization_msgs::MarkerArray marker_array; 
// 		geometry_msgs::Point p;
		double duration = 5;
		bool locked = true;
		bool finish= true;
		float center_of_projected_cloud_x=0 , center_of_projected_cloud_y=0 , center_of_projected_cloud_z=0 ;
		geometry_msgs::Point center_of_projected_cloud;
		geometry_msgs::Vector3 dimensions;
		geometry_msgs::Point p;

		/* _________________________________
	      	   |                                 |
	      	   |   Draw Projected Point Cloud    |
	      	   |_________________________________| */
	      	if (1)
	      	{
		      visualization_msgs::Marker marker;
		      //marker.header.frame_id = object_frame_id;
		      marker.header.stamp = ros::Time();		
		      marker.frame_locked = locked;
		      marker.header.frame_id = tracker_frame_id;

		      marker.ns = "projected views";
		      marker.id = TID;
		      marker.type = visualization_msgs::Marker::POINTS;
		      marker.lifetime = Duration(duration);
		      marker.action = visualization_msgs::Marker::ADD;
		      marker.pose.position.x = 0;	marker.pose.position.y = 0;	marker.pose.position.z = 0;
		      marker.pose.orientation.x = 0.0; marker.pose.orientation.y = 0.0; marker.pose.orientation.z = 0.0; marker.pose.orientation.w = 1.0;
		      marker.color = cm->color(TID);
		      marker.scale.x = 0.005; marker.scale.y = 0.005; marker.scale.z = 0.005; marker.color.a = 1;
		      //marker.color.r = 1.0; marker.color.g = 1.0;	marker.color.b = 1.0; marker.color.a = 1.0; 
		      
		      //marker.points.erase(marker.points.begin(), marker.points.end());
		      geometry_msgs::Point p;

		      for (size_t i=0; i<all_projected_views->points.size(); i++)
		      {
			  p.x = all_projected_views->points.at(i).x;
			  p.y = all_projected_views->points.at(i).y;
			  p.z = all_projected_views->points.at(i).z;
			  marker.points.push_back(p);
			  center_of_projected_cloud_x += p.x;
			  center_of_projected_cloud_y += p.y;
			  center_of_projected_cloud_z += p.z;
		      }
		      center_of_projected_cloud.x= center_of_projected_cloud_x/all_projected_views->points.size();
		      center_of_projected_cloud.y= center_of_projected_cloud_y/all_projected_views->points.size();
		      center_of_projected_cloud.z= center_of_projected_cloud_z/all_projected_views->points.size();
		      
		      marker_array.markers.push_back(marker);
		}
	      
		      /* _________________________________
			|                                 |
			|             DRAW BBOX           |
			|_________________________________| */
		      if (1)
		      {
			      visualization_msgs::Marker marker;
			      marker.header.frame_id = tracker_frame_id;
			      marker.header.stamp = ros::Time();
		    
			      marker.ns = "boundingbox";
			      marker.id = TID;
			      marker.type = visualization_msgs::Marker::CUBE;
			      marker.frame_locked = locked;
			      //if (finish)
			      //marker.action = visualization_msgs::Marker::DELETE;
			      //else
			      marker.action = visualization_msgs::Marker::ADD;
			      marker.lifetime = Duration(5);
		    
			      
			      marker.pose.position =center_of_projected_cloud;
			      compute_bounding_box_dimensions(all_projected_views, dimensions);
			      ROS_INFO("box dimensions (x, y, z) = (%f, %f, %f) ", dimensions.x, dimensions.y, dimensions.z);
		      
			      marker.scale.x = dimensions.x+0.0; 
			      marker.scale.y = dimensions.y+0.2; 
			      marker.scale.z = dimensions.z+0.02; 
		    
			      marker.color = cm->color(TID);
						
			      marker.color.a = 0.3;
			      marker.color.r = 0.0;
			      marker.color.g = 0.5;
			      marker.color.b = 0.5;
			      if (finish)
			      {
				      marker.color.r = 0.9;
				      marker.color.g = 0.0;
				      marker.color.b = 0.0;
			      }
		    
			      marker_array.markers.push_back(marker);
		      }
		      /* _________________________________
			|                                 |
			|             DRAW WIREFRAME      |
			|_________________________________| */
		      if (1)
		      {
			
			      visualization_msgs::Marker marker;
			      marker.header.frame_id = tracker_frame_id;
			      marker.header.stamp = ros::Time();
		    
			      marker.ns = "wireframe";
			      marker.id = TID;
			      marker.frame_locked = locked;
			      marker.type = visualization_msgs::Marker::LINE_LIST;
			      //if (finish)
			      //marker.action = visualization_msgs::Marker::DELETE;
			      //else
			      marker.action = visualization_msgs::Marker::ADD;
			      marker.lifetime = Duration(5);
		    			      
			      marker.pose.position =center_of_projected_cloud;

			      //marker.pose = _tracked_object_msg.bounding_box.pose_stamped.pose;
		    
			      marker.scale.x = 0.005; 
			      double x = dimensions.x/2; 
			      double y = dimensions.y/2; 
			      double z = dimensions.z/2; 
		    
			      marker.color.a = 0.5;
			      marker.color.r = 0.5;
			      marker.color.g = 0.5;
			      marker.color.b = 0.5;
			      //marker
			      if (finish)
			      {
				      marker.color.r = 0.1;
				      marker.color.g = 0.1;
				      marker.color.b = 0.1;
			      }

			      //outside lines
			      			      
			      p.x =  x; p.y =  y; p.z = -z; marker.points.push_back(p);
			      p.x =  x; p.y =  y; p.z =  z; marker.points.push_back(p);
			      p.x =  x; p.y =  -y; p.z = -z; marker.points.push_back(p);
			      p.x =  x; p.y =  -y; p.z =  z; marker.points.push_back(p);
			      p.x =  x; p.y =  -y; p.z = z; marker.points.push_back(p);
			      p.x =  x; p.y =  y; p.z =  z; marker.points.push_back(p);
			      p.x =  x; p.y =  -y; p.z = -z; marker.points.push_back(p);
			      p.x =  x; p.y =  y; p.z =  -z; marker.points.push_back(p);
			      
			      
			      //inside	lines		      
			      p.x =  x; p.y =  y/2; p.z = -z; marker.points.push_back(p);
			      p.x =  x; p.y =  y/2; p.z =  z; marker.points.push_back(p);			      
			      p.x =  x; p.y =  0 ; p.z = -z; marker.points.push_back(p);
			      p.x =  x; p.y =  0; p.z =  z; marker.points.push_back(p);
			      p.x =  x; p.y =  -y/2; p.z = -z; marker.points.push_back(p);
			      p.x =  x; p.y =  -y/2; p.z =  z; marker.points.push_back(p);
			      
			    
			      p.x =  x; p.y =  -y; p.z = z/2; marker.points.push_back(p);
			      p.x =  x; p.y =  y; p.z =  z/2; marker.points.push_back(p);	
			      
			      p.x =  x; p.y =  -y; p.z = 0; marker.points.push_back(p);
			      p.x =  x; p.y =  y; p.z =  0; marker.points.push_back(p);	
			     
			      p.x =  x; p.y =  -y; p.z = -z/2; marker.points.push_back(p);
			      p.x =  x; p.y =  y; p.z =  -z/2; marker.points.push_back(p);	
			      
			      
			      
			      marker_array.markers.push_back(marker);
		      }
	      
	      	
// 		/* _________________________________
// 		    |                                 |
// 		    |           DRAW XYZ AXES         |
// 		    |_________________________________| */
// 		if (1)
// 		{	
// 			visualization_msgs::Marker marker;
// 			double axis_dimension = 0.9;
// 			marker.header.frame_id = object_frame_id.c_str();
// 			marker.header.stamp = ros::Time();
// 
// 			//marker.frame_locked = locked;
// 			marker.type = visualization_msgs::Marker::LINE_STRIP;
// 		      //  if (finish)
// 		      //	  marker.action = visualization_msgs::Marker::DELETE;
// 		      // else
// 				marker.action = visualization_msgs::Marker::ADD;
// 
// 			marker.scale.x = 0.05; marker.scale.y = 0.5; marker.scale.z = 4;
// 			marker.scale.x = 05; marker.scale.y = 0.5; marker.scale.z = 4; 
// 			marker.lifetime = Duration(duration);
// 
// 			
// 			//X axis
// 			marker.ns = "axes_x";
// 			marker.id = TID;
// 			marker.color.r = 1.0; marker.color.g = 0.0;	marker.color.b = 0.0; marker.color.a = 1.0; //red color
// 			marker.points.erase(marker.points.begin(), marker.points.end());
// 			p.x = 0; p.y = 0; p.z = 0; marker.points.push_back(p);
// 			p.x = 1 * axis_dimension; p.y = 0; p.z = 0; marker.points.push_back(p);
// 			marker_array.markers.push_back(marker);
// 
// 			//Y axis
// 			marker.ns = "axes_y";
// 			marker.id = TID;
// 			marker.color.r = 0.0; marker.color.g = 1.0;	marker.color.b = 0.0; marker.color.a = 1.0; //green color
// 			marker.points.erase(marker.points.begin(), marker.points.end());
// 			p.x = 0; p.y = 0; p.z = 0; marker.points.push_back(p);
// 			p.x = 0; p.y = 1 * axis_dimension; p.z = 0; marker.points.push_back(p);
// 			marker_array.markers.push_back(marker);
// 
// 			//Z axis
// 			marker.ns = "axes_z";
// 			marker.id = TID;
// 			marker.color.r = 0.0; marker.color.g = 0.0;	marker.color.b = 1.0; marker.color.a = 1.0; //blue color
// 			marker.points.erase(marker.points.begin(), marker.points.end());
// 			p.x = 0; p.y = 0; p.z = 0; marker.points.push_back(p);
// 			p.x = 0; p.y = 0; p.z = 1 * axis_dimension; marker.points.push_back(p);
// 			marker_array.markers.push_back(marker);
// 		}
// 
// 
// 		
// 		
		
		/* _________________________________
			|                                 |
			|         DRAW TEXT INFO          |
			|_________________________________| */
	      // 	if (1)
	      // 	{
	      // 		visualization_msgs::Marker marker;
	      // 		marker.header.frame_id = _tracker_frame_id;
	      // 		//marker.header.frame_id = _fixed_frame_id;
	      // 		marker.frame_locked = locked;
	      // 		marker.header.stamp = ros::Time();
	      // 		marker.ns = "information";
	      // 		marker.id = _track_id;
	      // 		marker.type = visualization_msgs::Marker::TEXT_VIEW_FACING;
	      // 		marker.lifetime = Duration(duration);
	      // 		marker.action = visualization_msgs::Marker::ADD;
	      // 		//if (finish)
	      // 		//marker.action = visualization_msgs::Marker::DELETE;
	      // 		//else
	      // 
	      // 		//marker.pose = _tracked_object_msg.bounding_box.pose_stamped.pose;
	      // 
	      // 		//marker.pose.position.x = 0 + marker.pose.position.x*0.9 + 0;
	      // 		//marker.pose.position.y = 0 + marker.pose.position.y*0.9 + 0.1;
	      // 		marker.pose.position.z = _tracked_object_msg.bounding_box.dimensions.z/2+0.1;
	      // 		marker.scale.z = 0.02; 
	      // 		//marker.color.r  = 1; marker.color.g  = 1; marker.color.b  = 1; marker.color.a = 1;
	      // 
	      // 		if (finish)
	      // 		{
	      // 			marker.color.r  = 0.5; marker.color.g  = 0; marker.color.b  = 0; marker.color.a = 1;
	      // 		}
	      // 		else
	      // 		{
	      // 			marker.color.r  = 0; marker.color.g  = 0; marker.color.b  = 0; marker.color.a = 1;
	      // 		}
	      // 
	      // 		marker.lifetime = Duration(duration);
	      // 
	      // 		char tmp_str[255]; 
	      // 		//sprintf(tmp_str,"\nfr=%0.1f ad=%0.2f vel=%0.2f t=%0.1f did=%d", _fit_ratio, _accumulated_distance, _velocity, _time_since_velocity_computation, _demonstrator_id);
	      // 		sprintf(tmp_str,"%0.1f", _fit_ratio);
	      // 
	      // 		marker.text = "TID" + boost::lexical_cast<std::string>(_track_id) + " V" + boost::lexical_cast<std::string>(view_count) + "(" + tmp_str + ")";
	      // 
	      // 		//marker.text.append(tmp_str);
	      // 		//char tmp_str1[255]; 
	      // 		//sprintf(tmp_str1,"\nr=%0.1f p=%0.1f y=%0.1f",diff_roll, diff_pitch, diff_yaw);
	      // 		//marker.text.append(tmp_str1);
	      // 
	      // 		marker.text.append("\n");
	      // 
	      // 		if (_state_is_moving)
	      // 		{
	      // 			marker.text.append("[M");
	      // 		}
	      // 		else
	      // 		{
	      // 			marker.text.append("[S");
	      // 		}
	      // 
	      // 		//marker.text.append("\n");
	      // 		if ((ros::Time::now() - _point_cloud_sent_tic).toSec() < 1.0 && (ros::Time::now() - _key_view_tic).toSec() < 1.0) 
	      // 			marker.text.append(",C,K]");
	      // 		else if ((ros::Time::now() - _point_cloud_sent_tic).toSec() < 1.0) 
	      // 			marker.text.append(",C,_]");
	      // 		else
	      // 			marker.text.append(",_,_]");
	      // 
	      // 		if (finish)	marker.text.append("\n(TRACKING LOST)");
	      // 
	      // 
	      // 		marker_array.markers.push_back(marker);
	      // 	}

	      // 	/* _________________________________
	      // 	   |                                 |
	      // 	   |             DRAW WIREFRAME      |
	      // 	   |_________________________________| */
	      // 	if (1)
	      // 	{
	      // 		visualization_msgs::Marker marker;
	      // 		marker.header.frame_id = _tracker_frame_id;
	      // 		marker.header.stamp = ros::Time();
	      // 
	      // 		marker.ns = "wireframe";
	      // 		marker.id = _track_id;
	      // 		marker.frame_locked = locked;
	      // 		marker.type = visualization_msgs::Marker::LINE_LIST;
	      // 		//if (finish)
	      // 		//marker.action = visualization_msgs::Marker::DELETE;
	      // 		//else
	      // 		marker.action = visualization_msgs::Marker::ADD;
	      // 		marker.lifetime = Duration(duration);
	      // 
	      // 		//marker.pose = _tracked_object_msg.bounding_box.pose_stamped.pose;
	      // 
	      // 		marker.scale.x = 0.005; 
	      // 		double x = _tracked_object_msg.bounding_box.dimensions.x/2; 
	      // 		double y = _tracked_object_msg.bounding_box.dimensions.y/2; 
	      // 		double z = _tracked_object_msg.bounding_box.dimensions.z/2; 
	      // 
	      // 		_color.a = 0.5;
	      // 		marker.color = _color;
	      // 		marker.color.r = 0.5;
	      // 		marker.color.g = 0.5;
	      // 		marker.color.b = 0.5;
	      // 		//marker
	      // 		if (finish)
	      // 		{
	      // 			marker.color.r = 0.1;
	      // 			marker.color.g = 0.1;
	      // 			marker.color.b = 0.1;
	      // 		}
	      // 
	      // 		p.x =  x; p.y =  y; p.z =  z; marker.points.push_back(p);
	      // 		p.x = -x; p.y =  y; p.z =  z; marker.points.push_back(p);
	      // 		p.x =  x; p.y =  y; p.z = -z; marker.points.push_back(p);
	      // 		p.x = -x; p.y =  y; p.z = -z; marker.points.push_back(p);
	      // 		p.x =  x; p.y =  y; p.z = -z; marker.points.push_back(p);
	      // 		p.x =  x; p.y =  y; p.z =  z; marker.points.push_back(p);
	      // 		p.x = -x; p.y =  y; p.z = -z; marker.points.push_back(p);
	      // 		p.x = -x; p.y =  y; p.z =  z; marker.points.push_back(p);
	      // 
	      // 		p.x =  x; p.y =  -y; p.z =  z; marker.points.push_back(p);
	      // 		p.x = -x; p.y =  -y; p.z =  z; marker.points.push_back(p);
	      // 		p.x =  x; p.y =  -y; p.z = -z; marker.points.push_back(p);
	      // 		p.x = -x; p.y =  -y; p.z = -z; marker.points.push_back(p);
	      // 		p.x =  x; p.y =  -y; p.z = -z; marker.points.push_back(p);
	      // 		p.x =  x; p.y =  -y; p.z =  z; marker.points.push_back(p);
	      // 		p.x = -x; p.y =  -y; p.z = -z; marker.points.push_back(p);
	      // 		p.x = -x; p.y =  -y; p.z =  z; marker.points.push_back(p);
	      // 
	      // 		p.x =  x; p.y =  y; p.z =  z; marker.points.push_back(p);
	      // 		p.x =  x; p.y = -y; p.z =  z; marker.points.push_back(p);
	      // 		p.x =  x; p.y =  y; p.z = -z; marker.points.push_back(p);
	      // 		p.x =  x; p.y = -y; p.z = -z; marker.points.push_back(p);
	      // 
	      // 		p.x = -x; p.y =  y; p.z =  z; marker.points.push_back(p);
	      // 		p.x = -x; p.y = -y; p.z =  z; marker.points.push_back(p);
	      // 		p.x = -x; p.y =  y; p.z = -z; marker.points.push_back(p);
	      // 		p.x = -x; p.y = -y; p.z = -z; marker.points.push_back(p);
	      // 
	      // 		marker_array.markers.push_back(marker);
	      // 	}
	      // 
	      // 
	      // 	/* _________________________________
	      // 	   |                                 |
	      // 	   |             DRAW BBOX           |
	      // 	   |_________________________________| */
	      // 	if (1)
	      // 	{
	      // 		visualization_msgs::Marker marker;
	      // 		marker.header.frame_id = _tracker_frame_id;
	      // 		marker.header.stamp = ros::Time();
	      // 
	      // 		marker.ns = "boundingbox";
	      // 		marker.id = _track_id;
	      // 		marker.type = visualization_msgs::Marker::CUBE;
	      // 		marker.frame_locked = locked;
	      // 		//if (finish)
	      // 		//marker.action = visualization_msgs::Marker::DELETE;
	      // 		//else
	      // 		marker.action = visualization_msgs::Marker::ADD;
	      // 		marker.lifetime = Duration(duration);
	      // 
	      // 		//marker.pose = _tracked_object_msg.bounding_box.pose_stamped.pose;
	      // 
	      // 		marker.scale.x = _tracked_object_msg.bounding_box.dimensions.x; 
	      // 		marker.scale.y = _tracked_object_msg.bounding_box.dimensions.y; 
	      // 		marker.scale.z = _tracked_object_msg.bounding_box.dimensions.z; 
	      // 
	      // 		_color.a = 0.1;
	      // 		marker.color = _color;
	      // 		if (finish)
	      // 		{
	      // 			marker.color.r = 0.9;
	      // 			marker.color.g = 0.0;
	      // 			marker.color.b = 0.0;
	      // 		}
	      // 
	      // 		marker_array.markers.push_back(marker);
	      // 	}

		      neat_marker_publisher.publish(marker_array);
		      return 1;


            void Time();
		  }
//            void ROS_INFO(const char* arg1, int adaptive_support_lenght);
            //void ROS_INFO(const char* arg1, int adaptive_support_lenght);
			//  void rot_mat(int arg1);


    };

    
    
    class ObjectDescriptorNodelet: public ObjectDescriptor<pcl::PointXYZRGBA>{};
//    PLUGINLIB_DECLARE_CLASS(race_object_descriptor, ObjectDescriptorNodelet, race_object_descriptor::ObjectDescriptorNodelet, nodelet::Nodelet);
   PLUGINLIB_EXPORT_CLASS(race_object_descriptor::ObjectDescriptorNodelet, nodelet::Nodelet);
}//end feature_extraction namespace
#endif


		//STEP 0: compute the transform between two coordinate frames.
/*		try
		{
			_p_transform_listener->lookupTransform(_fixed_frame_id_tmp, _table_frame_id, ros::Time(0), stf);
		}
		catch (tf::TransformException &ex)
		{
			ROS_ERROR("Could not get table frame_id. Will not update tracker reference cloud. Could not get skeleton tf. tf error was: %s",  ex.what());
		}
		
		//br.sendTransform(tf::StampedTransform(computedTransform, ros::Time::now(), "parent name e.g world", transformName));
		_br->sendTransform(StampedTransform(stf, Time::now(), _fixed_frame_id_tmp, "STF"));
			*/	    	  


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 		boost::shared_ptr<pcl::PointCloud<PointT> > initial_cloud_proj_x (new PointCloud<PointT>);//Declare a boost share ptr to the pointCloud
// 		boost::shared_ptr<pcl::PointCloud<PointT> > initial_cloud_proj_y (new PointCloud<PointT>);//Declare a boost share ptr to the pointCloud
// 		boost::shared_ptr<pcl::PointCloud<PointT> > initial_cloud_proj_z (new PointCloud<PointT>);//Declare a boost share ptr to the pointCloud
// 		bool visualize = false;
// 		PointXYZ pt; 
// 		boost::shared_ptr<pcl::PointCloud<PointT> > all_projected_views (new PointCloud<PointT>);//Declare a boost share ptr to the pointCloud
// 		vector < boost::shared_ptr<pcl::PointCloud<PointT> > > vector_of_projected_views;
// 
// 		
// // 		boost::shared_ptr<PointCloud<PointXYZ> > new_vec_z = (boost::shared_ptr<PointCloud<PointXYZ> >) new PointCloud<PointXYZ>;
// // 		Eigen::Affine3d eigen_trf;
// // 		// Obtain the aligned point cloud
// // 		tf::TransformTFToEigen (stf, eigen_trf);
// // 		pcl::transformPointCloud<PointXYZ>(*vec_z, *new_vec_z, eigen_trf);
// // 		
// 		
// // 		pt.x = 1; pt.y = 0.0; pt.z = 0.0;
// // 		project_objectview_to_plane (  target_pc, 
// // 					       pt,
// // 					       transf_object,
// // 					       initial_cloud_proj_x, 
// // 					       visualize);
// // // 		for (int i=0; i< initial_cloud_proj_x->points.size(); i++)
// // // 		{
// // // 		    initial_cloud_proj_x->points.at(i).x = 0.3;
// // // 		}
// // 		vector_of_projected_views.push_back(initial_cloud_proj_x);
// // // 		
// // 
// // 		pt.x = 0.0; pt.y = 1; pt.z = 0.0;
// // 		project_objectview_to_plane ( target_pc, 
// // 					      pt,
// // 					      transf_object,
// // 					      initial_cloud_proj_y, 
// // 					      visualize);
// // // 		for (int i=0; i< initial_cloud_proj_y->points.size(); i++)
// // // 		{
// // // 		    initial_cloud_proj_y->points.at(i).y = 0.3;
// // // 		}
// // 		
// // 		vector_of_projected_views.push_back(initial_cloud_proj_y);
// // 
// // 		pt.x = 0.0; pt.y = 0.0; pt.z = 1;
// // 		project_objectview_to_plane ( target_pc, 
// // 					      pt,
// // 					      transf_object,
// // 					      initial_cloud_proj_z, 
// // 					      visualize);
// // // 		for (int i=0; i< initial_cloud_proj_z->points.size(); i++)
// // // 		{
// // // 		     initial_cloud_proj_z->points.at(i).z = 0.30;
// // // 		}
// // 		vector_of_projected_views.push_back(initial_cloud_proj_z);
// 
// 		
// 		//We fill in the ModelCoefficients values. In this case, we use a plane model, with ax+by+cz+d=0, where a=b=d=0, and c=1, or said differently, the X-Y plane.
// // 		  pcl::ModelCoefficients::Ptr coefficientsX (new pcl::ModelCoefficients ());
// // 		  coefficientsX->values.resize (4);
// // 		  coefficientsX->values[0] = 1.0;
// // 		  coefficientsX->values[1] = 0;
// // 		  coefficientsX->values[2] = 0;
// // 		  coefficientsX->values[3] = 0;
// // 		
// // 		project_pc_to_plane(target_pc, coefficientsX, initial_cloud_proj_x);
// // 		for (int i=0; i< initial_cloud_proj_x->points.size(); i++)
// // 		{
// // 		    initial_cloud_proj_x->points.at(i).x = 0.3;
// // 		}
// // 		vector_of_projected_views.push_back(initial_cloud_proj_x);
// // 
// // 		
// // 				  //We fill in the ModelCoefficients values. In this case, we use a plane model, with ax+by+cz+d=0, where a=b=d=0, and c=1, or said differently, the X-Y plane.
// // 		  pcl::ModelCoefficients::Ptr coefficientsY (new pcl::ModelCoefficients ());
// // 		  coefficientsY->values.resize (4);
// // 		  coefficientsY->values[0] = 0.0;
// // 		  coefficientsY->values[1] = 1.0;
// // 		  coefficientsY->values[2] = 0;
// // 		  coefficientsY->values[3] = 0;
// // 
// // 		project_pc_to_plane(target_pc, coefficientsY, initial_cloud_proj_y);
// // 		for (int i=0; i< initial_cloud_proj_y->points.size(); i++)
// // 		{
// // 		    initial_cloud_proj_y->points.at(i).y = 0.3;
// // 		}
// // 		vector_of_projected_views.push_back(initial_cloud_proj_y);
// // 
// // 		 // *initial_cloud_proj_y = cPoints;
// // 		
// // // 		  vector_of_projected_views.push_back(initial_cloud_proj_y);
// // 		  	  
// // 		  //We fill in the ModelCoefficients values. In this case, we use a plane model, with ax+by+cz+d=0, where a=b=d=0, and c=1, or said differently, the X-Y plane.
// // 		  pcl::ModelCoefficients::Ptr coefficientsZ (new pcl::ModelCoefficients ());
// // 		  coefficientsZ->values.resize (4);
// // 		  coefficientsZ->values[0] = 0;
// // 		  coefficientsZ->values[1] = 0;
// // 		  coefficientsZ->values[2] = 1.0;
// // 		  coefficientsZ->values[3] = 0;
// // 
// // 		
// // 		  project_pc_to_plane(target_pc, coefficientsZ, initial_cloud_proj_z);
// // 		  for (int i=0; i< initial_cloud_proj_z->points.size(); i++)
// // 		  {
// // 		      initial_cloud_proj_z->points.at(i).z = 0.30;
// // 		  }		
// // 		  
// // 		  vector_of_projected_views.push_back(initial_cloud_proj_z);
// 		
// 		  //We create the ProjectInliers object and use the ModelCoefficients defined above as the model to project onto.		  
// // 		  //pcl::ProjectInliers<pcl::PointXYZ> proj;
// // 		  proj.setModelType (pcl::SACMODEL_PLANE);
// // 		  proj.setInputCloud (target_pc);
// // 		  proj.setModelCoefficients (coefficientsZ);
// // 		  proj.filter (*initial_cloud_proj_z);
// // 
// // 		  //*initial_cloud_proj_z = cPoints;
// // 		  for (int i=0; i< initial_cloud_proj_z->points.size(); i++)
// // 		  {
// // 		      initial_cloud_proj_z->points.at(i).z = 0.1;
// // 		  }		
// // 		  
// // 		  vector_of_projected_views.push_back(initial_cloud_proj_z);
// // 		  
// 		  
// 		
// // 		pt.x = 0.25; pt.y = 0.0; pt.z = 0.0;
// // // // 		pt.x = 0.001; pt.y = 0.0; pt.z = 0.0;
// // // 		
// // 		project_objectview_to_plane (  target_pc, 
// // 					       pt,
// // 					       stf,
// // 					       initial_cloud_proj_x, 
// // 					       visualize);
// // // 
// // 		vector_of_projected_views.push_back(initial_cloud_proj_x);
// // 		//set_neat_visualization_marker_array_object_descriptor_projected_plane(all_projected_views, msg->header.frame_id, msg -> track_id );
// // 		//set_neat_visualization_marker_array_object_descriptor(all_projected_views, msg->header.frame_id, msg -> track_id );
// //  		*all_projected_views += *initial_cloud_proj_x;
// // 	    	
// // 		pt.x = 0.0; pt.y = 5.5; pt.z = 0.0;
// // // 		pt.x = 0.0; pt.y = 0.35; pt.z = 0.0;
// // 		project_objectview_to_plane ( target_pc, 
// // 					      pt,
// // 					      stf,
// // 					      initial_cloud_proj_y, 
// // 					      visualize);
// // 		vector_of_projected_views.push_back(initial_cloud_proj_y);
// // 
// // 		//set_neat_visualization_marker_array_object_descriptor(initial_cloud_proj, msg->header.frame_id, msg -> track_id );
// // 		*all_projected_views += *initial_cloud_proj_y;		
// // 		
// // 		pt.x = 0.0; pt.y = 0.0; pt.z = 0.35;
// // 		project_objectview_to_plane ( target_pc, 
// // 					      pt,
// // 					      stf,
// // 					      initial_cloud_proj_z, 
// // 					      visualize);
// 		
// 		
// 		  ///////////////////////////////////////////////// the theory of new shape descriptor ////////////////////////////////////////////////////////////////
// 		  // //NOTE  the PCA base reference frame construction basically does:
// 		  // 1) compute the centroid (c0, c1, c2) and the normalized covariance
// 		  // 2) compute the eigenvectors e0, e1, e2. The reference system will be (e0, e1, e0 X e1) --- note: e0 X e1 = +/- e2
// 		  // 3) move the points in that RF --- note: the transformation given by the rotation matrix (e0, e1, e0 X e1) & (c0, c1, c2) must be inverted
// 		  // 4) compute the max, the min and the center of the diagonal (mean_diag)
// 		  // 5) given a box centered at the origin with size (max_pt.x - min_pt.x, max_pt.y - min_pt.y, max_pt.z - min_pt.z) 
// 		  //    the transformation you have to apply is Rotation = (e0, e1, e0 X e1) & Translation = Rotation * mean_diag + (c0, c1, c2)
// 
// 		  
// 		  // compute principal direction
// 		  Eigen::Vector4f centroid;
// 		  pcl::compute3DCentroid(*target_pc, centroid);
// 		  Eigen::Matrix3f covariance;
// 		  computeCovarianceMatrixNormalized(*target_pc, centroid, covariance);
// 		  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> eigen_solver(covariance, Eigen::ComputeEigenvectors);
// 		  Eigen::Matrix3f eigVectors = eigen_solver.eigenvectors();
// 		  eigVectors.col(2) = eigVectors.col(0).cross(eigVectors.col(1));
// 
// 		  //TODO : create a print function for covariance matrix 
// 		  std::cout << "column1: " << eigVectors.col(0) << std::endl;
// 		  std::cout << "column2: " << eigVectors.col(1) << std::endl;
// 		  std::cout << "column3: " << eigVectors.col(2) << std::endl;
// 		  
// 		  //std::cout << "first element of the first col: " << eigVectors(0,0) << std::endl;
// 		  //std::cout << "second element of the first col: " << eigVectors(1,0) << std::endl;  
// 		  //std::cout << "third element of the first col:" << eigVectors(2,0) << std::endl;  
// 
// 		  Eigen::Vector3f eigen_values = eigen_solver.eigenvalues();
// 		  std::cout << "The eigenvalues of the covariance matrix are:" << eigen_values << endl;
// // 		  std::cout << "The eigenvalue of the X axis is :" << eigen_values(0,0) << endl;
// // 		  std::cout << "The eigenvalue of the Y axis is :" << eigen_values(0,1) << endl;
// // 		  std::cout << "The eigenvalue of the Z axis is :" << eigen_values(0,2) << endl;
// 
// 		  // move the points to the PCA based reference frame
// 		  Eigen::Matrix4f p2w(Eigen::Matrix4f::Identity());
// 		  p2w.block<3,3>(0,0) = eigVectors.transpose();
// 		  p2w.block<3,1>(0,3) = -1.f * (p2w.block<3,3>(0,0) * centroid.head<3>());
// 		  
// 		  //Declare a boost share ptr to the PCA_pointCloud
// 		  boost::shared_ptr<PointCloud<PointT> > pca_pc (new PointCloud<PointT>); 
// 		  
// 		  pcl::PointCloud<PointT> cPoints;
// 		  pcl::transformPointCloud(*target_pc, *pca_pc, p2w);
// 
// 		  //compute the max, the min and the center of the diagonal (mean_diag)
// 		  PointT min_pt, max_pt;
// 		  pcl::getMinMax3D(*pca_pc, min_pt, max_pt);
// 		  const Eigen::Vector3f mean_diag = 0.5f*(max_pt.getVector3fMap() + min_pt.getVector3fMap());
// 
// // 		  // final transform
// 		  Eigen::Quaternionf qfinal(eigVectors);//rotation matrix
// 		  Eigen::Vector3f center_of_bbox = eigVectors*mean_diag + centroid.head<3>(); // Translation = Rotation * center_diag + (c0, c1, c2)
// 
// 		  std::cout << "center of box (x,y,z) =" << center_of_bbox << std::endl;
// 
//   
// 		  
// 		//We fill in the ModelCoefficients values. In this case, we use a plane model, with ax+by+cz+d=0, where a=b=d=0, and c=1, or said differently, the X-Y plane.
// 		pcl::ModelCoefficients::Ptr coefficientsX (new pcl::ModelCoefficients ());
// 		coefficientsX->values.resize (4);
// 		coefficientsX->values[0] = 1.0;
// 		coefficientsX->values[1] = 0;
// 		coefficientsX->values[2] = 0;
// 		coefficientsX->values[3] = 0;
// 		
// 		project_pc_to_plane(pca_pc, coefficientsX, initial_cloud_proj_x);
// 		for (int i=0; i< initial_cloud_proj_x->points.size(); i++)
// 		{
// 		    initial_cloud_proj_x->points.at(i).x = 0.3;
// 		}
// 		vector_of_projected_views.push_back(initial_cloud_proj_x);
// 
// 		
// 		//We fill in the ModelCoefficients values. In this case, we use a plane model, with ax+by+cz+d=0, where a=b=d=0, and c=1, or said differently, the X-Y plane.
// 		  pcl::ModelCoefficients::Ptr coefficientsY (new pcl::ModelCoefficients ());
// 		  coefficientsY->values.resize (4);
// 		  coefficientsY->values[0] = 0.0;
// 		  coefficientsY->values[1] = 1.0;
// 		  coefficientsY->values[2] = 0;
// 		  coefficientsY->values[3] = 0;
// 
// 		project_pc_to_plane(pca_pc, coefficientsY, initial_cloud_proj_y);
// 		for (int i=0; i< initial_cloud_proj_y->points.size(); i++)
// 		{
// 		    initial_cloud_proj_y->points.at(i).y = 0.3;
// 		}
// 		vector_of_projected_views.push_back(initial_cloud_proj_y);
// 
// 		  	  
// 		  //We fill in the ModelCoefficients values. In this case, we use a plane model, with ax+by+cz+d=0, where a=b=d=0, and c=1, or said differently, the X-Y plane.
// 		  pcl::ModelCoefficients::Ptr coefficientsZ (new pcl::ModelCoefficients ());
// 		  coefficientsZ->values.resize (4);
// 		  coefficientsZ->values[0] = 0;
// 		  coefficientsZ->values[1] = 0;
// 		  coefficientsZ->values[2] = 1.0;
// 		  coefficientsZ->values[3] = 0;
// 
// 		
// 		  project_pc_to_plane(pca_pc, coefficientsZ, initial_cloud_proj_z);
// 		  for (int i=0; i< initial_cloud_proj_z->points.size(); i++)
// 		  {
// 		      initial_cloud_proj_z->points.at(i).z = 0.30;
// 		  }		
// 		  
// 		  vector_of_projected_views.push_back(initial_cloud_proj_z);
// 	      
// 		  
// 		  		
// 		geometry_msgs::Vector3 dimensions;
// 		compute_bounding_box_dimensions(pca_pc, dimensions);
// 		ROS_INFO("++++++++++box dimensions (x, y, z) = (%f, %f, %f) ", dimensions.x, dimensions.y, dimensions.z);
// 		
// 		double largest_side;
// 		  if ((dimensions.y >= dimensions.x))
// 		  {
// 		    if (dimensions.y >= dimensions.z)
// 		    {
// 		      largest_side = dimensions.y;
// 		    }
// 		    else if (dimensions.z >= dimensions.x)
// 		    {
// 		      largest_side= dimensions.z;
// 		    }
// 		    
// 		  } 
// 		  else if(dimensions.z >= dimensions.x)
// 		  {
// 		    if (dimensions.z >= dimensions.y)
// 		    {
// 		      largest_side = dimensions.z;
// 		    }
// 		  }
// 		  else
// 		  {
// 		    largest_side = dimensions.x;
// 		  }
// 		  
// 		  if (adaptive_support_lenght == 0)
// 		  {
// 		    largest_side +=0.02;
// 		    ROS_INFO("Dimension of the (largest_side+0.02)/2 is %f m", largest_side/2);
// 		    ROS_INFO("adaptive_support_lenght = %d", adaptive_support_lenght);
// 		  }
// 		  else
// 		  {
// 		    largest_side = global_image_width;
// 		    ROS_INFO("global_image_width = %f", largest_side);
// 		    ROS_INFO("adaptive_support_lenght = %d", adaptive_support_lenght);
// 		  }
// 		   
// 		  largest_side =0;
// 		  compute_largest_side_of_bounding_box (dimensions, largest_side);// returens largest_side + 2cm
// 
// 		  ROS_INFO("Dimension of the (largest_side+0.02)/2 is %f m", largest_side/2);
// 
// 		  
// 		  //draw the cloud and the box
// 
// // 		  //draw the cloud and the box
// // // 		  pcl::visualization::PCLVisualizer viewer;
// // // 		  viewer.addPointCloud(target_pc);
// // // 		  viewer.addCube(tfinal, qfinal, max_pt.x - min_pt.x, max_pt.y - min_pt.y, max_pt.z - min_pt.z);
// // // 		  viewer.spin();
// // // 		  viewer.addPointCloud(target_pc);
// // 
// // 		  boost::shared_ptr<PointCloud<PointT> > XProjectionPoints (new PointCloud<PointT>); //Declare a boost share ptr to the pointCloud
// // 		  
// 		  
// // 		  //We fill in the ModelCoefficients values. In this case, we use a plane model, with ax+by+cz+d=0, where a=b=d=0, and c=1, or said differently, the X-Y plane.
// // 		  pcl::ModelCoefficients::Ptr coefficientsX (new pcl::ModelCoefficients ());
// // 		  coefficientsX->values.resize (4);
// // 		  coefficientsX->values[0] = 1.0;
// // 		  coefficientsX->values[1] = 0;
// // 		  coefficientsX->values[2] = 0;
// // 		  coefficientsX->values[3] = 0;
// 
// // 		  Eigen::Affine3f* trans ;
// 		  //ModelCoefficients::Ptr coefficients;
// 		// get_transform_from_pca_and_plane(target_pc,initial_cloud_proj_x, trans, coefficientsX);
// 		    
// 
// 		  //We create the ProjectInliers object and use the ModelCoefficients defined above as the model to project onto.		  
// // 		  pcl::ProjectInliers<PointT> proj;
// // 		  proj.setModelType (pcl::SACMODEL_PLANE);
// // 		  proj.setInputCloud (target_pc);
// // 		  proj.setModelCoefficients (coefficientsX);
// // 		  proj.filter (*initial_cloud_proj_x);
// // 		  
// 		  //*initial_cloud_proj_x = cPoints;
// // 		  for (int i=0; i< initial_cloud_proj_x->points.size(); i++)
// // 		  {
// // 		      initial_cloud_proj_x->points.at(i).x = 0.1;
// // 		  }		
// // 		  
// // 		  vector_of_projected_views.push_back(initial_cloud_proj_x);
// // 		  
// // 		  
// // 		  //We fill in the ModelCoefficients values. In this case, we use a plane model, with ax+by+cz+d=0, where a=b=d=0, and c=1, or said differently, the X-Y plane.
// // 		  pcl::ModelCoefficients::Ptr coefficientsY (new pcl::ModelCoefficients ());
// // 		  coefficientsY->values.resize (4);
// // 		  coefficientsY->values[0] = 0.0;
// // 		  coefficientsY->values[1] = 1.0;
// // 		  coefficientsY->values[2] = 0;
// // 		  coefficientsY->values[3] = 0;
// // 
// // 		  //We create the ProjectInliers object and use the ModelCoefficients defined above as the model to project onto.		  
// // 		  //pcl::ProjectInliers<pcl::PointXYZ> proj;
// // 		  proj.setModelType (pcl::SACMODEL_PLANE);
// // 		  proj.setInputCloud (target_pc);
// // 		  proj.setModelCoefficients (coefficientsY);
// // 		  proj.filter (*initial_cloud_proj_y);
// // 
// // 		 // *initial_cloud_proj_y = cPoints;
// // 		  for (int i=0; i< initial_cloud_proj_y->points.size(); i++)
// // 		  {
// // 		      initial_cloud_proj_y->points.at(i).y = 0.1;
// // 		  }		
// // 		  vector_of_projected_views.push_back(initial_cloud_proj_y);
// // 		  	  
// // 		  //We fill in the ModelCoefficients values. In this case, we use a plane model, with ax+by+cz+d=0, where a=b=d=0, and c=1, or said differently, the X-Y plane.
// // 		  pcl::ModelCoefficients::Ptr coefficientsZ (new pcl::ModelCoefficients ());
// // 		  coefficientsZ->values.resize (4);
// // 		  coefficientsZ->values[0] = 0;
// // 		  coefficientsZ->values[1] = 0;
// // 		  coefficientsZ->values[2] = 1.0;
// // 		  coefficientsZ->values[3] = 0;
// // 
// // 		  //We create the ProjectInliers object and use the ModelCoefficients defined above as the model to project onto.		  
// // 		  //pcl::ProjectInliers<pcl::PointXYZ> proj;
// // 		  proj.setModelType (pcl::SACMODEL_PLANE);
// // 		  proj.setInputCloud (target_pc);
// // 		  proj.setModelCoefficients (coefficientsZ);
// // 		  proj.filter (*initial_cloud_proj_z);
// // 
// // 		  //*initial_cloud_proj_z = cPoints;
// // 		  for (int i=0; i< initial_cloud_proj_z->points.size(); i++)
// // 		  {
// // 		      initial_cloud_proj_z->points.at(i).z = 0.1;
// // 		  }		
// // 		  
// // 		  vector_of_projected_views.push_back(initial_cloud_proj_z);
// // 		  
// 		  
// 		  
// 		  /*
// 		  *initial_cloud_proj_x = cPoints;
// 		  for (int i=0; i< initial_cloud_proj_x->points.size(); i++)
// 		  {
// 		      initial_cloud_proj_x->points.at(i).x = 0.3;
// 		  }		
// 		  vector_of_projected_views.push_back(initial_cloud_proj_x);
// 
// 		  
// 		  
// 		  *initial_cloud_proj_y = cPoints;
// 		  for (int i=0; i< initial_cloud_proj_y->points.size(); i++)
// 		  {
// 		      initial_cloud_proj_y->points.at(i).y = 0.3;
// 		  }		
// 		  vector_of_projected_views.push_back(initial_cloud_proj_y);
// 		  
// 		  *initial_cloud_proj_z = cPoints;
// 		  for (int i=0; i< initial_cloud_proj_z->points.size(); i++)
// 		  {
// 		      initial_cloud_proj_z->points.at(i).z = 0.3;
// 		  }		
// 		  vector_of_projected_views.push_back(initial_cloud_proj_z);
// 		  */
// 
// 		  
// // 		  pcl::visualization::PCLVisualizer viewer;
// // 		  viewer.addPointCloud(target_pc);
// // 		  PointT minimum_pt;
// // 		  PointT maximum_pt;
// // 		  getMinMax3D(*target_pc, minimum_pt, maximum_pt); // min max for bounding box
// // 		  viewer.addCube(minimum_pt.x, maximum_pt.x, minimum_pt.y, maximum_pt.y, minimum_pt.z, maximum_pt.z);
// // 		  viewer.spin();
// // 
// // 		  boost::shared_ptr<PointCloud<PointT> > projected_pcs (new PointCloud<PointT>);
// // 		  *projected_pcs += *initial_cloud_proj_x;
// // 		  *projected_pcs += *initial_cloud_proj_y;
// // 		  *projected_pcs += *initial_cloud_proj_z;
// // 		  pcl::visualization::PCLVisualizer viewer2;		  
// // 		  viewer2.addPointCloud(projected_pcs);
// // 		  viewer2.spin();
// 		  
// // 			  
// 		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 		
// 		
// 		//vector_of_projected_views.push_back(initial_cloud_proj_x);
// 		//TODO put a flag parameters in if
// 		if ((signDisambiguationFlag == false) or (off_line_flag == 0))
// 		{
// // 		    ROS_INFO("signDisambiguationFlag FALSE");
// // 		    signDisambiguation(initial_cloud_proj_z, threshold, sign );
// // 		    signDisambiguationFlag = true;
// // 		    ROS_INFO("signDisambiguationFlag is updated");
// 		    ROS_INFO("signDisambiguationFlag FALSE");
// 		    int Sx=1, Sy=1;
// 		    XsignDisambiguation(initial_cloud_proj_y, threshold, Sx );//XoZ Plane
// 		    YsignDisambiguation(initial_cloud_proj_x, threshold, Sy );//YoZ Plane
// 		    sign = Sx * Sy;
// 		    ROS_INFO("sign = Sx (%d) * Sy(%d) = %d", Sx, Sy, sign);
// 		    signDisambiguationFlag = true;
// 		    ROS_INFO("signDisambiguationFlag is updated");
// 		}
// 		else 
// 		{
// 		  ROS_INFO("signDisambiguationFlag TRUE");
// 		  ROS_INFO("signDisambiguationFlag is NOT updated");
// 		}
// 	
// 		
// 		vector <int> complete_object_histogram;
// 		vector <int> complete_object_histogram_normalized;//each projection view is normalized sepreatly
// 
// 		vector < vector<int> > XOZ_histogram;      
// 		vector < vector<int> > YOZ_histogram;
// 		
// 		
// 		vector <float> view_point_entropy;
// 		vector <vector <float> > normalized_projected_views;
// 
// 		//projection along X axis
// 		YOZ2DObjectHistogram( initial_cloud_proj_x,
// 				      largest_side, 
// 				      number_of_bins, 
// 				      sign,
// 				      YOZ_histogram);
// 		
// 		vector <int> histogramYOZ1D;
// 		convert2DhistogramTo1Dhisftogram(YOZ_histogram, histogramYOZ1D);
// 		//printHistogram ( histogramYOZ1D, "YOZ");
// 		complete_object_histogram.insert(complete_object_histogram.end(), histogramYOZ1D.begin(), histogramYOZ1D.end());
// 		vector <float> normalized_histogramYoZ;
// 		normalizingHistogram( histogramYOZ1D, normalized_histogramYoZ);
// 		normalized_projected_views.push_back(normalized_histogramYoZ);
// 		
// 		//printHistogram ( normalized_histogramYoZ, "normalized YOZ");
// 		complete_object_histogram_normalized.insert(complete_object_histogram_normalized.end(), normalized_histogramYoZ.begin(), normalized_histogramYoZ.end());
// 		float YoZ_entropy = 0;
// 		//viewpointEntropy(normalized_histogramYoZ, YoZ_entropy);
// 		viewpointEntropyNotNormalized(histogramYOZ1D, YoZ_entropy);
// 
// 		ROS_INFO("viewpointEntropyYoZ = %f", YoZ_entropy);
// 		view_point_entropy.push_back(YoZ_entropy);
// 
// 		//projection along Y axis
// 		XOZ2DObjectHistogram( initial_cloud_proj_y,
// 				      largest_side, 
// 				      number_of_bins, 
// 				      sign,
// 				      XOZ_histogram);
//  	      
// 		vector <int> histogramXOZ1D;
// 		convert2DhistogramTo1Dhisftogram(XOZ_histogram, histogramXOZ1D);
// 		//printHistogram ( histogramXOZ1D, "XOZ");
// 		complete_object_histogram.insert(complete_object_histogram.end(), histogramXOZ1D.begin(), histogramXOZ1D.end());
// 
// 		vector <float> normalized_histogramXoZ;
// 		normalizingHistogram( histogramXOZ1D, normalized_histogramXoZ);
// 		//printHistogram ( normalized_histogramXoZ, "normalized XOZ");
// 		normalized_projected_views.push_back(normalized_histogramXoZ);
// 
// 		complete_object_histogram_normalized.insert(complete_object_histogram_normalized.end(), 
// 							   normalized_histogramXoZ.begin(), 
// 							   normalized_histogramXoZ.end());
// 		float XoZ_entropy = 0;
// 		//viewpointEntropy(normalized_histogramXoZ, XoZ_entropy);
// 		viewpointEntropyNotNormalized(histogramXOZ1D, XoZ_entropy);
// 		ROS_INFO("viewpointEntropyXoZ = %f", XoZ_entropy);
// 		view_point_entropy.push_back(XoZ_entropy);
// 
// 
// 		//projection along Z axis
// 		vector < vector<int> > XOY_histogram;		
// 	        XOY2DObjectHistogram( initial_cloud_proj_z,
// 					largest_side, 
// 					number_of_bins, 
// 					sign,
// 					XOY_histogram);
// 
// 		vector <int> histogramXOY1D;
// 		convert2DhistogramTo1Dhisftogram(XOY_histogram, histogramXOY1D);
// 		//printHistogram ( histogramXOY1D, "XOY");		
// 		complete_object_histogram.insert(complete_object_histogram.end(), histogramXOY1D.begin(), histogramXOY1D.end());
// 		//printHistogram ( complete_object_histogram, "complete_object_histogram");
// 		
// 		vector <float> normalized_histogramXoY;
// 		normalizingHistogram( histogramXOY1D, normalized_histogramXoY);
// 		//printHistogram ( normalized_histogramXoY, "normalized XoY");
// 		normalized_projected_views.push_back(normalized_histogramXoY);
// 
// 		complete_object_histogram_normalized.insert(complete_object_histogram_normalized.end(), normalized_histogramXoY.begin(), normalized_histogramXoY.end());
// 		float XoY_entropy = 0;
// 		//viewpointEntropy(normalized_histogramXoY, XoY_entropy);
// 		viewpointEntropyNotNormalized(histogramXOY1D, XoY_entropy);
// 
// 		ROS_INFO("viewpointEntropyXoY = %f", XoY_entropy);
// 		view_point_entropy.push_back(XoY_entropy);
// 
// 		//printHistogram ( complete_object_histogram_normalized, "complete_object_histogram_normalized");
// 		
// 		vector <float> normalized_histogram;
//  		normalizingHistogram( complete_object_histogram, normalized_histogram);
// 		//printHistogram ( normalized_histogram, "normalized_complete_object_histogram");
// 
// 		int maximum_entropy_index = 0;
// 		findMaxViewPointsEntropy(view_point_entropy, maximum_entropy_index);
// 		ROS_INFO("Summary of entropy : \n H(YoZ) = %f, H(XoZ) = %f, H(XoY) = %f, Max_ind = %d", 
// 			  view_point_entropy.at(0), view_point_entropy.at(1), view_point_entropy.at(2) , maximum_entropy_index );		
// 		
// 		vector< float > sorted_normalized_projected_views;
// 		string std_name_of_sorted_projected_plane;
// 		objectViewHistogram( maximum_entropy_index,
// 				     view_point_entropy,
// 				    normalized_projected_views,
// 				    sorted_normalized_projected_views,
// 				    std_name_of_sorted_projected_plane
//  				  );
// 		
// 		ROS_INFO("Projected views are sorted as follows: %s", std_name_of_sorted_projected_plane.c_str());   		
// 		//printHistogram ( sorted_normalized_projected_views, "sorted_normalized_projected_views");
// 
// 
// 		
// 		SITOV object_representation;			
// 		for (size_t i = 0; i < sorted_normalized_projected_views.size(); i++)
// 		{
// 		    object_representation.spin_image.push_back(sorted_normalized_projected_views.at(i));
// 		}
// 		
// 		
// 			/* _____________________________________________
//                    |                                            |
//                    |  Write features to DB based on TID and VID |
//                    |____________________________________________| */
// 
//                 beginProc = ros::Time::now();
// 
//                 //Declare SITOV (Spin Images of Tracked Object View)
//                 SITOV _sitov;
// 
//                 //Declare RTOV (Representation of Tracked Object View)
//                 RTOV _rtov;
//                 _rtov.track_id = msg->track_id;
//                 _rtov.view_id = msg->view_id;
// 
// 		pp.info(std::ostringstream().flush() << "Track_id = " << msg->track_id << "\tView_id = " << msg->view_id );
// 		
// 		
//                 //declare the RTOV complete variable
//                 race_perception_msgs::CompleteRTOV _crtov;
//                 _crtov.track_id = msg->track_id;
//                 _crtov.view_id = msg->view_id;
//                 _crtov.ground_truth_name = msg->ground_truth_name.c_str();
// 
// // 		    ROS_INFO("ground_truth_name = %s", msg->ground_truth_name.c_str());
// 
//                 //Add the object view representation in msg_out to put in the DB
// 		_sitov = object_representation; //copy spin images
// 		_sitov.track_id = msg->track_id; //copy track_id
// 		_sitov.view_id = msg->view_id; //copy view_id
// 		_sitov.spin_img_id = 1; //copy spin image id
// 
// 		//Addd sitov to completertov sitov list
// 		_crtov.sitov.push_back(_sitov);
// 
// 		if (msg->is_key_view) //add sitovs to the DB only if this is a key view
// 		{
// 		    //Serialize to add to DB
// 		    uint32_t serial_size = ros::serialization::serializationLength(_sitov);
// 		    boost::shared_array<uint8_t> buffer(new uint8_t[serial_size]);
// 		    PerceptionDBSerializer<boost::shared_array<uint8_t>, SITOV>::serialize(buffer, _sitov, serial_size);	
// 		    leveldb::Slice s((char*)buffer.get(), serial_size);
// 		    std::string key = _pdb->makeSIKey(key::SI, msg->track_id, msg->view_id, 1 );
// 
// 		    //Put slice to the DB
// 		    _pdb->put(key, s); 
// 
// 		    //Add to the list of SITOV keys for this RTOV
// 		    _rtov.sitov_keys.push_back(key);
// 		    buffer.reset();
// 		}
// 
//                 //Add RTOV to the DB (only if this is a key view)
//                 if (msg->is_key_view)                 
//                 {
//                     uint32_t serial_size = ros::serialization::serializationLength(_rtov);
//                     boost::shared_array<uint8_t> buffer(new uint8_t[serial_size]);
//                     PerceptionDBSerializer<boost::shared_array<uint8_t>, RTOV>::serialize(buffer, _rtov, serial_size);	
//                     leveldb::Slice s((char*)buffer.get(), serial_size);
//                     std::string key = _pdb->makeKey(key::RV, msg->track_id, msg->view_id);
// 
//                     //Put slice to the db
//                     _pdb->put(key, s);
//                     buffer.reset();
//                 }
// 
//                 //Publish the CompleteRTOV to recognition
//                 _p_crtov_publisher->publish (_crtov);
// 
//                 //Toc
//                 ros::Duration duration = (ros::Time::now() - beginProc);
//                 double duration_sec = duration.toSec();
//                 pp.info(std::ostringstream().flush() << "Write the features to DB took " << duration_sec << " secs");
// 		
// 		
// 			     
// 		
// 
// 		
// 		
// 		
// 		
// 		
// 		
// 		//set_neat_visualization_marker_array_object_descriptor(initial_cloud_proj, msg->header.frame_id, msg -> track_id );
// 		*all_projected_views += *initial_cloud_proj_z;
// 
// 		//Declare PCTOV msg 
// 		boost::shared_ptr<race_perception_msgs::PCTOV> projected_point_cloud (new race_perception_msgs::PCTOV );
//  		//pcl::toROSMsg(*initial_cloud_proj, projected_point_cloud->point_cloud);	    
//  		//_p_projected_object_point_cloud_to_table_publisher->publish (projected_point_cloud->point_cloud);
// 		pcl::toROSMsg(*all_projected_views, projected_point_cloud->point_cloud);	    
// 		_p_projected_object_point_cloud_to_table_publisher->publish (projected_point_cloud->point_cloud);
// 
// 		//set_neat_visualization_marker_array_object_descriptor(all_projected_views, msg->header.frame_id, msg -> track_id );
// // 		set_neat_visualization_marker_array_object_descriptor_vector(target_pc,
// // 									       vector_of_projected_views,
// // 									       msg->header.frame_id,
// // 									       msg -> track_id, 
// // 									       largest_side, 
// // 									       sign,
// // 									       view_point_entropy,
// // 									       std_name_of_sorted_projected_plane
// // 									    );
// 		set_neat_visualization_marker_array_object_descriptor_vector_offline<>( target_pc,
// 											  pca_pc,
// 									  center_of_bbox,
// 											  vector_of_projected_views,
// 											  msg->header.frame_id,
// 											  msg -> track_id, 
// 											  largest_side, 
// 											  sign,
// 											  view_point_entropy,
// 											  std_name_of_sorted_projected_plane
// 											  );	
// 				//get toc
//                 duration = ros::Time::now() - beginProc;
//                 duration_sec = duration.toSec();
// 	        ROS_INFO(" compute projection of given object to three main axes palnes took  %f secs ", duration_sec);
// 		
// 		ros::Time start_time = ros::Time::now();
// 		while (ros::ok() && (ros::Time::now() - start_time).toSec() <30)
// 		{  //wait  
// 		}
// // 	

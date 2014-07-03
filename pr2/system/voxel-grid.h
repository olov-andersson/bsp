#ifndef __VOXEL_GRID_H__
#define __VOXEL_GRID_H__

#include "pr2-sim.h"
#include "../utils/rave-utils.h"
#include "../../util/logging.h"

#include <Eigen/Eigen>
using namespace Eigen;

#include <openrave-core.h>
namespace rave = OpenRAVE;

#include <boost/config.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/heap/fibonacci_heap.hpp>

#include <queue>
#include <unordered_set>
#include <assert.h>

typedef std::vector<Vector3i, aligned_allocator<Vector3i>> StdVector3i;

class Cube {
public:
	Cube() : x_size(0), y_size(0), z_size(0) { };

	Cube(int x, int y, int z) : x_size(x), y_size(y), z_size(z) {
		array = VectorXd(x_size*y_size*z_size);
		array.setZero();
	}

	inline double get(int x, int y, int z) const {
		assert((x >= 0) && (x < x_size) && (y >= 0) && (y < y_size) && (z >= 0) && (z < z_size));
		return array(z + y*(z_size) + x*(y_size*z_size));
	}

	inline double get(const Vector3i& xyz) const {
		return get(xyz(0), xyz(1), xyz(2));
	}

	inline void set(int x, int y, int z, double val) {
		assert((x >= 0) && (x < x_size) && (y >= 0) && (y < y_size) && (z >= 0) && (z < z_size));
		array(z + y*(z_size) + x*(y_size*z_size)) = val;
	}

	inline void set(const Vector3i& xyz, double val) {
		set(xyz(0), xyz(1), xyz(2), val);
	}

	inline void set_all(double val) {
		for(int i=0; i < x_size*y_size*z_size; ++i) {
			array(i) = val;
		}
	}

private:
	int x_size, y_size, z_size;
	VectorXd array;
};


class VoxelGrid {
public:
	VoxelGrid(const Vector3d& pos_center, double x_height, double y_height, double z_height, int resolution=512);

	void update_TSDF(const StdVector3d& pcl);
	Cube get_ODF(const Vector3d& obj);

	Vector3d signed_distance_complete_voxel_center(const Vector3d& object, const Cube& ODF,
			Camera* cam, const Matrix<double,H_SUB,W_SUB>& zbuffer, const Matrix4d& cam_pose);
	Vector3d signed_distance_greedy_voxel_center(const Vector3d& object, const Cube& ODF,
			Camera* cam, const Matrix<double,H_SUB,W_SUB>& zbuffer, const Matrix4d& cam_pose);

	double signed_distance_complete(const Vector3d& object, Cube& ODF,
			Camera* cam, const Matrix<double,H_SUB,W_SUB>& zbuffer, const Matrix4d& cam_pose);
	double signed_distance_greedy(const Vector3d& object, const Cube& ODF,
			Camera* cam, const Matrix<double,H_SUB,W_SUB>& zbuffer, const Matrix4d& cam_pose);

	StdVector3d get_obstacles();

	void plot_TSDF(rave::EnvironmentBasePtr env);
	void plot_ODF(Cube& ODF, rave::EnvironmentBasePtr env);
	void plot_FOV(rave::EnvironmentBasePtr env, Camera* cam, const Matrix<double,H_SUB,W_SUB>& zbuffer, const Matrix4d& cam_pose);

private:
	int resolution;
	Vector3d bottom_corner, top_corner;
	double dx, dy, dz, radius;

	Cube *TSDF;

	StdVector3i offsets;
	std::vector<double> offset_dists;

	void get_voxel_neighbors_and_dists(const Vector3i& voxel, StdVector3i& neighbors, std::vector<double>& dists);
	Vector3i voxel_from_point(const Vector3d& point);
	Vector3d point_from_voxel(const Vector3i& voxel);

	inline bool is_valid_point(const Vector3d& p) {
		return (((top_corner-p).minCoeff() > 0) && ((p - bottom_corner).minCoeff() > 0));
	}
	inline bool is_valid_voxel(const Vector3i& v) {
		return ((v(0) >= 0) && (v(0) < resolution) && (v(1) >= 0) && (v(1) < resolution) && (v(2) >= 0) && (v(2) < resolution));
	}
	inline int voxel_index(const Vector3i& voxel) {
		return (voxel(2) + voxel(1)*resolution + voxel(0)*resolution*resolution);
	}

};

#endif

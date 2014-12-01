// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <iostream>
#include <string>

// glut
#include <gl/glut.h>

// anttweakbar
#include <AntTweakBar.h>

// eigen
#include <Eigen/dense>

// ann
#include <ANN/ANN.h>

// libigl
#include <igl/write_triangle_mesh.h>
#include <igl/read_triangle_mesh.h>
#include <igl/file_dialog_open.h>
#include <igl/file_dialog_save.h>
#include <igl/per_face_normals.h>
#include <igl/per_vertex_normals.h>
#include <igl/avg_edge_length.h>


typedef Eigen::Vector3d Vec3d;
typedef Eigen::Vector2i Vec2i;

template <typename T>
T Min(T a, T b)
{
	return a > b ? b : a;
}

template <typename T>
T Max(T a, T b)
{
	return a > b ? a : b;
}
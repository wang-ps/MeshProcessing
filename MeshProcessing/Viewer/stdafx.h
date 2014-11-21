#pragma once
#include <iostream>
#include <string>
#include <gl/glut.h>
#include <AntTweakBar.h>
#include <Eigen/dense>
#include <igl/write_triangle_mesh.h>
#include <igl/read_triangle_mesh.h>
#include <igl/file_dialog_open.h>
#include <igl/file_dialog_save.h>
#include <igl/per_face_normals.h>
#include <igl/per_vertex_normals.h>

typedef Eigen::Vector3d Vec3d;
typedef Eigen::Vector2i Vec2i;
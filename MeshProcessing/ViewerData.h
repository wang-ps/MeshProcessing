#pragma once
#include <Eigen/Core>
typedef Eigen::Matrix<double, -1, -1, Eigen::RowMajor> MaxtrixXdc;
typedef Eigen::Matrix<int, -1, -1, Eigen::RowMajor> MaxtrixXic;

class ViewerData
{
public:
	ViewerData();

	enum DirtyFlags
	{
		DIRTY_NONE = 0x0000,
		DIRTY_POSITION = 0x0001,
		DIRTY_UV = 0x0002,
		DIRTY_NORMAL = 0x0004,
		DIRTY_AMBIENT = 0x0008,
		DIRTY_DIFFUSE = 0x0010,
		DIRTY_SPECULAR = 0x0020,
		DIRTY_TEXTURE = 0x0040,
		DIRTY_FACE = 0x0080,
		DIRTY_MESH = 0x00FF,
		DIRTY_OVERLAY_LINES = 0x0100,
		DIRTY_OVERLAY_POINTS = 0x0200,
		DIRTY_ALL = 0x03FF
	};

	// Empty all fields
	void clear();

	// Change the visualization mode, invalidating the cache if necessary
	void set_face_based(bool newvalue);

	// set new vertices and faces
	void set_mesh(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F);
	// set new vertices and keep the faces unchanged
	void set_vertices(const Eigen::MatrixXd& V);
	// set vertices or face normals
	void set_normals(const Eigen::MatrixXd& N);

	// Set the color of the mesh
	// Inputs: C  #V|#F|1 by 3 list of colors
	void set_colors(const Eigen::MatrixXd &C);

	// set the parameterization coordinate
	void set_uv(const Eigen::MatrixXd& UV);
	void set_uv(const Eigen::MatrixXd& UV_V, const Eigen::MatrixXi& UV_F);
	void set_texture(const Eigen::Matrix<char, Eigen::Dynamic, Eigen::Dynamic>& R,
		const Eigen::Matrix<char, Eigen::Dynamic, Eigen::Dynamic>& G,
		const Eigen::Matrix<char, Eigen::Dynamic, Eigen::Dynamic>& B);

	// Computes the normals of the mesh
	void compute_normals();

	// Assigns uniform colors to all faces/vertices
	void uniform_colors(Eigen::Vector3d ambient, Eigen::Vector3d diffuse, Eigen::Vector3d specular);

	// Generates a default grid texture
	void grid_texture();

public:
	Eigen::MatrixXd V; // Vertices of the current mesh (#V x 3)
	Eigen::MatrixXi  F; // Faces of the mesh (#F x 3)

	// Per face attributes
	Eigen::MatrixXd F_normals; // One normal per face

	Eigen::MatrixXd F_material_ambient; // Per face ambient color
	Eigen::MatrixXd F_material_diffuse; // Per face diffuse color
	Eigen::MatrixXd F_material_specular; // Per face specular color

	// Per vertex attributes
	Eigen::MatrixXd V_normals; // One normal per vertex

	Eigen::MatrixXd V_material_ambient; // Per vertex ambient color
	Eigen::MatrixXd V_material_diffuse; // Per vertex diffuse color
	Eigen::MatrixXd V_material_specular; // Per vertex specular color

	// UV parametrization
	Eigen::MatrixXd V_uv; // UV vertices
	Eigen::MatrixXi F_uv; // optional faces for UVs

	// Texture
	Eigen::Matrix<char, Eigen::Dynamic, Eigen::Dynamic> texture_R;
	Eigen::Matrix<char, Eigen::Dynamic, Eigen::Dynamic> texture_G;
	Eigen::Matrix<char, Eigen::Dynamic, Eigen::Dynamic> texture_B;

	// Marks dirty buffers that need to be uploaded to OpenGL
	unsigned dirty;

	// Enable per-face or per-vertex properties
	bool face_based;
};

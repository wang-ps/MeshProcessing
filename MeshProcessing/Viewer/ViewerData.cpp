#include "stdafx.h"
#include "ViewerData.h"

MeshData::MeshData()
{
  clear();
  obj = gluNewQuadric();

};

MeshData::~MeshData()
{
	delete ann_kdTree;
	gluDeleteQuadric(obj);
	annClose();
}

void MeshData::clear()
{
  V                       = Eigen::MatrixXd (0,3);
  F                       = Eigen::MatrixXi (0,3);

  F_material_ambient      = Eigen::MatrixXd (0,3);
  F_material_diffuse      = Eigen::MatrixXd (0,3);
  F_material_specular     = Eigen::MatrixXd (0,3);

  V_material_ambient      = Eigen::MatrixXd (0,3);
  V_material_diffuse      = Eigen::MatrixXd (0,3);
  V_material_specular     = Eigen::MatrixXd (0,3);

  F_normals               = Eigen::MatrixXd (0,3);
  V_normals               = Eigen::MatrixXd (0,3);

  V_uv                    = Eigen::MatrixXd (0,2);
  F_uv                    = Eigen::MatrixXi (0,3);

  face_based = false;

  selected_pts.clear();
}


void MeshData::set_face_based(bool newvalue)
{
  if (face_based != newvalue)
  {
    face_based = newvalue;
  }
}

// Helpers that draws the most common meshes
void MeshData::set_mesh(const Eigen::MatrixXd& _V, const Eigen::MatrixXi& _F)
{
  // empty the mesh
  clear(); 

  // If V only has two columns, pad with a column of zeros
  if (_V.cols() == 2)
  {
    V = Eigen::MatrixXd::Zero(_V.rows(),3);
    V.block(0,0,_V.rows(),2) = _V;
  }
  else
  {
    V = _V;
  }

  // set the faces
  F = _F;
  
  // calc bounding box
  p_min = V.colwise().minCoeff();
  p_max = V.colwise().maxCoeff();

  // average edge lenght
  avg_edge = igl::avg_edge_length(V, F);
  compute_normals();
  uniform_colors(Vec3d(0.2, 0.2, 0.2),
                 Vec3d(0.6, 0.5, 0),
                 Vec3d(0.3, 0.3, 0.3));

  grid_texture();

  init_kdTree();
}

void MeshData::set_vertices(const Eigen::MatrixXd& _V)
{
  V = _V;
  assert(F.size() == 0 || F.maxCoeff() < V.rows());
}

void MeshData::set_normals(const Eigen::MatrixXd& N)
{
  if (N.rows() == V.rows())
  {
    set_face_based(false);
    V_normals = N;
  }
  else if (N.rows() == F.rows() || N.rows() == F.rows()*3)
  {
    set_face_based(true);
    F_normals = N;
  }
  else
    std::cerr << "ERROR (set_normals): Please provide a normal per face, per corner or per vertex.";
}

void MeshData::set_colors(const Eigen::MatrixXd &C)
{
  using namespace Eigen;
  
  // Ambient color should be darker color
  const auto ambient = [](const MatrixXd & C)->MatrixXd
  {
    return 0.1*C;
  };

  // Specular color should be a less saturated and darker color: dampened  highlights
  const auto specular = [](const MatrixXd & C)->MatrixXd
  {
    const double grey = 0.3;
    return grey+0.1*(C.array()-grey);
  };
  
  // for constant color
  if (C.rows() == 1)
  {
    for (unsigned i=0;i<V_material_diffuse.rows();++i)
    {
      V_material_diffuse.row(i) = C.row(0);
    }
    V_material_ambient = ambient(V_material_diffuse);
    V_material_specular = specular(V_material_diffuse);

    for (unsigned i=0;i<F_material_diffuse.rows();++i)
    {
      F_material_diffuse.row(i) = C.row(0);
    }
    F_material_ambient = ambient(F_material_diffuse);
    F_material_specular = specular(F_material_diffuse);
  }
  // for vertices color matrix
  else if (C.rows() == V.rows())
  {
    set_face_based(false);
    V_material_diffuse = C;
    V_material_ambient = ambient(V_material_diffuse);
    V_material_specular = specular(V_material_diffuse);
  }
  // for faces color matrix
  else if (C.rows() == F.rows())
  {
    set_face_based(true);
    F_material_diffuse = C;
    F_material_ambient = ambient(F_material_diffuse);
    F_material_specular = specular(F_material_diffuse);
  }
  else
    std::cerr << "ERROR (set_colors): Please provide a single color, or a color per face or per vertex.";
}

void MeshData::set_uv(const Eigen::MatrixXd& UV)
{
  if (UV.rows() == V.rows())
  {
    set_face_based(false);
    V_uv = UV;
  }
  else
    std::cerr << "ERROR (set_UV): Please provide uv per vertex.";
}

void MeshData::set_uv(const Eigen::MatrixXd& UV_V, const Eigen::MatrixXi& UV_F)
{
  set_face_based(true);
  V_uv = UV_V;
  F_uv = UV_F;
}

void MeshData::set_texture(
  const Eigen::Matrix<char,Eigen::Dynamic,Eigen::Dynamic>& R,
  const Eigen::Matrix<char,Eigen::Dynamic,Eigen::Dynamic>& G,
  const Eigen::Matrix<char,Eigen::Dynamic,Eigen::Dynamic>& B)
{
  texture_R = R;
  texture_G = G;
  texture_B = B;
}

void MeshData::compute_normals()
{
  igl::per_face_normals(V, F, F_normals);
  igl::per_vertex_normals(V, F, F_normals, V_normals);
}

void MeshData::uniform_colors(Vec3d ambient, Vec3d diffuse, Vec3d specular)
{
  V_material_ambient.resize(V.rows(),3);
  V_material_diffuse.resize(V.rows(),3);
  V_material_specular.resize(V.rows(),3);
  for (unsigned i=0; i<V.rows();++i)
  {
    V_material_ambient.row(i) = ambient;
    V_material_diffuse.row(i) = diffuse;
    V_material_specular.row(i) = specular;
  }

  F_material_ambient.resize(F.rows(),3);
  F_material_diffuse.resize(F.rows(),3);
  F_material_specular.resize(F.rows(),3);
  for (unsigned i=0; i<F.rows();++i)
  {
    F_material_ambient.row(i) = ambient;
    F_material_diffuse.row(i) = diffuse;
    F_material_specular.row(i) = specular;
  }
}

void MeshData::grid_texture()
{
  if (V_uv.rows() == 0)
  {
    V_uv = V.block(0, 0, V.rows(), 2);
    V_uv.col(0) = V_uv.col(0).array() - V_uv.col(0).minCoeff();
    V_uv.col(0) = V_uv.col(0).array() / V_uv.col(0).maxCoeff();
    V_uv.col(1) = V_uv.col(1).array() - V_uv.col(1).minCoeff();
    V_uv.col(1) = V_uv.col(1).array() / V_uv.col(1).maxCoeff();
    V_uv = V_uv.array() * 10;
  }

  unsigned size = 128;
  unsigned size2 = size/2;
  texture_R.resize(size, size);
  for (unsigned i=0; i<size; ++i)
  {
    for (unsigned j=0; j<size; ++j)
    {
      texture_R(i,j) = 0;
      if ((i<size2 && j<size2) || (i>=size2 && j>=size2))
        texture_R(i,j) = 255;
    }
  }

  texture_G = texture_R;
  texture_B = texture_R;
}

void MeshData::init_kdTree()
{
	int n = V.rows();
	ann_pts = annAllocPts(n, 3);
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			ann_pts[i][j] = V(i, j);
		}
	}

	ann_kdTree = new ANNkd_tree(ann_pts, n, 3);
}

void MeshData::select_pt(Vec3d &pt)
{
	for (int i = 0; i < 3; i++)
	{
		if (pt[i]<p_min[i] - avg_edge||
			pt[i]> p_max[i] + avg_edge)
			return;
	}

	ANNpoint queryPt = annAllocPt(3);
	ANNidx* Idx = new ANNidx;
	ANNdist* dist = new ANNdist;
	queryPt[0] = pt[0]; queryPt[1] = pt[1]; queryPt[2] = pt[2];
	ann_kdTree->annkSearch(queryPt, 1, Idx, dist);

	if (*dist < 3 * avg_edge)
	{
		auto it = find(selected_pts.begin(), selected_pts.end(), *Idx);
		if (it == selected_pts.end() || selected_pts.empty())
			selected_pts.push_back(*Idx);
		else
			selected_pts.erase(it);
	}

	delete Idx;
	delete dist;
}

void MeshData::draw_mesh(int mode)
{
	// color material && face-based && flat
	if (mode == 0)
	{
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT, GL_DIFFUSE);
		glShadeModel(GL_FLAT);
		glBegin(GL_TRIANGLES);
		for (int i = 0; i < F.rows(); i++)
		{
			glNormal3d(F_normals(i, 0), F_normals(i, 1), F_normals(i, 2));
			for (int j = 0; j < 3; j++)
			{
				int iv = F(i, j);
				glColor3d(V_material_diffuse(iv, 0), V_material_diffuse(iv, 1), V_material_diffuse(iv, 2));
				glVertex3d(V(iv, 0), V(iv, 1), V(iv, 2));
			}
		}
		glEnd();
		glDisable(GL_COLOR_MATERIAL);
	}
	
	// color material && vertex-based && smooth
	if (mode == 1)
	{
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT, GL_DIFFUSE);
		glShadeModel(GL_SMOOTH);
		glBegin(GL_TRIANGLES);
		for (int i = 0; i < F.rows(); i++)
		{
			for (int j = 0; j < 3; j++)
			{
				int iv = F(i, j);
				glColor3d(V_material_diffuse(iv, 0), V_material_diffuse(iv, 1), V_material_diffuse(iv, 2));
				glNormal3d(V_normals(iv, 0), V_normals(iv, 1), V_normals(iv, 2));
				glVertex3d(V(iv, 0), V(iv, 1), V(iv, 2));
			}
		}
		glEnd();
		glDisable(GL_COLOR_MATERIAL);
	}
	
	// face-based && flat
	if (mode == 2)
	{
		glShadeModel(GL_FLAT);
		glBegin(GL_TRIANGLES);
		for (int i = 0; i < F.rows(); i++)
		{
			glNormal3d(F_normals(i, 0), F_normals(i, 1), F_normals(i, 2));
			for (int j = 0; j < 3; j++)
			{
				int iv =F(i, j);
				glVertex3d(V(iv, 0), V(iv, 1), V(iv, 2));
			}
		}
		glEnd();
	}
}

void MeshData::draw_select_pts()
{
	int n = selected_pts.size();
	if (n < 1)return;

	double radius = Min((p_max - p_min).norm()*0.01, avg_edge/3);

	gluQuadricDrawStyle(obj, GLU_FILL);
	gluQuadricNormals(obj, GLU_SMOOTH);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	glColor3d(0.8, 0.0, 0.0);
	for (int i = 0; i < n; i++)
	{
		Vec3d pt = V.row(selected_pts[i]);
		glPushMatrix();
		glTranslatef(pt[0], pt[1], pt[2]);
		gluSphere(obj, radius, 15, 15);
		glPopMatrix();
	}
	glDisable(GL_COLOR_MATERIAL);

// 	glDisable(GL_LIGHTING);
// 	glColor3d(1.0, 0.0, 0.0);
// 	glPointSize(0.02*avg_edge);
// 	glBegin(GL_POINTS);
// 	for (int i = 0; i < n; i++)
// 	{
// 		Vec3d pt = V.row(selected_pts[i]);
// 		glVertex3d(pt[0], pt[1], pt[2]);
// 	}
// 	glEnd();
// 	glEnable(GL_LIGHTING);
}
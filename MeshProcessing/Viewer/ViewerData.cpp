#include "stdafx.h"
#include "ViewerData.h"



ViewerData::ViewerData()
{
  clear();
};

void ViewerData::clear()
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
}


void ViewerData::set_face_based(bool newvalue)
{
  if (face_based != newvalue)
  {
    face_based = newvalue;
    dirty = DIRTY_ALL;
  }
}

// Helpers that draws the most common meshes
void ViewerData::set_mesh(const Eigen::MatrixXd& _V, const Eigen::MatrixXi& _F)
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
  
  compute_normals();
  uniform_colors(Eigen::Vector3d(51.0/255.0,43.0/255.0,33.3/255.0),
                 Eigen::Vector3d(255.0/255.0,228.0/255.0,58.0/255.0),
                 Eigen::Vector3d(255.0/255.0,235.0/255.0,80.0/255.0));

  grid_texture();

  dirty |= DIRTY_FACE | DIRTY_POSITION;
}

void ViewerData::set_vertices(const Eigen::MatrixXd& _V)
{
  V = _V;
  assert(F.size() == 0 || F.maxCoeff() < V.rows());
  dirty |= DIRTY_POSITION;
}

void ViewerData::set_normals(const Eigen::MatrixXd& N)
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
  dirty |= DIRTY_NORMAL;
}

void ViewerData::set_colors(const Eigen::MatrixXd &C)
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
  dirty |= DIRTY_DIFFUSE;
}

void ViewerData::set_uv(const Eigen::MatrixXd& UV)
{
  if (UV.rows() == V.rows())
  {
    set_face_based(false);
    V_uv = UV;
  }
  else
    std::cerr << "ERROR (set_UV): Please provide uv per vertex.";
  dirty |= DIRTY_UV;
}

void ViewerData::set_uv(const Eigen::MatrixXd& UV_V, const Eigen::MatrixXi& UV_F)
{
  set_face_based(true);
  V_uv = UV_V;
  F_uv = UV_F;
  dirty |= DIRTY_UV;
}

void ViewerData::set_texture(
  const Eigen::Matrix<char,Eigen::Dynamic,Eigen::Dynamic>& R,
  const Eigen::Matrix<char,Eigen::Dynamic,Eigen::Dynamic>& G,
  const Eigen::Matrix<char,Eigen::Dynamic,Eigen::Dynamic>& B)
{
  texture_R = R;
  texture_G = G;
  texture_B = B;
  dirty |= DIRTY_TEXTURE;
}

void ViewerData::compute_normals()
{
  igl::per_face_normals(V, F, F_normals);
  igl::per_vertex_normals(V, F, F_normals, V_normals);
  dirty |= DIRTY_NORMAL;
}

void ViewerData::uniform_colors(Eigen::Vector3d ambient, Eigen::Vector3d diffuse, Eigen::Vector3d specular)
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
  dirty |= DIRTY_SPECULAR | DIRTY_DIFFUSE | DIRTY_AMBIENT;
}

void ViewerData::grid_texture()
{
  if (V_uv.rows() == 0)
  {
    V_uv = V.block(0, 0, V.rows(), 2);
    V_uv.col(0) = V_uv.col(0).array() - V_uv.col(0).minCoeff();
    V_uv.col(0) = V_uv.col(0).array() / V_uv.col(0).maxCoeff();
    V_uv.col(1) = V_uv.col(1).array() - V_uv.col(1).minCoeff();
    V_uv.col(1) = V_uv.col(1).array() / V_uv.col(1).maxCoeff();
    V_uv = V_uv.array() * 10;
    dirty |= DIRTY_TEXTURE;
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
  dirty |= DIRTY_TEXTURE;
}

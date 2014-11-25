#include "stdafx.h"
#include "MeshViewer.hh"

// -----------
MeshViewer::MeshViewer(const char* _title, int _width, int _height)
  :GlutViewer(_title, _width, _height)
{
}

// -----------
void MeshViewer::open_mesh(const char* _filename)
{
	Eigen::MatrixXd V;
	Eigen::MatrixXi F;
	igl::read_triangle_mesh(_filename, V, F);
	
	Vec3d p1 = V.colwise().minCoeff();
	Vec3d p2 = V.colwise().maxCoeff();
	setup_scene((p1 + p2)*0.5, (p1 - p2).norm()/2.0);
 	
	mesh_.set_mesh(V, F);

	create_display_list();
}

void MeshViewer::draw()
{
	if (!mesh_.V.rows())
	{
		GlutViewer::draw();
		return;
	}
	if (draw_mode_ == HIDDEN_LINE)
	{
		glDisable(GL_LIGHTING);
		glColor3f(0.298, 0.298, 0.502);
		glDepthRange(0.01, 1.0);
		glCallList(draw_list_[1]);

		glColor3f(0.7, 0.7, 0.7);
		glDepthRange(0.0, 1.0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glCallList(draw_list_[0]);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (draw_mode_ == WIRE_FRAME)
	{
		glEnable(GL_LIGHTING);
		glPolygonOffset(1, 1);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glCallList(draw_list_[0]);
		glDisable(GL_POLYGON_OFFSET_FILL);		

		glDisable(GL_LIGHTING);
		glColor3f(0.7, 0.7, 0.7);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glCallList(draw_list_[0]);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

    if (draw_mode_ == SOLID_FLAT)
	{
		glEnable(GL_LIGHTING);
		glCallList(draw_list_[0]);		
	}

	if (draw_mode_ == SOLID_SMOOTH)
	{
		glEnable(GL_LIGHTING);
		glCallList(draw_list_[1]);
	}
}

void MeshViewer::create_display_list()
{
	draw_list_[0] = glGenLists(1);
	glNewList(draw_list_[0], GL_COMPILE);
		glShadeModel(GL_FLAT);
		glBegin(GL_TRIANGLES);
		for (int i = 0; i < mesh_.F.rows(); i++)
		{
			glNormal3d(mesh_.F_normals(i, 0), mesh_.F_normals(i, 1), mesh_.F_normals(i, 2));
			for (int j = 0; j < 3; j++)
			{
				int iv = mesh_.F(i, j);
				glVertex3d(mesh_.V(iv, 0), mesh_.V(iv, 1), mesh_.V(iv, 2));
			}
		}
		glEnd();
	glEndList();

	draw_list_[1] = glGenLists(1);
	glNewList(draw_list_[1], GL_COMPILE);
	glShadeModel(GL_SMOOTH);
		glBegin(GL_TRIANGLES);
		for (int i = 0; i < mesh_.F.rows(); i++)
		{
			for (int j = 0; j < 3; j++)
			{
				int iv = mesh_.F(i, j);
				glNormal3d(mesh_.V_normals(iv, 0), mesh_.V_normals(iv, 1), mesh_.V_normals(iv, 2));
				glVertex3d(mesh_.V(iv, 0), mesh_.V(iv, 1), mesh_.V(iv, 2));
			}
		}
		glEnd();
	glEndList();

}

void MeshViewer::setup_anttweakbar()
{
	GlutViewer::setup_anttweakbar();
	TwAddButton(bar_, "Open File", tw_open_file, this, "group = 'File'");
	TwAddButton(bar_, "Save File", tw_save_file, this, "group = 'File'");
}

void MeshViewer::tw_open_file(void *_clientData)
{
	std::string filename = igl::file_dialog_open();
	if (!filename.empty())
	{
		MeshViewer* viewer = (MeshViewer*)_clientData;
		viewer->open_mesh(filename.c_str());
	}
}

void MeshViewer::tw_save_file(void *_clientData)
{
	std::string filename = igl::file_dialog_save();
	if (!filename.empty())
	{
		MeshViewer* viewer = (MeshViewer*)_clientData;
		igl::write_triangle_mesh(filename, viewer->mesh_.V, viewer->mesh_.F);
	}
}
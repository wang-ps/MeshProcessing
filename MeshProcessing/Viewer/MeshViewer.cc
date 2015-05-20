#include "stdafx.h"
#include "MeshViewer.hh"

// -----------
MeshViewer::MeshViewer(const char* _title, int _width, int _height)
:GlutViewer(_title, _width, _height), select_flag(false)
{
}

// -----------
void MeshViewer::open_mesh(const char* _filename)
{
	Eigen::MatrixXd V;
	Eigen::MatrixXi F;
	igl::read_triangle_mesh(_filename, V, F);
	
	set_mesh(V, F);
}

void MeshViewer::set_mesh(Eigen::MatrixXd &_V, Eigen::MatrixXi &_F)
{
	mesh_.set_mesh(_V, _F);

	Vec3d p1 = _V.colwise().minCoeff();
	Vec3d p2 = _V.colwise().maxCoeff();
	setup_scene((p1 + p2)*0.5, (p1 - p2).norm() / 2.0);

	create_display_list();
}

void MeshViewer::set_color(Eigen::MatrixXd &C)
{
	mesh_.set_colors(C);
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
		glCallList(draw_list_+2);

		glColor3f(0.7, 0.7, 0.7);
		glDepthRange(0.0, 1.0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glCallList(draw_list_+2);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (draw_mode_ == WIRE_FRAME)
	{
		glEnable(GL_LIGHTING);
		glPolygonOffset(1, 1);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glCallList(draw_list_);
		glDisable(GL_POLYGON_OFFSET_FILL);		

		glDisable(GL_LIGHTING);
		glColor3f(0.2, 0.2, 0.2);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glCallList(draw_list_ + 2);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

    if (draw_mode_ == SOLID_FLAT)
	{
		glEnable(GL_LIGHTING);
		glDepthRange(0.01, 1.0);
		glCallList(draw_list_);		
	}

	if (draw_mode_ == SOLID_SMOOTH)
	{
		glEnable(GL_LIGHTING);
		glDepthRange(0.01, 1.0);
		glCallList(draw_list_+1);
	}

	glEnable(GL_LIGHTING);
	mesh_.draw_select_pts();
	//glPolygonOffset(1, 1);
	glDepthRange(0.0, 1.0);
	mesh_.draw_select_faces();
}

void MeshViewer::mouse(int button, int state, int x, int y)
{

	// select point
	int modifier = glutGetModifiers();
	if (( modifier == GLUT_ACTIVE_CTRL ||modifier == GLUT_ACTIVE_ALT) && state == GLUT_DOWN)
	{
		GLdouble winX = double(x);
		GLdouble winY = double(viewport_[3] - y);
		GLfloat winZ = 0.0;
		GLdouble pt[3];
		glReadPixels((int)winX, (int)winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
		gluUnProject(winX, winY, (GLdouble)winZ, modelview_matrix_, projection_matrix_, viewport_, &pt[0], &pt[1], &pt[2]);

		//std::cout << winX << " " << winY << " " << winZ << std::endl;
		//std::cout << pt[0] << " " << pt[1] << " " << pt[2] << std::endl;

		if (modifier == GLUT_ACTIVE_CTRL)
			mesh_.select_pt(Vec3d(pt));
		else
			mesh_.select_face(Vec3d(pt));
	}
	else{
		GlutViewer::mouse(button, state, x, y);
	}

}

void MeshViewer::create_display_list()
{
	glDeleteLists(draw_list_, 3);
	draw_list_ = glGenLists(3);

	glNewList(draw_list_, GL_COMPILE);
	mesh_.draw_mesh(0);
	glEndList();

	glNewList(draw_list_+1, GL_COMPILE);
	mesh_.draw_mesh(1);
	glEndList();

	glNewList(draw_list_+2, GL_COMPILE);
	mesh_.draw_mesh(2);
	glEndList();
}

void MeshViewer::setup_anttweakbar()
{
	GlutViewer::setup_anttweakbar();
	TwAddButton(bar_, "Open File", tw_open_file, this, "group = 'File'");
	TwAddButton(bar_, "Save File", tw_save_file, this, "group = 'File'");
	
	TwAddButton(bar_, "Clear Selection", tw_clear_select, this, "group = 'Select' ");
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

void MeshViewer::tw_clear_select(void *_clientData)
{
	MeshViewer* viewer = (MeshViewer*)_clientData;
	viewer->mesh_.selected_pts.clear();
	viewer->mesh_.selected_faces.clear();
}
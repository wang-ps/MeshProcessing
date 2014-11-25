#include "stdafx.h"
#include "GlutViewer.hh"

// -----------
// static data component
GlutViewer* GlutViewer::current_viewer_ = NULL;

// -----------
// constructor and destructor
GlutViewer::GlutViewer(const char* _title, int _width, int _height)
: title_(_title), width_(_width), height_(_height)
{
	// screen center and radius
	center_ = Vec3d(0.0, 0.0, 0.0);
	radius_ = 1;

	// projection parameter
	near_ = 0.01 * radius_;
	far_  = 10.0 * radius_;
	fovy_ = 45.0; 

	for (int i = 0; i < 16; i++)
	{
		double v = (i % 5) ? 0.0 : 1.0;
		rotation_matrix_[i] = v;
		projection_matrix_[i] = v;
		modelview_matrix_[i] = v;
	}

	//rotation_matrix_ = Eigen::AngleAxisf(0, Vec3f(0, 0, 1));
	trans_ = Vec3d(0, 0, 0);
	scale_ = 1.0;

	// menu related variables
	draw_mode_ = SOLID_FLAT;

	// init mouse buttons
	for (int i = 0; i < 5; ++i)
		button_down_[i] = false;

	// not full screen
	fullscreen_ = false;

	current_viewer_ = this;
}
  
GlutViewer::~GlutViewer()
{
  glutDestroyWindow(glutGetWindow());
}

// -----------
// public interface
void GlutViewer::launch(void)
{
	setup_glut();
	setup_view();
	setup_anttweakbar();
	setup_scene(Vec3d(0.0, 0.0, 0.0), 1);

	glutMainLoop();
}

// -----------
// setup 
void GlutViewer::setup_glut(void)
{
	// create window
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE | GLUT_ALPHA);
	glutInitWindowSize(width_, height_);
	glutCreateWindow(title_.c_str());

	// register callbacks
	glutDisplayFunc(display__);
	glutKeyboardFunc(keyboard__);
	glutSpecialFunc(special__);
	glutMouseFunc(mouse__);
	glutMotionFunc(motion__);
	glutPassiveMotionFunc(passivemotion__);
	glutReshapeFunc(reshape__);
	glutVisibilityFunc(visibility__);
}

void GlutViewer::setup_anttweakbar(void)
{
	// Initialize AntTweakBar
 	TwInit(TW_OPENGL, NULL);

	//  tell the size of the graphic window to AntTweakBar
	TwWindowSize(width_, height_);
	
	// get glut modifiers
	TwGLUTModifiersFunc(glutGetModifiers);

	// Create a tweak bar
	bar_ = TwNewBar("TweakBar");
	TwDefine(" GLOBAL help='This is a meshviewer.' "); // Message added to the help bar.
	TwDefine(" TweakBar size='200 400' color='76 76 127' refresh=0.5"); // change default tweak bar size and color

	// draw mode
	TwEnumVal DrawmodeEV[4] = { { HIDDEN_LINE, "Hidden Line" }, { WIRE_FRAME, "Wire Frame" },
	{ SOLID_FLAT, "Solid Flat" }, { SOLID_SMOOTH, "Solid Smooth" } };
	TwType DrwamodeType = TwDefineEnum("DrawMode", DrawmodeEV, 4);
	TwAddVarRW(bar_, "Draw Mode", DrwamodeType, &draw_mode_, "group = 'Draw'");

	
	// 	Called after glutMainLoop ends
	atexit(terminate__);
}

void GlutViewer::setup_view(void)
{
	// OpenGL state
	glClearColor(0.298, 0.298, 0.502, 0.0);
	glDisable(GL_DITHER);
	glEnable(GL_DEPTH_TEST);

	// some performance settings
	//   glEnable( GL_CULL_FACE );
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);


	// material
	GLfloat mat_a[] = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat mat_d[] = { 0.7, 0.6, 0.0, 1.0 };
	GLfloat mat_s[] = { 0.3, 0.3, 0.3, 1.0 };
	GLfloat shine[] = { 35.0 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_a);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_d);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_s);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shine);

	// lighting
	GLfloat pos[] = { 0.0, 0.0, 0.1, 0.0 };
	GLfloat col[] = { 1.0, 1.0, 1.0, 1.0 };
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, col);
	glLightfv(GL_LIGHT0, GL_SPECULAR, col);
}

void GlutViewer::setup_scene( const Vec3d& _cog, double _radius )
{
	// keep the rotation but reset the translation
	trans_ = trans_ * _radius/radius_;

	center_ = _cog;
	radius_ = _radius;

	near_  = 0.01 * radius_;
	far_   = 10.0 * radius_;

	apply_projection_matrix();
	apply_modelview_matrix();
	
}

void GlutViewer::apply_projection_matrix()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy_, (GLfloat)width_ / (GLfloat)height_, near_, far_);
	glGetDoublev(GL_PROJECTION_MATRIX, projection_matrix_);
}

void GlutViewer::apply_modelview_matrix()
{
	// scene pos and size
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslated(0, 0, -3 * radius_);					// view all
 	glTranslated(trans_[0], trans_[1], trans_[2]);		// translate
 	glMultMatrixd(rotation_matrix_);					// rotation
	//glScaled(scale_, scale_, scale_);					// scale
	glTranslated(-center_[0], -center_[1], -center_[2]);//	move the object to (0,0,0)
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix_);
}

// -----------
// reshape, need to update projection matrix
void GlutViewer::reshape(int _w, int _h)
{
	width_  = _w; 
	height_ = _h;
	glViewport(0, 0, _w, _h);
	apply_projection_matrix();

	// for anttweak bar
	TwWindowSize(_w, _h);
}

// -----------
// draw the sceen
void GlutViewer::draw()
{
	if (draw_mode_ == SOLID_FLAT)
	{
		glEnable(GL_LIGHTING);
		glShadeModel(GL_FLAT);
		glutSolidTeapot(0.5);
	}
	else if (draw_mode_ == WIRE_FRAME)
	{
		glEnable(GL_LIGHTING);
		glShadeModel(GL_SMOOTH);
		glPolygonOffset(1, 1);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glutSolidTeapot(0.5);
		glDisable(GL_POLYGON_OFFSET_FILL);

		glDisable(GL_LIGHTING);
		glColor3f(0.8, 0.8, 0.8);
		glDepthRange(0.0, 1.0);
		glutWireTeapot(0.5);
	}
	else if (draw_mode_ == SOLID_SMOOTH)
	{
		glEnable(GL_LIGHTING);
		glShadeModel(GL_SMOOTH);
		glutSolidTeapot(0.5);
	}
	else if (draw_mode_ == HIDDEN_LINE)
	{
		glDisable(GL_LIGHTING);
		glShadeModel(GL_SMOOTH);
		glColor3f(0.298, 0.298, 0.502);

		glDepthRange(0.01, 1.0);
		glutSolidTeapot(0.5);

		glColor3f(0.8, 0.8, 0.8);
		glDepthRange(0.0, 1.0);
		glutWireTeapot(0.5);
	}
	else
	{
		std::cout << "This view mode is not supported for this "
			<< "geometry, you need to load a mesh!" << std::endl;
	}
}

// -----------
// mouse and motion, to zoom, translate and rotate the scene
void GlutViewer::mouse(int button, int state, int x, int y)
{
	// mouse press
	if (state == GLUT_DOWN)
	{
		last_point_2D_ = Vec2i(x,y);
		last_point_3D_ = map_to_sphere(last_point_2D_);

		button_down_[button] = true;
	}
	// mouse release
	else
	{
		button_down_[button] = false;
	}
}

void GlutViewer::motion(int x, int y)
{
	if (x < 0 || x > width_ || y < 0 || y > height_) return;

	if (button_down_[2])
	{
		translation(x, y);
	}
	else if (button_down_[0])  // left button
	{
		rotation(x, y);
	}
	else if (button_down_[1])  // middle button
	{
		zoom(x, y);
	}

	// remeber points
	last_point_2D_ = Vec2i(x, y);
	last_point_3D_ = map_to_sphere(last_point_2D_);
}

// virtual trackball: map 2D screen point to unit sphere
Vec3d GlutViewer::map_to_sphere( const Vec2i& _v2D)
{
	double x = (double)(_v2D[0] - 0.5*width_) / (double)width_;
	double y = (double)(0.5*height_ - _v2D[1]) / (double)height_;
	double sinx = sin(M_PI * x * 0.5);
	double siny = sin(M_PI * y * 0.5);
	double sinx2siny2 = sinx * sinx + siny * siny;

	Vec3d v3D;
	v3D[0] = sinx;
	v3D[1] = siny;
	v3D[2] = sinx2siny2 < 1.0 ? sqrt(1.0 - sinx2siny2) : 0.0;

	return v3D;
}

void GlutViewer::rotation(int x, int y)
{
	Vec2i  new_point_2D = Vec2i(x, y);
	Vec3d  new_point_3D = map_to_sphere(new_point_2D);

	Vec3d axis = last_point_3D_ .cross(new_point_3D); // cross product
	double cos_angle = last_point_3D_.dot( new_point_3D); // dot product

	if (fabs(cos_angle) < 1.0)
	{
		double angle = 3.0*acos(cos_angle) * 180.0 / M_PI;
// 		axis.normalize();
// 		rotation_matrix_ = Eigen::AngleAxisf(angle, axis) *rotation_matrix_;

		glLoadIdentity();
		glRotatef(angle, axis[0], axis[1], axis[2]);
		glMultMatrixd(rotation_matrix_);
		glGetDoublev(GL_MODELVIEW_MATRIX, rotation_matrix_);
	}
}

void GlutViewer::translation(int x, int y)
{
	double z = -trans_[2] + 3 * radius_;
	double up = 2 * tan(fovy_ / 2.0f*M_PI / 180.f) * z;
	double ratio = up / height_;
	double dx = (x - last_point_2D_[0])*ratio;
	double dy = (last_point_2D_[1] - y)*ratio;

	trans_[0] += dx;
	trans_[1] += dy;
}

void GlutViewer::zoom(int x, int y)
{
	double dy = y - last_point_2D_[1];
	double h  = height_;
	dy *= 2.0; // to magnify the zooming effect 
	trans_[2] += (3.0 * radius_ )* dy / h;

	// NEED TO NORMALIZE THE NORMAL
// 	float dy = y - last_point_2D_[1];
// 	float z = 3 * radius_;
// 	scale_ += 2 * dy / height_;
// 	glEnable(GL_NORMALIZE);
}

// -----------
// for keyboard reaction
void GlutViewer::keyboard(int key, int x, int y) 
{
	switch (key)
	{
	case 27:
		exit(0); 
		break;
	case GLUT_KEY_F12:
		if (!fullscreen_) 
		{
			bak_left_   = glutGet(GLUT_WINDOW_X);
			bak_top_    = glutGet(GLUT_WINDOW_Y);
			bak_width_  = glutGet(GLUT_WINDOW_WIDTH);
			bak_height_ = glutGet(GLUT_WINDOW_HEIGHT);
			glutFullScreen();
			fullscreen_ = true;
		}
		else
		{
			glutReshapeWindow(bak_width_, bak_height_);
			glutPositionWindow(bak_left_, bak_top_);
			fullscreen_ = false;
		}
		break;
	}
} 

// -----------
void GlutViewer::passivemotion(int x, int y) {}
void GlutViewer::visibility(int visible) {}
void GlutViewer::idle(void) {} 

// -----------
// static function, just interface
void GlutViewer::display__(void) 
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	current_viewer_->apply_modelview_matrix();
	current_viewer_->draw();
	TwDraw();	// Draw tweak bars
	glutSwapBuffers();
}

void GlutViewer::idle__(void) {
	current_viewer_->idle();
}

void GlutViewer::keyboard__(unsigned char key, int x, int y) {
	// for anttweakbar
	if (!TwEventKeyboardGLUT(key, x, y))
	{
		// for glut
		current_viewer_->keyboard((int)key, x, y);
	}
}

void GlutViewer::motion__(int x, int y) {
	// for anttweakbar
	if (!TwEventMouseMotionGLUT(x, y))
	{
		// for glut
		current_viewer_->motion(x, y);
	}

	glutPostRedisplay();
}

void GlutViewer::mouse__(int button, int state, int x, int y) {
	// for anttweakbar
	if (!TwEventMouseButtonGLUT(button, state, x, y))
	{
		// for glut
		current_viewer_->mouse(button, state, x, y);
	}

	glutPostRedisplay();
}

void GlutViewer::passivemotion__(int x, int y) 
{
	int width = current_viewer_->width_;
	int height = current_viewer_->height_;
	
	// for anttweakbar
	if (TwEventMouseMotionGLUT(x, y))
	{
		glutPostRedisplay();
	}
	else{
		current_viewer_->passivemotion(x, y);
	}
}

void GlutViewer::reshape__(int w, int h) {
	current_viewer_->reshape(w, h);

	glutPostRedisplay();
}

void GlutViewer::special__(int key, int x, int y) {
	current_viewer_->keyboard(key, x, y);
}

void GlutViewer::visibility__(int visible) {
	current_viewer_->visibility(visible);
}

void GlutViewer::terminate__()
{
	TwTerminate();

}
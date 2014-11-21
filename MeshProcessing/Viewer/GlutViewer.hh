
#pragma once
#include "stdafx.h"

typedef enum { HIDDEN_LINE = 1, WIRE_FRAME, SOLID_FLAT, SOLID_SMOOTH} DrawMode;

class GlutViewer
{
public:   
	GlutViewer(const char* _title, int _width, int _height);
	virtual ~GlutViewer();
	void launch(void);

protected:
	virtual void setup_glut(void);
	virtual void setup_view(void);
	virtual void setup_anttweakbar(void);
	virtual void setup_scene(const Vec3d& _cog, double _radius);

	virtual void reshape(int w, int h); 
	virtual void draw();	
	virtual void mouse(int button, int state, int x, int y);
	virtual void motion(int x, int y);
	virtual void keyboard(int key, int x, int y);
	virtual void passivemotion(int x, int y);
	virtual void visibility(int visible);
	virtual void idle(void); 

private:
	void rotation(int x, int y);
	void translation(int x, int y);
	void zoom(int x, int y);
	
	void apply_projection_matrix();
	void apply_modelview_matrix();
	Vec3d map_to_sphere(const Vec2i& _point);

private:
	static void display__(void);
	static void idle__(void); 
	static void keyboard__(unsigned char key, int x, int y);
	static void motion__(int x, int y);
	static void mouse__(int button, int state, int x, int y);
	static void passivemotion__(int x, int y);
	static void reshape__(int w, int h); 
	static void special__(int key, int x, int y);   
	static void visibility__(int visible);
	static void terminate__();

protected:
	// screen width and height and title
	int  width_, height_;
	std::string title_;

	// scene position and dimension
	Vec3d center_;
	double radius_;
	Vec3d trans_;
	//Eigen::Affine3f rotation_matrix_;
	double rotation_matrix_[16];
	double scale_;

	// projection parameters
	double near_, far_, fovy_;

	// OpenGL matrices
	double projection_matrix_[16];
	double modelview_matrix_[16];

	// trackball helpers
	Vec2i last_point_2D_;
	Vec3d last_point_3D_;
	bool button_down_[5];

	DrawMode draw_mode_;
	
	// Pointer to the tweak bar
	TwBar *bar_; 

private:
	static GlutViewer* current_viewer_; 

	bool fullscreen_;
	int  bak_left_, bak_top_, bak_width_, bak_height_;
};

#ifndef MESH_VIEWER_WIDGET_HH
#define MESH_VIEWER_WIDGET_HH

#include "GlutViewer.hh"
#include "ViewerData.h"

class MeshViewer : public GlutViewer
{
public:
	/// default constructor
	MeshViewer(const char* _title, int _width, int _height);

	/// open mesh
	void open_mesh(const char* _filename);

	/// set mesh
	void set_mesh(Eigen::MatrixXd &_V, Eigen::MatrixXi &_F);

	/// set color
	void set_color(Eigen::MatrixXd &C);

protected:
	/// setup anttweakbar
	virtual void setup_anttweakbar(void);

	/// mouse
	virtual void mouse(int button, int state, int x, int y);

	/// draw the scene
	virtual void draw();

private:
	void create_display_list();
	static void TW_CALL tw_open_file(void *_clientData);
	static void TW_CALL tw_save_file(void *_clientData);
	static void TW_CALL tw_clear_select(void *_clientData);

private:
	GLuint draw_list_;

protected:
	MeshData  mesh_;
	bool select_flag;
};

#endif 
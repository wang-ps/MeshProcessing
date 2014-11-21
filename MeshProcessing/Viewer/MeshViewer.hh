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

	/// setup anttweakbar
	void setup_anttweakbar(void);

	/// draw the scene
	virtual void draw();
private:
	void create_display_list();
	static void TW_CALL tw_open_file(void *_clientData);
	static void TW_CALL tw_save_file(void *_clientData);

private:
	GLuint draw_list_[2];

protected:
	ViewerData  mesh_;
};

#endif 
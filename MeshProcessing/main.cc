#include "stdafx.h"
#include "MeshViewer.hh"

int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  MeshViewer viewer("Mesh Viewer", 1000, 600);
  viewer.launch();
  return 0;
}
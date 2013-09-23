#include <osg/Geometry>
#include <osg/Geode>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>
#include <osgGA/GUIEventHandler>
#include <osgViewer/ViewerEventHandlers>
#include <osg/LineWidth>
#include <osgViewer/Viewer>
#include <osgDB/ReadFile>

#include <iostream>
#include <stdio.h>
#include <vector>

#include "two_dim_manipulator.h"

using namespace std;

osg::Vec2 main_window_size( 500, 500 );

class PickHandler : public osgGA::GUIEventHandler
{
public:
	PickHandler() : _mX( 0. ),_mY( 0. ) {}
	bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa ) {
		osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>( &aa );
		if (!viewer)
			return false;
		switch( ea.getEventType() ) {
			case osgGA::GUIEventAdapter::PUSH:
			case osgGA::GUIEventAdapter::MOVE: {
				_mX = ea.getX();
				_mY = ea.getY();
				return false;
			}
			case osgGA::GUIEventAdapter::RELEASE: {
				if (_mX == ea.getX() && _mY == ea.getY()) {
					if (pick( ea.getX(), ea.getY(), viewer ))
						return true;
				}
				return false;
			}
			case osgGA::GUIEventAdapter::KEYDOWN: {
				switch( ea.getKey() ) {
				case 'o':
				//case 'ù':
				  break;
				}
				return false;
			}
			default:
			return false;
		}
	}
protected:
  float _mX,_mY;
	int last_time;
	bool pick( const double x, const double y, osgViewer::Viewer* viewer ) {

		return false;
	}
};

osg::Camera* createHUDCamera( double left, double right, double bottom, double top )
{
  osg::ref_ptr<osg::Camera> camera = new osg::Camera;
  camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
 	camera->setClearMask(NULL);
  camera->setRenderOrder( osg::Camera::POST_RENDER );
  camera->setAllowEventFocus( false );
  camera->setProjectionMatrix( osg::Matrix::ortho2D(left, right, bottom, top) );
  return camera.release();
}

osg::Geometry* Fractal(vector<osg::Vec2*> original) {
	osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;

	colors->push_back( osg::Vec4( 0.0f, 0.0f, 0.0f, 1.0f ) );
	double depth = -0.2f;

  osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
  vector<osg::Vec2*>::iterator cii;
  for (cii=original.begin(); cii != original.end(); ++cii) {
  	vertices->push_back( osg::Vec3((*cii)->x(), (*cii)->y(), depth));
  }
	osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
	normals->push_back( osg::Vec3(0.0f,-1.0f, 0.0f) );
	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
	geom->setVertexArray( vertices.get() );
	geom->setNormalArray( normals.get() );
	geom->setNormalBinding( osg::Geometry::BIND_OVERALL );
	geom->setColorArray( colors.get() );
	geom->setColorBinding( osg::Geometry::BIND_OVERALL );

	osg::StateSet* stateset_width = new osg::StateSet;
	osg::LineWidth* linewidth = new osg::LineWidth();
	linewidth->setWidth(1.0f);
	stateset_width->setAttributeAndModes(linewidth, osg::StateAttribute::ON);
	geom->setStateSet( stateset_width );

  geom->addPrimitiveSet(
      new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0,
                          original.size()) );

  return geom.release();
}

int main (int argc, char **argv) {

  osg::Camera* camera = createHUDCamera( 0, main_window_size.x(), 0, main_window_size.y() );

  osg::Group* root = new osg::Group;
  root->addChild(camera);

  vector<osg::Vec2*> original;
  original.push_back(new osg::Vec2(0, 0));
  original.push_back(new osg::Vec2(0.33f, 0));
  original.push_back(new osg::Vec2(0.33f, 0.33f));
  original.push_back(new osg::Vec2(0.66f, 0.33f));
  original.push_back(new osg::Vec2(0.66f, 0));
  original.push_back(new osg::Vec2(1, 0));

  osg::Geode* geode = new osg::Geode;
  geode->addDrawable(Fractal(original));
  root->addChild(geode);

  osgViewer::Viewer viewer;

  viewer.setSceneData(root);
	viewer.addEventHandler( new PickHandler );
	viewer.setCameraManipulator( new TwoDimManipulator );
	viewer.setUpViewInWindow( 50, 50, 50 + main_window_size.x(), 50 + main_window_size.y(), 0 );
	viewer.run();

  return 0;
}

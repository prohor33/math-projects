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
#include <math.h>

#include "two_dim_manipulator.h"

using namespace std;

osg::Vec2 main_window_size( 500, 500 );

class PickHandler : public osgGA::GUIEventHandler
{
public:
	PickHandler() : _mX( 0. ),_mY( 0. ) {}
	bool handle( const osgGA::GUIEventAdapter& ea,
              osgGA::GUIActionAdapter& aa ) {
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
	bool pick( const double x, const double y,
            osgViewer::Viewer* viewer ) {

		return false;
	}
};

osg::Camera* createHUDCamera( double left, double right,
                             double bottom, double top ) {
  osg::ref_ptr<osg::Camera> camera = new osg::Camera;
  camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
 	camera->setClearMask(NULL);
  camera->setRenderOrder( osg::Camera::POST_RENDER );
  camera->setAllowEventFocus( false );
  camera->setProjectionMatrix( osg::Matrix::ortho2D(left, right,
                                                    bottom, top) );
  return camera.release();
}

class Line {
public:
  Line() {};
  Line(osg::Vec2 v0, osg::Vec2 v1) :
      v0(v0), v1(v1) {};
  osg::Vec2 v0;
  osg::Vec2 v1;
};

vector<Line> wholeFractal;

osg::Geometry* drawFractal(vector<Line> fractal) {
	osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
	colors->push_back( osg::Vec4( 0.0f, 0.0f, 0.0f, 1.0f ) );
	double depth = -0.2f;

  osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
  vector<Line>::iterator cii;
  for (cii=fractal.begin(); cii!=fractal.end(); ++cii) {
  	vertices->push_back( osg::Vec3((*cii).v0.x(), (*cii).v0.y(), depth));
	  vertices->push_back( osg::Vec3((*cii).v1.x(), (*cii).v1.y(), depth));
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
	stateset_width->setAttributeAndModes(linewidth,
                                       osg::StateAttribute::ON);
	geom->setStateSet( stateset_width );

  geom->addPrimitiveSet(
      new osg::DrawArrays(osg::PrimitiveSet::LINES, 0,
                          fractal.size()*2) );

  return geom.release();
}

vector<Line> makeFractal(vector<Line> fractal,
                         vector<Line> original,
                         bool leaveOriginal=false) {
  vector<Line> newFractal;
  vector<Line>::iterator cii;
  for (cii=fractal.begin(); cii!=fractal.end(); ++cii) {
    osg::Vec2 v0, v1;
    v0 = (*cii).v0;
    v1 = (*cii).v1;
    osg::Vec2 delta = v1 - v0;
    float coef = delta.length();
    vector<Line>::iterator cii2;
    for (cii2=original.begin(); cii2!=original.end(); ++cii2) {
      // scale the original fractal
      osg::Vec2 v0_orig = (*cii2).v0;
      osg::Vec2 v1_orig = (*cii2).v1;
      v0_orig *= coef;
      v1_orig *= coef;
      // than we shoud rotate it
      float alpha;
      if ( delta.x() == 0 ) {
        if (delta.y() > 0)
          alpha = osg::PI_2;
        else
          alpha = -osg::PI_2;
      } else {
        alpha = atan(delta.y() / delta.x());
        if (delta.x() < 0)
          alpha += osg::PI;
      }
      osg::Vec3 v0_orig3d(v0_orig.x(), v0_orig.y(), 0);
      osg::Vec3 v1_orig3d(v1_orig.x(), v1_orig.y(), 0);
      osg::Quat q = osg::Quat( 0, osg::Vec3d(1,0,0),
                              0, osg::Vec3d(0,1,0),
                              alpha, osg::Vec3d(0,0,1));
      v0_orig3d = q * v0_orig3d;
      v1_orig3d = q * v1_orig3d;
      v0_orig = osg::Vec2(v0_orig3d.x(), v0_orig3d.y());
      v1_orig = osg::Vec2(v1_orig3d.x(), v1_orig3d.y());
      // and after that translate
      v0_orig += v0;
      v1_orig += v0;
      // now we get a new line
      newFractal.push_back(Line(v0_orig, v1_orig));
    }
    if(leaveOriginal)
      newFractal.push_back(Line(v0, v1));
  }
  return newFractal;
}

int main (int argc, char **argv) {

  osg::Camera* camera = createHUDCamera( 0, main_window_size.x(),
                                        0, main_window_size.y() );

  osg::Group* root = new osg::Group;
  root->addChild(camera);

  vector<Line> f0;
  f0.push_back(Line(osg::Vec2(0, 0),
                          osg::Vec2(0.33f, 0)));
  f0.push_back(Line(osg::Vec2(0.33f, 0),
                          osg::Vec2(0.33f, 0.33f)));
  f0.push_back(Line(osg::Vec2(0.33f, 0.33f),
                          osg::Vec2(0.66f, 0.33f)));
  f0.push_back(Line(osg::Vec2(0.66f, 0.33f),
                          osg::Vec2(0.66f, 0)));
  f0.push_back(Line(osg::Vec2(0.66f, 0),
                          osg::Vec2(1.0f, 0)));

  vector<Line> f1 = makeFractal(f0, f0, false);
  vector<Line> f2 = makeFractal(f1, f0, false);

  osg::Geode* geode = new osg::Geode;
  geode->addDrawable(drawFractal(f2));
  root->addChild(geode);

  osgViewer::Viewer viewer;

  viewer.setSceneData(root);
	viewer.addEventHandler( new PickHandler );
	viewer.setCameraManipulator( new TwoDimManipulator );
	viewer.setUpViewInWindow( 50, 50, 50 + main_window_size.x(),
                           50 + main_window_size.y(), 0 );
	viewer.run();

  return 0;
}

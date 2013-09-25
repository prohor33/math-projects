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
#include <time.h>

#include "two_dim_manipulator.h"

using namespace std;

class Line {
public:
  Line() : final(false) {};
  Line(osg::Vec2 v0, osg::Vec2 v1, bool final=false) :
      v0(v0), v1(v1), final(final) {};
  osg::Vec2 v0;
  osg::Vec2 v1;
  bool final;
};

vector<Line> makeFractal(vector<Line> fractal,
                         vector<Line> original,
                         bool leaveOriginal=false);
osg::Geometry* drawFractal(vector<Line> fractal);

class FractalManager {
public:
  FractalManager() :
      currFractalNumb(0),
      fractalStage(0) {};
  static FractalManager* instance() {
    static FractalManager FractalManager_;
    return &FractalManager_;
  }
  vector<Line> getCurrFractal() {
    if (fractalStage >= FractalStages.size()) {
      throw -1;
      vector<Line> null;
      return null;
    }
    return FractalStages.at(fractalStage);
  }
  void Next() {
    if(currFractalNumb >= (OriginalCollection.size()-1))
      return;
    currFractalNumb++;
    fractalStage = 0;
    FractalStages.clear();
    FractalStages.push_back(
        OriginalCollection.at(currFractalNumb));
    UpdateScene();
  }
  void Previous() {
    if(currFractalNumb <= 0)
      return;
    currFractalNumb--;
    fractalStage = 0;
    FractalStages.push_back(
      OriginalCollection.at(currFractalNumb));
    UpdateScene();
  }
  void Up() {
    fractalStage++;
    if (fractalStage >= FractalStages.size()) {
      try {
        FractalStages.push_back(
            makeFractal(FractalStages.at(fractalStage-1),
            OriginalCollection.at(currFractalNumb),
            flagLeaveOrigCollection.at(currFractalNumb)));
      }
      catch (int e) {
        if (e == -1) {
          // this stage is uncomplete
          FractalStages.pop_back();
          fractalStage--;
          return;
        }
      }
    }
    UpdateScene();
  }
  void Down() {
    if(fractalStage == 0)
      return;
    fractalStage--;
    UpdateScene();
  }
  void UpdateScene() {
    if (root->getNumChildren() > 1)
      root->removeChild(1);
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    try {
      geode->addDrawable(drawFractal(getCurrFractal()));
    }
    catch (int e) {
      if(e == -1)
        cout << "exception: can't get current fractal" << endl;
    }
    root->addChild(geode.get());
    printLog();
  }
  void addOriginal(vector<Line> original, bool flagLeave=false) {
    OriginalCollection.push_back(original);
    flagLeaveOrigCollection.push_back(flagLeave);
  }
  void setUp(osg::Group* root) {
    this->root = root;
    FractalStages.push_back(
        OriginalCollection.at(currFractalNumb));
    UpdateScene();
  }
  void printLog() {
    cout << "fractal " << currFractalNumb <<
        " on stage " << fractalStage << endl;
  }
private:
  int currFractalNumb;
  int fractalStage;
  vector< vector<Line> > FractalStages;
  vector<bool> flagLeaveOrigCollection;
  vector< vector<Line> > OriginalCollection;
  osg::Group* root;
};

#define FM FractalManager::instance()

osg::Vec2 main_window_size( 800, 500 );

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
        case osgGA::GUIEventAdapter::KEY_Right:
          FM->Next();
          break;
        case osgGA::GUIEventAdapter::KEY_Left:
          FM->Previous();
          break;
        case osgGA::GUIEventAdapter::KEY_Up:
          FM->Up();
          break;
        case osgGA::GUIEventAdapter::KEY_Down:
          FM->Down();
          break;
	  		case 'o':
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
	linewidth->setWidth(2.0f);
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
                         bool leaveOriginal) {
  clock_t startTime = clock();
  vector<Line> newFractal;
  vector<Line>::iterator cii;
  for (cii=fractal.begin(); cii!=fractal.end(); ++cii) {
    // check if too much time gone
    if (double(clock() - startTime) /
        (double)CLOCKS_PER_SEC > 5) {
      cout << "it takes too much time " <<
        "(more 8 seconds) on this computer..." <<
          endl << "canceled." << endl;
      throw(-1);
      return original;
    }
    osg::Vec2 v0, v1;
    v0 = (*cii).v0;
    v1 = (*cii).v1;
    if ((*cii).final) {
      newFractal.push_back(Line(v0, v1, true));
      continue;
    }
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
      newFractal.push_back(Line(v0, v1, true));
  }
  cout << newFractal.size() << " vertices in " <<
    double( clock() - startTime ) / (double)CLOCKS_PER_SEC <<
    " seconds." << endl;
  return newFractal;
}

int main (int argc, char **argv) {

  osg::Camera* camera = createHUDCamera( 0, main_window_size.x(),
                                        0, main_window_size.y() );

  osg::Group* root = new osg::Group;
  root->addChild(camera);

  vector<Line> f0;
  f0.push_back(Line(osg::Vec2(0, 0),
                          osg::Vec2(1.0f/3.0f, 0)));
  f0.push_back(Line(osg::Vec2(1.0f/3.0f, 0),
                          osg::Vec2(1.0f/3.0f, 1.0f/3.0f)));
  f0.push_back(Line(osg::Vec2(1.0f/3.0f, 1.0f/3.0f),
                          osg::Vec2(2.0f/3.0f, 1.0f/3.0f)));
  f0.push_back(Line(osg::Vec2(2.0f/3.0f, 1.0f/3.0f),
                          osg::Vec2(2.0f/3.0f, 0)));
  f0.push_back(Line(osg::Vec2(2.0f/3.0f, 0),
                          osg::Vec2(1.0f, 0)));


  FM->addOriginal(f0);
  FM->setUp(root);

  osgViewer::Viewer viewer;

  viewer.getCamera()->setClearColor(
      osg::Vec4(1.0f, 1.0f, 0.8f, 1.0f));
  viewer.setSceneData(root);
	viewer.addEventHandler( new PickHandler );
	viewer.setCameraManipulator( new TwoDimManipulator );
	viewer.setUpViewInWindow( 50, 50, 50 + main_window_size.x(),
                           50 + main_window_size.y(), 0 );

  // change window title
  viewer.realize();
  typedef osgViewer::Viewer::Windows Windows;
  Windows windows;
  viewer.getWindows(windows);
  for (Windows::iterator window = windows.begin();
       window != windows.end(); ++window)
  (*window)->setWindowName("Fractals");

	viewer.run();

  return 0;
}

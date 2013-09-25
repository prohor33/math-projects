/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 4 Recipe 9
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#include"two_dim_manipulator.h"

TwoDimManipulator::TwoDimManipulator()
:   _distance(1.0)
{
}

TwoDimManipulator::~TwoDimManipulator()
{
}

osg::Matrixd TwoDimManipulator::getMatrix() const
{
    osg::Matrixd matrix;
    matrix.makeTranslate( 0.0f, 0.0f, _distance );
    matrix.postMultTranslate( _center );
    return matrix;
}

osg::Matrixd TwoDimManipulator::getInverseMatrix() const
{
    osg::Matrixd matrix;
    matrix.makeTranslate( 0.0f, 0.0f,-_distance );
    matrix.preMultTranslate( -_center );
    return matrix;
}

void TwoDimManipulator::setByMatrix( const osg::Matrixd& matrix )
{
    setByInverseMatrix( osg::Matrixd::inverse(matrix) );
}

void TwoDimManipulator::setByInverseMatrix( const osg::Matrixd& matrix )
{
    osg::Vec3d eye, center, up;
    matrix.getLookAt( eye, center, up );
    
    _center = center; _center.z() = 0.0f;
    if ( _node.valid() )
        _distance = abs((_node->getBound().center() - eye).z());
    else
        _distance = abs((eye - center).length());
}

void TwoDimManipulator::home( double )
{
    if ( _node.valid() )
    {
        _center = _node->getBound().center(); _center.z() = 0.0f;
        _distance = 2.5 * _node->getBound().radius();
    }
    else
    {
        _center.set( osg::Vec3() );
        _distance = 1.0;
    }
}

bool TwoDimManipulator::performMovementLeftMouseButton(
    const double eventTimeDelta, const double dx, const double dy )
{
    _center.x() -= dx*_distance;
    _center.y() -= dy*_distance;
    return false;
}

bool TwoDimManipulator::performMovementRightMouseButton(
    const double eventTimeDelta, const double dx, const double dy )
{
    _distance *= (1.0 + dy/_distance);
    if ( _distance<0.001 ) _distance = 0.001;
    return false;
}

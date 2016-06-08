// This code implementation is the intellectual property of
// the GEANT4 collaboration.
//
// By copying, distributing or modifying the Program (or any work
// based on the Program) you indicate your acceptance of this statement,
// and all its terms.
//
// $Id: G4OpenGLTransform3D.hh,v 1.2.8.1 1999/12/07 20:53:18 gunter Exp $
// GEANT4 tag $Name: geant4-01-00 $
//
// 
// Andrew Walkden  24th October 1996
// G4OpenGLTransform3D provides OpenGL style transformation matrix
// from G4Transform3D.

#ifdef G4VIS_BUILD_OPENGL_DRIVER

#ifndef G4OPENGLTRANSFORM3D_HH
#define G4OPENGLTRANSFORM3D_HH

#include "G4Transform3D.hh"

class G4OpenGLTransform3D : public G4Transform3D {
public:
  G4OpenGLTransform3D (const G4Transform3D &t);
  const GLdouble* GetGLMatrix ();
private:
  GLdouble m[16];
};

#endif

#endif
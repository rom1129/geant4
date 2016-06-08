// This code implementation is the intellectual property of
// the GEANT4 collaboration.
//
// By copying, distributing or modifying the Program (or any work
// based on the Program) you indicate your acceptance of this statement,
// and all its terms.
//
// $Id: G4OpenGLXmVWidgetContainer.hh,v 1.2.8.1 1999/12/07 20:53:21 gunter Exp $
// GEANT4 tag $Name: geant4-01-00 $
//
// 
//Base class for all Motif container widgets

#ifdef G4VIS_BUILD_OPENGLXM_DRIVER

#ifndef G4OPENGLXMVWIDGETCONTAINER_HH
#define G4OPENGLXMVWIDGETCONTAINER_HH

#include "G4OpenGLXmVWidgetObject.hh"

class G4OpenGLXmVWidgetShell;
class G4OpenGLXmVWidgetComponent;

class G4OpenGLXmVWidgetContainer : public G4OpenGLXmVWidgetObject
{

public:
  G4OpenGLXmVWidgetContainer();   //constructor
  ~G4OpenGLXmVWidgetContainer();  //destructor

  virtual void AddChild (G4OpenGLXmVWidgetComponent*) = 0;
  virtual void AddYourselfTo (G4OpenGLXmVWidgetShell*) = 0;

  virtual Widget* GetPointerToParent () = 0;
  virtual Widget* GetPointerToWidget () = 0;
  
private:

};

#endif

#endif
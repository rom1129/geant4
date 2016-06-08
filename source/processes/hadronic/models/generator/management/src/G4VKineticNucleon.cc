// This code implementation is the intellectual property of
// the GEANT4 collaboration.
//
// By copying, distributing or modifying the Program (or any work
// based on the Program) you indicate your acceptance of this statement,
// and all its terms.
//
// $Id: G4VKineticNucleon.cc,v 1.1.8.1 1999/12/07 20:51:46 gunter Exp $
// GEANT4 tag $Name: geant4-01-00 $
//
#include "G4VKineticNucleon.hh"

G4VKineticNucleon::G4VKineticNucleon()
{
}

G4VKineticNucleon::G4VKineticNucleon(const G4VKineticNucleon &right)
{
}


G4VKineticNucleon::~G4VKineticNucleon()
{
}


//const G4VKineticNucleon & G4VKineticNucleon::operator=(const G4VKineticNucleon &right)
//{}


int G4VKineticNucleon::operator==(const G4VKineticNucleon &right) const
{
	return this == &right;
}

int G4VKineticNucleon::operator!=(const G4VKineticNucleon &right) const
{
	return this != &right;

}




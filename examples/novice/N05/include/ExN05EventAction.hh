// This code implementation is the intellectual property of
// the GEANT4 collaboration.
//
// By copying, distributing or modifying the Program (or any work
// based on the Program) you indicate your acceptance of this statement,
// and all its terms.
//
// $Id: ExN05EventAction.hh,v 1.2.8.1 1999/12/07 20:47:33 gunter Exp $
// GEANT4 tag $Name: geant4-01-00 $
//

#ifndef ExN05EventAction_h
#define ExN05EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"

class G4Event;

class ExN05EventAction : public G4UserEventAction
{
  public:
    ExN05EventAction();
    virtual ~ExN05EventAction();

  public:
    virtual void BeginOfEventAction(const G4Event*);
    virtual void EndOfEventAction(const G4Event*);

  private:
    G4int calorimeterCollID;
    G4int hadCalorimeterCollID;
    G4bool drawFlag;

  public:
    inline void SetDrawFlag(G4bool val)
    { drawFlag = val; };
};

#endif

    
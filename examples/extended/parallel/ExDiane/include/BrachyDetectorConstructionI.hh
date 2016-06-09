//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
// $Id: BrachyDetectorConstructionI.hh,v 1.3 2006/06/29 17:31:10 gunter Exp $
// GEANT4 tag $Name: geant4-08-01 $
//
//    ****************************************
//    *                                      *
//    *    BrachyDetectorConstructionI.hh     *
//    *                                      *
//    ****************************************
//Author: Susanna Guatelli
//
#ifndef BrachyDetectorConstructionI_H
#define BrachyDetectorConstructionI_H 1

#include "globals.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4LogicalVolume.hh"

class G4LogicalVolume;
class G4Tubs;
class G4Box;
class G4Sphere;
class G4Tubs;
class G4Colour;
class G4VPhysicalVolume;
class G4VPhysicalVolume;
class BrachyMaterial;

class BrachyDetectorConstructionI
{
public:
  BrachyDetectorConstructionI();
  ~BrachyDetectorConstructionI();
  void  ConstructIodium(G4VPhysicalVolume*);// Construct iodium source

private:
  //air tub ...
  G4Tubs* defaultTub; 
  G4LogicalVolume* defaultTubLog;
  G4VPhysicalVolume* defaultTubPhys;
  
  //source titanium capsule ...
  G4Tubs* capsule ;
  G4LogicalVolume*  capsuleLog;   
  G4VPhysicalVolume* capsulePhys;

  G4Sphere* capsuleTip;
  G4LogicalVolume* capsuleTipLog;
  G4VPhysicalVolume* capsuleTipPhys1;
  G4VPhysicalVolume* capsuleTipPhys2;

  //radioactive core ...
  G4Tubs* iodiumCore;
  G4LogicalVolume* iodiumCoreLog;
  G4VPhysicalVolume* iodiumCorePhys;

  // golden marker ...
  G4Tubs* marker;
  G4LogicalVolume* markerLog;
  G4VPhysicalVolume* markerPhys;

  BrachyMaterial* pMaterial;   
};
#endif








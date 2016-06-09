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
// The code was written by :
//	*Louis Archambault louis.archambault@phy.ulaval.ca,
//      *Luc Beaulieu beaulieu@phy.ulaval.ca
//      +Vincent Hubert-Tremblay at tigre.2@sympatico.ca
//
//
// *Centre Hospitalier Universitaire de Quebec (CHUQ),
// Hotel-Dieu de Quebec, departement de Radio-oncologie
// 11 cote du palais. Quebec, QC, Canada, G1R 2J6
// tel (418) 525-4444 #6720
// fax (418) 691 5268
//
// + Universit-A� Laval, Qu�bec (QC) Canada$)B
//*******************************************************
//
//*******************************************************
//
//*******************************************************
//
// DicomHandler.cc :
//	- Handling of DICM images
// 	- Reading headers and pixels
//	- Transforming pixel to density and creating *.g4
//	  files
// 	- Definitions are in DicomHandler.hh
//*******************************************************

#include "DicomHandler.hh"
#include "globals.hh"
#include "G4ios.hh"
#include <fstream>

#include <cctype>
#include <cstring>


DicomHandler::DicomHandler()
    : DATABUFFSIZE(8192), LINEBUFFSIZE(128), FILENAMESIZE(512),
      compression(0), max(0), rows(0), columns(0),
      bitAllocated(0), maxPixelValue(0), minPixelValue(0),
      pixelSpacingX(0.), pixelSpacingY(0.),
      sliceThickness(0.), sliceLocation(0.),
      rescaleIntercept(0), rescaleSlope(0),
      littleEndian(true), implicitEndian(false),
      pixelRepresentation(0) {
    ;
}

DicomHandler::~DicomHandler() {
    ;
}

G4int DicomHandler::readHeader(FILE *dicom, char * filename2)
{
    G4int returnvalue = 0;
    char * buffer = new char[LINEBUFFSIZE];

    implicitEndian = false;
    littleEndian = true;

    std::fread( buffer, 1, 128, dicom ); // The first 128 bytes 
                                         //are not important
    // Reads the "DICOM" letters
    std::fread( buffer, 1, 4, dicom );
    // if there is no preamble, the FILE pointer is rewinded.
    if(std::strncmp("DICM", buffer, 4) != 0) {
        std::fseek(dicom, 0, SEEK_SET);
        implicitEndian = true;
    }

    short readGroupId;    // identify the kind of input data 
    short readElementId;  // identify a particular type information
    short elementLength2; // deal with element length in 2 bytes
    G4int elementLength4; // deal with element length in 4 bytes

    char * data = new char[DATABUFFSIZE];

    // Read information up to the pixel data
    while(true) {

	//Reading groups and elements :
	readGroupId = 0;
	readElementId = 0;
	// group ID
	std::fread(buffer, 2, 1, dicom);
	getValue(buffer, readGroupId);
	// element ID
	std::fread(buffer, 2, 1, dicom);
	getValue(buffer, readElementId);

	// Creating a tag to be identified afterward
	G4int tagDictionary = readGroupId*0x10000 + readElementId;


	// VR or element length
	std::fread(buffer,2,1,dicom);
	getValue(buffer, elementLength2);

	// If value representation (VR) is OB, OW, SQ, UN, 
	//the next length is 32 bits
	if((elementLength2 == 0x424f ||  // "OB"
	    elementLength2 == 0x574f ||  // "OW"
	    elementLength2 == 0x5153 ||  // "SQ"
	    elementLength2 == 0x4e55) && // "UN"
	   !implicitEndian ) {           // explicit VR

	    std::fread(buffer, 2, 1, dicom); // Skip 2 reserved bytes

	    // element length
	    std::fread(buffer, 4, 1, dicom);
	    getValue(buffer, elementLength4);

	    // beginning of the pixels
	    if(tagDictionary == 0x7FE00010) break;

	    // Reading the information with data
	    std::fread(data, elementLength4,1,dicom);


	} else { // length is 16 bits :

	    if(!implicitEndian || readGroupId == 2) {
		// element length (2 bytes)
		std::fread(buffer, 2, 1, dicom);
		getValue(buffer, elementLength2);
		elementLength4 = elementLength2;

	    } else { 
		// element length (4 bytes)
		if(std::fseek(dicom, -2, SEEK_CUR) != 0) {
		    G4cerr << "[DicomHandler] fseek failed" << G4endl;
		    exit(-10);
		}
		std::fread(buffer, 4, 1, dicom);
		getValue(buffer, elementLength4);
	    }

	    // beginning of the pixels
	    if(tagDictionary == 0x7FE00010) break;

	    std::fread(data, elementLength4, 1, dicom);
	}

	// NULL termination
	data[elementLength4] = '\0';

	// analyzing inforamtion 
	getInformation(tagDictionary, data);
    }

    // Creating files to store information
    storeInformation(filename2);

    //
    delete [] buffer;
    delete [] data;

    return returnvalue;
}

//
void DicomHandler::getInformation(G4int & tagDictionary, char * data) {
    if(tagDictionary == 0x00280010 ) { // Number of Rows
	getValue(data, rows);
	std::printf("[0x00280010] Rows -> %i\n",rows);

    } else if(tagDictionary == 0x00280011 ) { // Number of columns
	getValue(data, columns);
	std::printf("[0x00280011] Columns -> %i\n",columns);

    } else if(tagDictionary == 0x00280102 ) { // High bits  ( not used )
	short highBits;
	getValue(data, highBits);
	std::printf("[0x00280102] High bits -> %i\n",highBits);

    } else if(tagDictionary == 0x00280100 ) { // Bits allocated
	getValue(data, bitAllocated);
	std::printf("[0x00280100] Bits allocated -> %i\n", bitAllocated);

    } else if(tagDictionary == 0x00280101 ) { //  Bits stored ( not used )
	short bitStored;
	getValue(data, bitStored);
	std::printf("[0x00280101] Bits stored -> %i\n",bitStored);

    } else if(tagDictionary == 0x00280106 ) { //  Min. pixel value
	getValue(data, minPixelValue);
	std::printf("[0x00280106] Min. pixel value -> %i\n", minPixelValue);

    } else if(tagDictionary == 0x00280107 ) { //  Max. pixel value
	getValue(data, maxPixelValue);
	std::printf("[0x00280107] Max. pixel value -> %i\n", maxPixelValue);

    } else if(tagDictionary == 0x00281053) { //  Rescale slope
	rescaleSlope = atoi(data);
	std::printf("[0x00281053] Rescale Slope -> %d\n", rescaleSlope);

    } else if(tagDictionary == 0x00281052 ) { // Rescalse intercept
	rescaleIntercept = atoi(data);
	std::printf("[0x00281052] Rescale Intercept -> %d\n", rescaleIntercept );

    } else if(tagDictionary == 0x00280103 ) {
	//  Pixel representation ( functions not design to read signed bits )
	pixelRepresentation = atoi(data); // 0: unsigned  1: signed 
	std::printf("[0x00280103] Pixel Representation -> %i\n", pixelRepresentation);
	if(pixelRepresentation == 1 ) {
	    std::printf("### PIXEL REPRESENTATION = 1, BITS ARE SIGNED, ");
	    std::printf("DICOM READING SCAN FOR UNSIGNED VALUE, POSSIBLE ");
	    std::printf("ERROR !!!!!! -> \n");
	}

    } else if(tagDictionary == 0x00080006 ) { //  Modality
	std::printf("[0x00080006] Modality -> %s\n", data);

    } else if(tagDictionary == 0x00080070 ) { //  Manufacturer
	std::printf("[0x00080070] Manufacturer -> %s\n", data);

    } else if(tagDictionary == 0x00080080 ) { //  Institution Name
	std::printf("[0x00080080] Institution Name -> %s\n", data);

    } else if(tagDictionary == 0x00080081 ) { //  Institution Address
	std::printf("[0x00080081] Institution Address -> %s\n", data);

    } else if(tagDictionary == 0x00081040 ) { //  Institution Department Name
	std::printf("[0x00081040] Institution Department Name -> %s\n", data);

    } else if(tagDictionary == 0x00081090 ) { //  Manufacturer's Model Name
	std::printf("[0x00081090] Manufacturer's Model Name -> %s\n", data);

    } else if(tagDictionary == 0x00181000 ) { //  Device Serial Number
	std::printf("[0x00181000] Device Serial Number -> %s\n", data);

    } else if(tagDictionary == 0x00080008 ) { //  Image type ( not used )
	std::printf("[0x00080008] Image Types -> %s\n", data);
	    
    } else if(tagDictionary == 0x00283000 ) { //  Modality LUT Sequence ( not used )
	std::printf("[0x00283000] Modality LUT Sequence SQ 1 -> %s\n", data);

    } else if(tagDictionary == 0x00283002 ) { // LUT Descriptor ( not used )
	std::printf("[0x00283002] LUT Descriptor US or SS 3 -> %s\n", data);

    } else if(tagDictionary == 0x00283003 ) { // LUT Explanation ( not used )
	std::printf("[0x00283003] LUT Explanation LO 1 -> %s\n", data);

    } else if(tagDictionary == 0x00283004 ) { // Modality LUT ( not used )
	std::printf("[0x00283004] Modality LUT Type LO 1 -> %s\n", data);

    } else if(tagDictionary == 0x00283006 ) { // LUT Data ( not used )
	std::printf("[0x00283006] LUT Data US or SS -> %s\n", data);

    } else if(tagDictionary == 0x00283010 ) { // VOI LUT ( not used )
	std::printf("[0x00283010] VOI LUT Sequence SQ 1 -> %s\n", data);

    } else if(tagDictionary == 0x00280120 ) { // Pixel Padding Value ( not used )
	std::printf("[0x00280120] Pixel Padding Value US or SS 1 -> %s\n", data);

    } else if(tagDictionary == 0x00280030 ) { // Pixel Spacing
	char * buff = new char[LINEBUFFSIZE];
	char * sepPos = index(data, '\\');
	char * termPos = rindex(data, '\0');
	std::strncpy(buff, data, sepPos - data);
	pixelSpacingX = atof(buff);
	std::strncpy(buff, sepPos+1, termPos - sepPos - 1);
	pixelSpacingY = atof(buff);
	std::printf("[0x00280030] Pixel Spacing (mm) -> %s\n", data);
	delete [] buff;

    } else if(tagDictionary == 0x00200037 ) { // Image Orientation ( not used )
	std::printf("[0x00200037] Image Orientation (Patient) -> %s\n", data);

    } else if(tagDictionary == 0x00200032 ) { // Image Position ( not used )
	std::printf("[0x00200032] Image Position (Patient,mm) -> %s\n", data);

    } else if(tagDictionary == 0x00180050 ) { // Slice Thickness
	sliceThickness = atof(data);
	std::printf("[0x00180050] Slice Thickness (mm) -> %f\n", sliceThickness);

    } else if(tagDictionary == 0x00201041 ) { // Slice Location
	sliceLocation = atof(data);
	std::printf("[0x00201041] Slice Location -> %f\n", sliceLocation);

    } else if(tagDictionary == 0x00280004 ) { // Photometric Interpretation ( not used )
	std::printf("[0x00280004] Photometric Interpretation -> %s\n", data);

    } else if(tagDictionary == 0x00020010) { // Endian
	if(strcmp(data, "1.2.840.10008.1.2") == 0)
	    implicitEndian = true;
	else if(strncmp(data, "1.2.840.10008.1.2.2", 19) == 0)
	    littleEndian = false;
	//else 1.2.840..10008.1.2.1 (explicit little endian)
		   
	std::printf("[0x00020010] Endian -> %s\n", data);
    }

    // others
    else {
	std::printf("[0x%x] -> %s\n", tagDictionary, data);

    }

}

//
void DicomHandler::storeInformation(char * _filename) {

    char * compressionbuf = new char[LINEBUFFSIZE];
    char * maxbuf = new char[LINEBUFFSIZE];
    char * filename = new char[FILENAMESIZE];
    compression = 0;
    max = 0;
    FILE* configuration;

    configuration = std::fopen("Data.dat","r");
    if( configuration != 0 ) {
	std::fscanf(configuration,"%s",compressionbuf);
	compression = atoi(compressionbuf);
	std::fscanf(configuration,"%s",maxbuf);
	max = atoi(maxbuf);
	std::fclose(configuration);
    } else {
	std::printf("### WARNING, file Data.dat not here !!!\n");
	exit(1);
    }


    FILE* dat;
    std::sprintf(filename,"%s.dat", _filename);
    dat = std::fopen(filename,"w+");
    // Note: the .dat files contain basic information on the images.

    std::fprintf(dat,"Rows,columns(#):      %8i   %8i\n",rows,columns);
    std::fprintf(dat,"PixelSpacing_X,Y(mm): %8f   %8f\n",
                 pixelSpacingX,pixelSpacingY);
    std::fprintf(dat,"SliceThickness(mm):   %8f\n",sliceThickness);
    std::fprintf(dat,"SliceLocation(mm):    %8f\n",sliceLocation);
    std::fclose(dat);

    delete [] compressionbuf;
    delete [] maxbuf;
    delete [] filename;

}

//
G4int DicomHandler::readData(FILE *dicom,char * filename2)
{
    G4int returnvalue = 0;
    char * compressionbuf = new char[LINEBUFFSIZE];
    char * maxbuf = new char[LINEBUFFSIZE];
    short compression = 0;
    G4int max = 0;

    FILE* configuration = std::fopen("Data.dat","r");
    std::fscanf(configuration,"%s",compressionbuf);
    compression = atoi(compressionbuf);
    std::fscanf(configuration,"%s",maxbuf);
    max = atoi(maxbuf);
    std::fclose(configuration);

    //  READING THE PIXELS :
    G4int w = 0;
    G4int len = 0;
    
    G4int** tab = new G4int*[rows];
    for ( G4int i = 0; i < rows; i ++ ) {
      tab[i] = new G4int[columns];
    }

    if(bitAllocated == 8) { // Case 8 bits :

	std::printf("@@@ Error! Picture != 16 bits...\n");
	std::printf("@@@ Error! Picture != 16 bits...\n"); 
	std::printf("@@@ Error! Picture != 16 bits...\n"); 

	unsigned char ch = 0;

	len = rows*columns;
	for(G4int j = 0; j < rows; j++) {
	    for(G4int i = 0; i < columns; i++) {
		w++;
		std::fread( &ch, 1, 1, dicom);
		tab[j][i] = ch*rescaleSlope + rescaleIntercept;
	    }
	}
	returnvalue = 1;

    } else { //  from 12 to 16 bits :
	char sbuff[2];
	short pixel;
	len = rows*columns;
	for( G4int j = 0; j < rows; j++) {
	    for( G4int i = 0; i < columns; i++) {
		w++;
		std::fread(sbuff, 2, 1, dicom);
		getValue(sbuff, pixel);
		tab[j][i] = pixel*rescaleSlope + rescaleIntercept;
	    }
	}
    }

    // Creation of .g4 files wich contains averaged density data

    char * nameProcessed = new char[FILENAMESIZE];
    FILE* processed;

    std::sprintf(nameProcessed,"%s.g4",filename2);
    processed = std::fopen(nameProcessed,"w+b");
    std::printf("### Writing of %s ###\n",nameProcessed);

    std::fwrite(&rows, 2, 1, processed);
    std::fwrite(&columns, 2, 1, processed);
    std::fwrite(&pixelSpacingX, 8, 1, processed);
    std::fwrite(&pixelSpacingY, 8, 1, processed);
    std::fwrite(&sliceThickness, 8, 1, processed);
    std::fwrite(&sliceLocation, 8, 1, processed);
    std::fwrite(&compression, 2, 1, processed);

    std::printf("%8i   %8i\n",rows,columns);
    std::printf("%8f   %8f\n",pixelSpacingX,pixelSpacingY);
    std::printf("%8f\n", sliceThickness);
    std::printf("%8f\n", sliceLocation);
    std::printf("%8i\n", compression);

    G4int compSize = compression;
    G4int mean;
    G4double density;
    G4bool overflow = false;
    G4int cpt=1;

    if(compSize == 1) { // no compression: each pixel has a density value)
	for( G4int ww = 0; ww < rows; ww++) {
	    for( G4int xx = 0; xx < columns; xx++) {
		mean = tab[ww][xx];
		density = pixel2density(mean);
		std::fwrite(&density, sizeof(G4double), 1, processed);
            }
        }

    } else {
	// density value is the average of a square region of
	// compression*compression pixels
	for(G4int ww = 0; ww < rows ;ww += compSize ) {
	    for(G4int xx = 0; xx < columns ;xx +=compSize ) {
		overflow = false;
		mean = 0;
		for(int sumx = 0; sumx < compSize; sumx++) {
		    for(int sumy = 0; sumy < compSize; sumy++) {
			if(ww+sumy >= rows || xx+sumx >= columns) overflow = true;
			mean += tab[ww+sumy][xx+sumx];
		    }
		    if(overflow) break;
		}
		mean /= compSize*compSize;
		cpt = 1;

		if(!overflow) {
		    G4double density = pixel2density(mean);
		    std::fwrite(&density, sizeof(G4double), 1, processed);
		}
	    }

	}
    }
    std::fclose(processed);

    delete [] compressionbuf;
    delete [] maxbuf;
    delete [] nameProcessed;

    for ( G4int i = 0; i < rows; i ++ ) {
      delete [] tab[i];
    }
    delete [] tab;

    return returnvalue;
}

/*
  G4int DicomHandler::displayImage(char command[300])
  {
  //   Display DICOM images using ImageMagick
  char commandName[500];
  std::sprintf(commandName,"display  %s",command);
  std::printf(commandName);
  G4int i = system(commandName);
  return (G4int )i;
  }
*/

G4double DicomHandler::pixel2density(G4int pixel)
{
    G4double density = -1.;
    G4int nbrequali = 0;
    G4double deltaCT = 0;
    G4double deltaDensity = 0;

    // CT2Density.dat contains the calibration curve to convert CT (Hounsfield)
    // number to physical density
    std::ifstream calibration("CT2Density.dat");
    calibration >> nbrequali;

    G4double * valuedensity = new G4double[nbrequali];
    G4double * valueCT = new G4double[nbrequali];

    if(!calibration) {
	G4cerr << "@@@ No value to transform pixels in density!" << G4endl;
	exit(1);

    } else { // calibration was successfully opened
	for(G4int i = 0; i < nbrequali; i++) { // Loop to store all the pts in CT2Density.dat
	    calibration >> valueCT[i] >> valuedensity[i];
	}
    }
    calibration.close();

    for(G4int j = 1; j < nbrequali; j++) {
	if( pixel >= valueCT[j-1] && pixel < valueCT[j]) {

	    deltaCT = valueCT[j] - valueCT[j-1];
	    deltaDensity = valuedensity[j] - valuedensity[j-1];

	    // interpolating linearly
	    density = valuedensity[j] - ((valueCT[j] - pixel)*deltaDensity/deltaCT );
	    break;
	}
    }

    if(density < 0.) {
	std::printf("@@@ Error density = %f && Pixel = %i (0x%x) && deltaDensity/deltaCT = %f\n",density,pixel,pixel, deltaDensity/deltaCT);
    }
    
    delete [] valuedensity;
    delete [] valueCT;

    return density;
}


void DicomHandler::checkFileFormat()
{
    std::ifstream checkData("Data.dat");
    char * oneLine = new char[128];
    G4int nbImages;

    if(!(checkData.is_open())) { //Check existance of Data.dat

	G4cout << "\nDicomG4 needs Data.dat :\n\tFirst line: number of image pixel for a "
	       << "voxel (G4Box)\n\tSecond line: number of images (CT slices) to "
	       << "read\n\tEach following line contains the name of a Dicom image except "
	       << "for the .dcm extension\n";
	exit(0);
    }

    checkData >> nbImages;
    checkData >> nbImages;
    G4String oneName;
    checkData.getline(oneLine,100);
    std::ifstream testExistence;
    G4bool existAlready = true;
    for(G4int rep = 0; rep < nbImages; rep++) { 
	checkData.getline(oneLine,100);
	oneName = oneLine;
	oneName += ".g4"; // create dicomFile.g4
	testExistence.open(oneName.data());
	if(!(testExistence.is_open())) {
	    existAlready = false;
	    testExistence.clear();
	    testExistence.close();
	    break;
	}
	testExistence.clear();
	testExistence.close();
    }
    checkData.close();
    delete [] oneLine;

    if( existAlready == false ) { // The files *.g4 have to be created

	G4cout << "\nAll the necessary images were not found in processed form, starting "
	       << "with .dcm images\n";

	FILE * dicom;
	FILE * lecturePref;
	char * compressionc = new char[LINEBUFFSIZE];
	char * maxc = new char[LINEBUFFSIZE];
	//char name[300], inputFile[300];
	char * name = new char[FILENAMESIZE];
	char * inputFile = new char[FILENAMESIZE];

	lecturePref = std::fopen("Data.dat","r");
	std::fscanf(lecturePref,"%s",compressionc);
	compression = atoi(compressionc);
	std::fscanf(lecturePref,"%s",maxc);
	max = atoi(maxc);

	for( G4int i = 1; i <= max; i++ ) { // Begin loop on filenames

	    std::fscanf(lecturePref,"%s",inputFile);
	    std::sprintf(name,"%s.dcm",inputFile);
	    std::cout << "check 1: " << name << std::endl;
	    //  Open input file and give it to gestion_dicom :
	    std::printf("### Opening %s and reading :\n",name);
	    dicom = std::fopen(name,"rb");
	    // Reading the .dcm in two steps:
	    //      1.  reading the header
	    //	2. reading the pixel data and store the density in Moyenne.dat
	    if( dicom != 0 ) {
		readHeader(dicom,inputFile);
		readData(dicom,inputFile);
	    } else {
		G4cout << "\nError opening file : " << name << G4endl;
		exit(0);
	    }
	    std::fclose(dicom);
	}
	std::fclose(lecturePref);

	delete [] compressionc;
	delete [] maxc;
	delete [] name;
	delete [] inputFile;

    } 

}


template <class Type>
void DicomHandler::getValue(char * _val, Type & _rval) {

#if BYTE_ORDER == BIG_ENDIAN
    if(littleEndian) {      // little endian
#else // BYTE_ORDER == LITTLE_ENDIAN
    if(!littleEndian) {     // big endian
#endif
	const int SIZE = sizeof(_rval);
	char ctemp;
	for(int i = 0; i < SIZE/2; i++) {
	    ctemp = _val[i];
	    _val[i] = _val[SIZE - 1 - i];
	    _val[SIZE - 1 - i] = ctemp;
	}
    }
    _rval = *(Type *)_val;
}
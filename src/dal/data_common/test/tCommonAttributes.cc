/***************************************************************************
 *   Copyright (C) 2009                                                    *
 *   Lars B"ahren (bahren@astron.nl)                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <data_common/CommonAttributes.h>

// Namespace usage
using std::cout;
using std::endl;
using DAL::CommonAttributes;

/*!
  \file tCommonAttributes.cc

  \ingroup DAL
  \ingroup data_common

  \brief A collection of test routines for the CommonAttributes class
 
  \author Lars B&auml;hren
 
  \date 2009/08/31
*/

//_______________________________________________________________________________
//                                                              test_constructors

/*!
  \brief Test constructors for a new CommonAttributes object

  \return nofFailedTests -- The number of failed tests encountered within this
          function.
*/
int test_constructors ()
{
  cout << "\n[tCommonAttributes::test_constructors]\n" << endl;

  int nofFailedTests (0);
  DAL::Filename filename ("123456789","",DAL::Filename::uv,DAL::Filename::h5);
  
  cout << "[1] Testing default constructor ..." << endl;
  try {
    CommonAttributes attributes;
    //
    attributes.summary(); 
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }
  
  cout << "[2] Testing argumented constructor ..." << endl;
  try {
    std::string filetype ("tbb");
    std::string filedate ("2009-10-10T00:00:00.0");
    CommonAttributes attributes (filename,
				 filetype,
				 filedate);
    //
    attributes.summary(); 
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }
  
  cout << "[3] Testing copy constructor ..." << endl;
  try {
    std::string filetype ("tbb");
    std::string filedate ("2009-10-10T00:00:00.0");
    CommonAttributes attributes (filename,
				 filetype,
				 filedate);
    attributes.summary();
    //
    CommonAttributes attributesCopy (attributes);
    attributesCopy.summary();
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }
  
  return nofFailedTests;
}

//_______________________________________________________________________________
//                                                                   test_methods

/*!
  \brief Test access to the individual attributes

  \return nofFailedTests -- The number of failed tests encountered within this
          function.
*/
int test_methods (std::string const &outfile)
{
  cout << "\n[tCommonAttributes::test_methods]" << endl;

  int nofFailedTests (0);
  DAL::Filename filename ("123456789","",DAL::Filename::uv,DAL::Filename::h5);
  CommonAttributes attr;

  // Assign new values to the attributes ___________________

  cout << "\n[1] Assign new values to the attributes ..." << endl;
  try {
    std::vector<std::string> stations;
    stations.push_back("CS001");
    stations.push_back("CS003");
    stations.push_back("CS005");

    attr.setFilename (filename);
    attr.setFiletype ("tbb");
    attr.setFiledate ("2009-10-10T01:00:00.0");
    attr.setTelescope ("LOFAR");
    attr.setProjectID ("CR-2009-10-10");
    attr.setProjectTitle ("CR test observation");
    attr.setProjectPI ("Mr. CR");
    attr.setProjectCoI ("Mrs. CR");
    attr.setProjectContact ("cr@lofar.org");
    attr.setObserver("Mr. LOFAR");
    // attr.setObservationID ("1234567890");
    attr.setAntennaSet ("LBA_X");
    attr.setFilterSelection ("LBH_30_80");
    attr.setClockFrequency (200);
    attr.setClockFrequencyUnit ("MHz");
    attr.setTarget ("Everything under the sky");
    attr.setSystemVersion ("LOFAR system v1.0");
    attr.setPipelineName ("CR_Offline");
    attr.setPipelineVersion ("v0.9");
    // attr.setStationsList (stations);
    attr.setNotes ("Life is a Lemmon");
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }

  cout << "\n[2] Show new values of the attributes ..." << endl;
  try {
    attr.summary();
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }

  cout << "\n[3] Create a copy of the object storing the attributes ..." << endl;
  try {
    CommonAttributes attrCopy (attr);
    //
    attrCopy.summary();
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }

  // Write attributes to HDF5 file _________________________

#ifdef DAL_WITH_HDF5
  
  cout << "\n[4] Write attributes to file ..." << endl;
  try {
    hid_t fileID (0);
    herr_t h5error (0);
    // create a new HDF5 file
    fileID = H5Fcreate (outfile.c_str(),
			H5F_ACC_TRUNC,
			H5P_DEFAULT,
			H5P_DEFAULT);
    // write the attributes to the root group of the file
    attr.h5write (fileID);
    // close the HDF5 file once we are done
    h5error = H5Fclose (fileID);
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }

  cout << "\n[5] Read the attributes back in from file ..." << endl;
  try {
    hid_t fileID (0);
    herr_t h5error (0);
    // open the file from which to read in the attributes
    fileID = H5Fopen (outfile.c_str(),
		      H5F_ACC_RDWR,
		      H5P_DEFAULT);
    // close the HDF5 file once we are done
    h5error = H5Fclose (fileID);
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }

#endif

  // Retrieve attributes through casa::Record ______________

#ifdef DAL_WITH_CASA

  cout << "[6] Testing getAttributes(casa::Record) ..." << endl;
  {
    casa::Record rec;

    attr.getAttributes (rec);
    
    cout << rec << endl;
  }

#endif
  
  return nofFailedTests;
}

//_______________________________________________________________________________
//                                                                           main

int main ()
{
  int nofFailedTests (0);
  std::string filename ("tCommonAttributes.h5");
  
  // Test for the constructor(s)
  nofFailedTests += test_constructors ();
  // Test access to the individual attributes
  nofFailedTests += test_methods (filename);

  return nofFailedTests;
}

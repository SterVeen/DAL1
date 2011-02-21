
/***************************************************************************
*   Copyright (C) 2007                                                    *
*   Joseph Masters                                                        *
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

/*!
  \file lopes2h5.cpp

  \ingroup DAL
  \ingroup dal_apps

  \brief Convert LopesEvent format data to HDF5-based time-series format

  \author Joseph Masters, Lars B&auml;hren

  <h3>Usage</h3>

  In order to convert a data stored in a LopesEvent data file to the time-series
  data format defined for the DAL use:
  \verbatim
  ./lopes2h5 <LopesEvent file> <HDF5 file>
  \endverbatim
*/

#include <fstream>

#include <dal_config.h>
#include <dalDataset.h>
#include <dalGroup.h>
#include <TBB.h>
#include <LOPES_EventFile.h>

using namespace DAL;

void export_data (std::string const &filename,
                  casa::Matrix<short> const &data)
{
  std::ofstream outfile;
  int nofAntennas (0);
  int nofSamples (0);
  int antenna (0);
  int sample (0);

  data.shape(nofSamples,nofAntennas);

  std::cout << "-- Name of output file      = " << filename     << std::endl;
  std::cout << "-- Shape of the data array  = " << data.shape() << std::endl;
  std::cout << "-- nof. antennas            = " << nofAntennas  << std::endl;
  std::cout << "-- nof. samples per antenna = " << nofSamples   << std::endl;

  std::cout << "-- writing data to file ... " << std::flush;
  outfile.open (filename.c_str());
  for (sample=0; sample<nofSamples; sample++)
    {
      for (antenna=0; antenna<nofAntennas; antenna++)
        {
          outfile << data(sample,antenna) << "\t";
        }
      outfile << std::endl;
    }
  outfile.close();
  std::cout << "done" << std::endl;

}

int main (int argc, char *argv[])
{

  // parameter check
  if ( argc < 3 )
    {
      cout << endl << "Too few parameters..." << endl << endl;
      cout << "The first parameter is the raw LOPES input file name." << endl;
      cout << "The second parameter is the hdf5 dataset name." << endl;
      cout << endl;
      return DAL::FAIL;
    }

  dalDataset * dataset;
  if ( NULL == argv[3] )
    dataset = new dalDataset( argv[2], "HDF5" );
  else
    dataset = new dalDataset( argv[2], argv[3] );

  //
  /////////////////////////////////////////
  // create the "Station" group
  /////////////////////////////////////////
  //

  dalGroup * stationGroup = dataset->createGroup( "Station" );

  std::vector<string> telescope        (1, "LOFAR"       );
  std::vector<string> observer         (1, "I.B. User"   );
  std::vector<string> project          (1, "Cosmic Rays" );
  std::vector<string> observation_id   (1, "1287"        );
  std::vector<string> observation_mode (1, "Normal");

  // Add attributes to "Station" group
  stationGroup->setAttribute ("TELESCOPE", telescope );
  stationGroup->setAttribute ("OBSERVER",  observer );
  stationGroup->setAttribute ("PROJECT",   project );
  stationGroup->setAttribute ("OBS_ID",    observation_id );
  stationGroup->setAttribute ("OBS_MODE",  observation_mode );
  /**************************************************************************/
  std::string filename ("UNDEFINED");

  if (argc < 2)
    {
      cerr << "[tLopesEvent] Missing name of event file!" << endl;
    }
  else
    {
      filename = argv[1];
    }

  /**************************/
  try
    {
      short *data;
      unsigned int nofAntennas (0);
      unsigned int blocksize (0);



      LOPES_EventFile event (filename);
      const unsigned int BUFSIZE=10000;
      typedef struct CosmicRayStruct {
	//int nofDatapoints;
	Int16 data;
      } CosmicRayStruct;
      typedef struct CRwritebuffer {
	CosmicRayStruct cr[ BUFSIZE ];
      } CRwritebuffer;
      CRwritebuffer wb;
      
      nofAntennas = event.nofAntennas();
      stationGroup->setAttribute("NUM_ANTS", &nofAntennas );
      
      blocksize   = event.blocksize();
      
      unsigned int samps =  blocksize;
      stationGroup->setAttribute("NUMSAMPS", &samps );
      
      data = new short[ blocksize ];
      
      for (unsigned int antenna(0); antenna<nofAntennas; antenna++)
        {
	  
          string antName = "ANTENNA" + stringify(antenna);
          dalTable * AntennaTable = dataset->createTable( antName, "Station" );
          int ri = 1;
          AntennaTable->setAttribute("RSP_ID", &ri );
          AntennaTable->addColumn( "DATA", dal_SHORT );
	  
          event.data(data,antenna);
          unsigned int jj=0;
	  
          // for all of the datapoint for an antenna
          while (jj < blocksize) {
	    // fill a write buffer, checking to make sure we haven't
	    //   reached the end of the input data array
	    unsigned int ii;
	    for (ii=0;
		 (ii < BUFSIZE) && ((jj+ii)<blocksize);
		 ii++ )
	      {
		wb.cr[ii].data = data[jj+ii];
	      }
	    // write the data buffer to the output file
	    AntennaTable->appendRows(&wb,ii);
	    jj += BUFSIZE;
	  }
          delete AntennaTable;
        }
      
    }
  catch (std::string message) {
    std::cerr << message << std::endl;
  }
  /**************************/
  
  delete stationGroup;
  delete dataset;
  
  return 0;
}

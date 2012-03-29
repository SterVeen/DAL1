/***************************************************************************
 *   Copyright (C) 2006 by Joseph Masters                                  *
 *   jmasters@science.uva.nl                                               *
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

/**
  \file msread.cpp

  \ingroup DAL1

  \brief Test program for reading in measurement set data stored as HDF5.

  \author Joseph Masters

  \date 12-04-06
*/

#include <core/dalDataset.h>
#include <core/dalGroup.h>

using namespace DAL1;

const long BSIZE = 10000;
const int LOOPMAX = 10000;

/*! doxygen comment in dal.cpp */
int main(int argc, char *argv[])
{

  // parameter check
  if ( argc < 2 )
    {
      cout << endl << "Too few parameters..." << endl << endl;
      cout << "The first parameter is the dataset name." << endl;
      cout << "The second parameter is the filetype. (optional)" << endl;
      cout << endl;
      return DAL1::FAIL;
    }


  dalDataset * dataset = new dalDataset;
  /*
    if ( NULL == argv[2] )
  	  dataset = new dalDataset( argv[1] );
    else
  	  dataset = new dalDataset( argv[1], argv[2] );
    */

  if ( 0 != dataset->open( argv[1] ) )
    {
      cout << "Problem opening dataset: " << argv[1] << '.' << " Quiting." << endl;
      return(DAL1::FAIL);
    }

  // define the structure of a table
  // define the data to go in the table
  // create the table in the file or group
  dalTable * tableA = dataset->openTable("MAIN","Sub");

  typedef struct skycol
    {
      int x;
      int y;
    } skycol;

  // describe and fill data, and provide offsets and types
  // describe the data or structure of the table
  typedef struct MainTable
    {
      double time;
      int antenna1;
      int antenna2;
      int feed1;
      int feed2;
      int data_desc_id;
      int processor_id;
      int field_id;
      double interval;
      double exposure;
      double time_centroid;
      int scan_number;
      int array_id;
      int observation_id;
      int state_id;
      double uvw[3];
      skycol sky;
    } MainTable;

// define a structure to read the data into
//   in this case it's MainTable (defined above)
  const int NUMBERROWS = 1;
  long startRow = 0;
  MainTable data_out[NUMBERROWS];
// first param is the structure to read into
// second param is the row to start with
// third param is the number of rows to read
  long maximum = BSIZE * LOOPMAX;
  cout << maximum << endl;
//for ( long ii = 0; ii < maximum; ii += NUMBERROWS ) {
  for ( long ii = 0; ii < 10; ii ++ )
    {

      tableA->readRows( data_out, startRow, NUMBERROWS);

      // print some values from the read
      for (int gg=0; gg < NUMBERROWS; gg++)
        {
          cout << data_out[gg].time << ',';
          cout << data_out[gg].antenna1 << ',';
          cout << data_out[gg].antenna2 << ',';
          cout << data_out[gg].feed1 << endl;
        }

      startRow += NUMBERROWS;
    }
  delete tableA;


  // define the structure of an image
  // define the data to go in the image
  // create the image in the file or group
//   dataset.createImage();


  delete dataset;

  cout << "SUCCESS" << endl;
  return DAL1::SUCCESS;
}

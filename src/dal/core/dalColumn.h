/***************************************************************************
 *   Copyright (C) 2007 by Joseph Masters                                  *
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

#ifndef DALCOLUMN_H
#define DALCOLUMN_H

#include <core/dalCommon.h>
#include <core/dalData.h>
#include <core/dalFileType.h>

#ifdef PYTHON
#include <pydal/num_util.h>
#endif

#ifdef DAL_WITH_CASA
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#endif

namespace DAL {
  
  /*!
    \class dalColumn
    \ingroup DAL
    \ingroup core
    \brief Represents a table column.
    \author Joseph Masters
    \date 12-11-2006
  */
  class dalColumn
  {
    //! File type: CASA_MS, HDF5, FITS, etc.
    dalFileType itsFiletype;
    //! Name of the column
    std::string name;
    //! Name of the table
    std::string itsTablename;
    std::string itsDatatype;   // Column datatype
    //! Datatype size
    int size;
    //! Number of rows within this column
    uint itsNofRows;

    //! HDF5 file object identifier
    hid_t itsFileID;
    //! HDF5 table object identifier
    hid_t itsTableID;
    //! HDF5 field count
    hsize_t itsNofFields;
    //! HDF5 record count
    hsize_t itsNofRecords;
    //! Identifier for the column type
    hid_t coltype;
    //! HDF5 call return status
    herr_t  status;
    //! Object to hold the actual column data
    dalData * itsColumnData;
    
#ifdef DAL_WITH_CASA
    
    //! Column data type
    std::string itsCasaDatatype;
    //! Column descriptor
    casa::ColumnDesc itsColumnDesc;
    casa::ROTableColumn * itsROTableColumn;
    
    // COLUMNs
    casa::ROArrayColumn<int> * roac_int;
    casa::ROArrayColumn<double> * roac_dbl;
    casa::ROArrayColumn< std::complex< float > > * roac_comp;
    casa::ROArrayColumn<casa::String> * roac_string;
    
    casa::ROScalarColumn<int> * rosc_int;
    casa::ROScalarColumn<bool> * rosc_bool;
    casa::ROScalarColumn<double> * rosc_dbl;
    casa::ROScalarColumn< std::complex< float > > * rosc_comp;
    casa::ROScalarColumn<casa::String> * rosc_string;
    
    // VECTORs
    vector< std::complex< float > > stl_vec_comp;
    
    bool deleteIt;
    
    std::string getCasaDataType();
    dalData * CasaData_scalar( );
    dalData * CasaData_array( );
#endif
    
    dalData * H5data (int &start,
		      int &length);
    
  public:

    // === Construction =========================================================

    //! Default constructor
    dalColumn();
    //! Create a new complex column.
    dalColumn (std::string const &complexcolname);
    //! Argumented constructor for a new column object.
    dalColumn (std::string const &colname,
	       std::string const &coltype);
    //! Argumented constructor for a new column object.
    dalColumn( hid_t const &fileid,
	       hid_t const &tableid,
	       dalFileType const &filetype,
	       std::string const &tablename,
	       std::string const &colname,
	       std::string const &coldatatype );
    
#ifdef DAL_WITH_CASA
    //! Create a new column object from a CASA table.
    dalColumn (casa::Table table,
	       std::string colname);
#endif
    
    // === Public methods =======================================================
    
    //! Add a member to a compound column.
    void addMember (std::string const &member_name,
		    std::string const &type);
    /*!
      \brief Get the name of the column.
      \return name -- The name of the column.
    */
    inline std::string getName() {
      return name;
    }
    //! Set the name of the column.
    void setName(std::string colname);
    //! Set the file type of the dataset containing the column.
    void setFiletype (std::string const &type);
    //! Set the file type of the dataset containing the column.
    void setFiletype (dalFileType const &filetype);
    //! Get the type of the data stored inside the column.
    std::string getDataType();
    int getSize();
    //! Close the column.
    void close();
    //! Retrieve the datatype of the column.
    inline std::string getType() {
      return itsDatatype;
    }
    //! Is the column an array?
    bool isArray();
    //! Is the column a scalar?
    bool isScalar();
    //! Get the number of dimensions of the column.
    int ndims();
    //! Get the shape of the column data
    std::vector<int> shape();
    //! Get the number of rows in the column.
    uint nofRows ();
    //! Get the data object for the column.
    dalData * data (int &start,
		    int &length)
    {

      switch (itsFiletype.type()) {
#ifdef DAL_WITH_CASA
      case dalFileType::CASA_MS:
	{
	  try {
	    if ( isScalar() ) {
	      return CasaData_scalar();
	    }
	    else if ( isArray() ) {
	      return CasaData_array();
	    }
	    else {
	      std::cerr << "dalColumn::data() Column is neither "
			<< "scalar nor array.  "
			<< "Do not know how to handle.\n";
	      return NULL;
	    }
	  } catch (casa::AipsError x) {
	    std::cerr << "ERROR: " << x.getMesg() << endl;
	    return NULL;
	  }
	}
	break;
#endif
      case dalFileType::HDF5:
	{
	  return H5data (start, length);
	}
	break;
      default:
	{
	  std::cerr << "[dalColumn::data] Not yet supported for "
		    << itsFiletype.name()
		    << std::endl;
	  return NULL;
	}
	break;
      };
      
    }
    //! Get the data object for the column.
    dalData * data() {
      int start  = -1;
      int length = -1;
      return data (start, length);
    }
    /*!
      \brief Retrieve data from the column
      \param columnData -- Data stored inside the table column
      \return status    -- Status of the operation; returns \e false in case
              an error was encountered.
    */
    template <class T>
      bool readData (std::vector<T> &columnData)
      {
	bool status        = true;
	dalData * buffer   = data();
	unsigned int nelem = nofRows();
	/* Adjust the size of the array returning the data */
	if (nelem>0) {
	  columnData.resize(nelem);
	} else {
	  return false;
	}
	/* Copy data values */
	for (unsigned int n=0; n<nelem; ++n) {
	  columnData[n] = *(T*)(buffer->get(n));
	}
	/* Debugging feedback */
	for (unsigned int n=0; n<10; ++n) {
	  std::cout << columnData[n] << std::endl;
	}
	/* Return status */
	return status;
      }
    
    //! Provide a summary of the object's internal parameters and status
    inline void summary() {
      summary(std::cout);
    }
    //! Provide a summary of the object's internal parameters and status
    void summary(std::ostream &os);

    // === Boost.Python wrappers ================================================

#ifdef PYTHON
    
    boost::python::tuple shape_boost();
    boost::python::numeric::array data_boost1( );
    boost::python::numeric::array data_boost2( int32_t length );
    boost::python::numeric::array data_boost3( int64_t offset, int32_t length );

#endif

    // === Private methods ======================================================

  private:

    //! Initialize the object's internal parameters
    void init (std::string const &columnName="");

  };  // dalColumn class
  
  
} // DAL namespace

#endif // DALCOLUMN_H

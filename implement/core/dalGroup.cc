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

#include <core/dalGroup.h>

namespace DAL {
  
  // ============================================================================
  //
  //  Construction
  //
  // ============================================================================
  
  //_____________________________________________________________________________
  //                                                                     dalGroup
  
  dalGroup::dalGroup()
  {
    init ();
  }

  //_____________________________________________________________________________
  //                                                                     dalGroup
  
  /*!
    Create a group in a certain file.
    
    \param groupname The name of the group to create.
    \param file A pointer to the file where you want to create the group.
  */
  dalGroup::dalGroup (const char * gname,
		      void * voidfile)
  {
    init ();

    hid_t * lclfile = (hid_t*)voidfile; // H5File object
    file            = lclfile;
    itsFileID         = *lclfile;  // get the file handle

    groupname_p    = gname;
    groupname_full = "/" + stringify(gname);
    if ( ( itsGroupID = H5Gcreate( itsFileID, groupname_full.c_str(),  H5P_DEFAULT,
                                 H5P_DEFAULT, H5P_DEFAULT ) ) < 0 )
      {
        std::cerr << "ERROR: Could not create group'" << groupname_full
                  << "'.\n";
      }

  }


  //_____________________________________________________________________________
  //                                                                     dalGroup

  /*!
    Create a subgroup in an existing group.
    
    \param group_id The parent group identifier.
    \param gname The name of the subgroup.
  */
  dalGroup::dalGroup (hid_t location_id,
		      const char * gname )
  {
    init();
    
    if ( ( itsGroupID = H5Gcreate( location_id, gname, H5P_DEFAULT, H5P_DEFAULT,
                                 H5P_DEFAULT ) ) < 0 )
      {
        std::cerr << "ERROR: Could not create group'" << std::string( gname )
                  << "'.\n";
      }
  }


  //_____________________________________________________________________________
  //                                                                         open

  /*!
    \param file A pointer to the file.
    \param groupname The name of the group you want to open.
    \return An identifier for the new group.
   */
  int dalGroup::open (void * voidfile,
		      std::string gname)
  {
    hid_t * lclfile = (hid_t*)voidfile; // H5File object
    file            = lclfile;
    itsFileID         = *lclfile;  // get the file handle
    groupname_p     = gname;

    groupname_full = "/" + groupname_p;
    if ( ( itsGroupID = H5Gopen ( itsFileID, groupname_full.c_str(), H5P_DEFAULT ) ) < 0 )
      {
        std::cerr << "ERROR: Could not create group'" << groupname_full
                  << "'.\n";
        return 0;
      }

    return( itsGroupID );
  }

  //_____________________________________________________________________________
  //                                                                        close

  bool dalGroup::close()
  {
    if ( 0 != itsGroupID ) {
      if ( H5Gclose(itsGroupID) < 0 ) {
	std::cerr << "ERROR: dalGroup::close() failed.\n";
	return DAL::FAIL;
      }
      itsGroupID = 0;
    }
    return DAL::SUCCESS;
  }
  
  // ============================================================================
  //
  //  Destruction
  //
  // ============================================================================
  
  dalGroup::~dalGroup()
  {
    if ( 0 != itsGroupID ) {
      if ( H5Gclose(itsGroupID) < 0 ) {
	std::cerr << "ERROR: dalGroup::close() failed.\n";
      }
      itsGroupID = 0;
    }
  }
  
  // ============================================================================
  //
  //  Parameters
  //
  // ============================================================================

  //_____________________________________________________________________________
  //                                                                        getId
  
  /*!
    \return The group identifier as an integer.
  */
  hid_t dalGroup::getId()
  {
    return itsGroupID;
  }
  
  // ============================================================================
  //
  //  Methods
  //
  // ============================================================================
  
  void dalGroup::init () 
  {
    file           = NULL;
    groupname_p    = "UNKNOWN";
    groupname_full = "UNKNOWN";
    group          = NULL;
    filter         = NULL;
    itsFileID        = 0;
    itsGroupID       = 0;
    status         = 0;
  }
  
  //_____________________________________________________________________________
  //                                                                      summary
  
  /*!
    \param os -- Output stream to which the summary will be written.
  */
  void dalGroup::summary(std::ostream &os)
  {
    os << "[dalGroup] Summary of object properties"  << endl;
    
    os << "-- File ID            = " << itsFileID     << std::endl;
    os << "-- Group ID           = " << itsGroupID    << std::endl;
    os << "-- Group name         = " << groupname_p << std::endl;
    os << "-- Status             = " << status      << std::endl;
    
    std::vector<std::string> memberNames = getMemberNames();
    os << "-- nof. group members = " << memberNames.size() << std::endl;
    os << "-- Member names       = [";
    for (unsigned int n(0); n<memberNames.size(); n++)
      {
        os << " " << memberNames[n];
      }
    os << " ]" << std::endl;
    
  }

  // -------------------------------------------------- dalGroup_file_info

  /*
   * Operator function.
   */
  herr_t dalGroup_file_info(hid_t loc_id, const char *name, void *opdata)
  {
    H5G_stat_t statbuf;

    /*
     * Get type of the object and display its name and type.
     * The name of the object is passed to this function by
     * the Library. Some magic :-)
     */
    if ( H5Gget_objinfo(loc_id, name, false, &statbuf) > 0 )
      {
        std::cerr << "ERROR: Could not get group object info.\n";
        return 1;
      }

    switch (statbuf.type)
      {
      case H5G_DATASET:
        (*(std::vector<std::string>*)opdata).push_back( std::string(name) );
        break;
      case H5G_GROUP:
      case H5G_TYPE:
      default:
        break;
      }

    return 0;
  }

  //_____________________________________________________________________________
  //                                                               getMemberNames

  /*!
    \return memberNames -- A vector of strings representing the member names; if
            the group contains no members or is invalid itself, an empty list will
	    be returned.
  */
  std::vector<std::string> dalGroup::getMemberNames ()
  {
    std::vector<std::string> member_names;
    
    if (H5Iis_valid(itsFileID)) {
      H5Giterate (itsFileID,
		  groupname_full.c_str(),
		  NULL,
		  dalGroup_file_info,
		  &member_names);
    }
    
    return member_names;
  }
  
  //_____________________________________________________________________________
  //                                                                      setName
  
  /*!
    Set the name of the group.

    \param gname The name of the group.
    \return bool -- DAL::FAIL or DAL::SUCCESS
   */
  bool dalGroup::setName ( std::string gname )
  {
    if ( gname.length() > 0 ) {
      groupname_p = gname;
      return SUCCESS;
    }
    else {
      std::cerr << "Error:  Group name must not be empty.\n";
      return FAIL;
    }
  }
  
  
  // ------------------------------------------------------- createShortArray
  
  /*!
    Create an array of shorts with any dimensions.  This is usually
    called from the dataset object and not from the developer.
    
    \param arrayname A string containing he name of the array.
    \param dims A vector specifying the array dimensions.
    \param data A structure containing the data to be written.  The size
                of the data must match the provided dimensions.
    \param cdims The chunk dimensions for an extendible array.

    \return dalArray * A pointer to an array object.
  */
  dalArray *
  dalGroup::createShortArray( std::string arrayname,
                              std::vector<int> dims,
                              short data[],
                              std::vector<int> cdims )
  {
    dalShortArray * la;
    la = new dalShortArray( itsGroupID, arrayname, dims, data, cdims );
    return la;
  }



// ------------------------------------------------------------ createIntArray

  /*!
    \brief Create an array of integers within the group.

    Create an array of integers with any dimensions.  This is usually
    called from the dataset object and not from the developer.

    \param arrayname A string containing he name of the array.
    \param dims A vector specifying the array dimensions.
    \param data A structure containing the data to be written.  The size
                of the data must match the provided dimensions.
    \param cdims The chunk dimensions for an extendible array.

    \return dalArray * A pointer to an array object.
  */
  dalArray *
  dalGroup::createIntArray( std::string arrayname,
                            std::vector<int> dims,
                            int data[],
                            std::vector<int> cdims )
  {
    dalIntArray * la;
    la = new dalIntArray( itsGroupID, arrayname, dims, data, cdims );
    return la;
  }


// ------------------------------------------------------- createFloatArray

  /*!
    \brief Create an array of floating point values within the group.

    Create an array of floating point values with any dimensions.  This
    is usually called from the dataset object and not from the developer.

    \param arrayname A string containing he name of the array.
    \param dims A vector specifying the array dimensions.
    \param data A structure containing the data to be written.  The size
                of the data must match the provided dimensions.
    \param cdims The chunk dimensions for an extendible array.

    \return dalArray * A pointer to an array object.
  */
  dalArray *
  dalGroup::createFloatArray( std::string arrayname,
                              std::vector<int> dims,
                              float data[],
                              std::vector<int> cdims )
  {
    dalFloatArray * la;
    la = new dalFloatArray( itsGroupID, arrayname, dims, data, cdims );
    return la;
  }


// ------------------------------------------------ createComplexFloatArray

  /*!
    \brief Create an array of complex floating point values within the group.

    Create an array of complex floating point values with any dimensions.
    This is usually called from the dataset object and not from the
    developer.

    \param arrayname A string containing he name of the array.
    \param dims A vector specifying the array dimensions.
    \param data A structure containing the data to be written.  The size
                of the data must match the provided dimensions.
    \param cdims The chunk dimensions for an extendible array.

    \return dalArray * A pointer to an array object.
  */
  dalArray *
  dalGroup::createComplexFloatArray( std::string arrayname,
                                     std::vector<int> dims,
                                     std::complex<float> data[],
                                     std::vector<int> cdims )
  {
    dalComplexArray_float32 * la;
    la = new dalComplexArray_float32( itsGroupID, arrayname, dims, data, cdims );
    return la;
  }


// ------------------------------------------------- createComplexShortArray

  /*!
    \brief Create an array of complex int16 values within the group.

    Create an array of complex int16 values with any dimensions.
    This is usually called from the dataset object and not from the
    developer.

    \param arrayname A string containing he name of the array.
    \param dims A vector specifying the array dimensions.
    \param data A structure containing the data to be written.  The size
                of the data must match the provided dimensions.
    \param cdims The chunk dimensions for an extendible array.

    \return dalArray * A pointer to an array object.
  */
  dalArray *
  dalGroup::createComplexShortArray( std::string arrayname,
                                     std::vector<int> dims,
                                     std::complex<Int16> data[],
                                     std::vector<int> cdims )
  {
    dalComplexArray_int16 * la;
    la = new dalComplexArray_int16( itsGroupID, arrayname, dims, data, cdims );
    return la;
  }

  // ---------------------------------------------------------- createGroup
  
  /*!
    \brief Create a new group.

    Create a new group.

    \param groupname
    \return dalGroup
  */
  dalGroup * dalGroup::createGroup( const char * gname )
  {
    dalGroup * lg = NULL;
    lg = new dalGroup( itsGroupID, gname );
    return lg;
  }

#ifdef PYTHON
  /************************************************************************
   *
   * The following functions are boost wrappers to allow some previously
   *   defined functions to be easily called from a python prompt.
   *
   ************************************************************************/

  /******************************************************
   * wrappers for createShortArray
   ******************************************************/

// ------------------------------------------------------------ csa_boost_list

  dalArray * dalGroup::csa_boost_list( std::string arrayname,
                                       bpl::list pydims,
                                       bpl::list pydata )
  {
    std::vector<int> dims;
    std::vector<int> chnkdims;

    for (int ii=0; ii<bpl::len(pydims); ii++)
      dims.push_back(bpl::extract<int>(pydims[ii]));

    long size = bpl::len(pydata);
    short * data = NULL;
    data = new short[size];

    for (int ii=0; ii<size; ii++)
      data[ii] = bpl::extract<short>(pydata[ii]);

    dalArray * array = createShortArray(arrayname, dims, data, chnkdims);

    delete [] data;
    data = NULL;

    return array;
  }

  /******************************************************
   * wrappers for createIntArray
   ******************************************************/

// ------------------------------------------------------------ cia_boost_list

  dalArray * dalGroup::cia_boost_list( std::string arrayname,
                                       bpl::list pydims,
                                       bpl::list pydata )
  {
    std::vector<int> dims;
    std::vector<int> chnkdims;

    for (int ii=0; ii<bpl::len(pydims); ii++)
      dims.push_back(bpl::extract<int>(pydims[ii]));

    long size = bpl::len(pydata);
    int * data = NULL;
    data = new int[size];

    for (int ii=0; ii<size; ii++)
      data[ii] = bpl::extract<int>(pydata[ii]);

    dalArray * array = createIntArray(arrayname, dims, data, chnkdims);

    delete [] data;
    data = NULL;

    return array;
  }

// ------------------------------------------------------------ ria_boost

  bpl::numeric::array dalGroup::ria_boost( std::string arrayname )
  {
    hid_t lclfile;
    hid_t  status;

    // get the dataspace
    lclfile = H5Dopen ( itsGroupID, arrayname.c_str(), H5P_DEFAULT );
    hid_t filespace = H5Dget_space(lclfile);

    // what is the rank of the array?
    hid_t data_rank = H5Sget_simple_extent_ndims(filespace);
    hsize_t dims[ data_rank ];
#ifdef DAL_DEBUGGING_MESSAGES
    std::cerr << "data rank: " << data_rank << endl;
#endif
    status = H5Sget_simple_extent_dims(filespace, dims, NULL);

    int size = 1;
    std::vector<int> dimsvec;
    for (int ii=0; ii<data_rank; ii++)
      {
#ifdef DAL_DEBUGGING_MESSAGES
        std::cerr << "dims["  << ii << "]: " << dims[ii] << endl;
#endif
        size *= dims[ii];
        dimsvec.push_back( dims[ii] );
      }

#ifdef DAL_DEBUGGING_MESSAGES
    std::cerr << "size: " << size << endl;
#endif

    int * data = NULL;
    data = new int[size];

    status = H5LTread_dataset_int( itsGroupID, arrayname.c_str(), data );

#ifdef DAL_DEBUGGING_MESSAGES
    for (int ii=0; ii<size; ii++)
      {
        std::cerr << data[ii] << endl;
      }
#endif

    return num_util::makeNum( data, dimsvec );
  }

  /******************************************************
   * wrappers for createFloatArray
   ******************************************************/

// ------------------------------------------------------------ cfa_boost_list

  dalArray * dalGroup::cfa_boost_list( std::string arrayname,
                                       bpl::list pydims,
                                       bpl::list pydata )
  {

    std::vector<int> dims;
    std::vector<int> chnkdims;

    for (int ii=0; ii<bpl::len(pydims); ii++)
      dims.push_back(bpl::extract<int>(pydims[ii]));

    long size = bpl::len(pydata);
    float * data = NULL;
    data = new float[size];

    for (int ii=0; ii<size; ii++)
      data[ii] = bpl::extract<float>(pydata[ii]);

    dalArray * array = createFloatArray(arrayname, dims, data, chnkdims);

    delete [] data;
    data = NULL;

    return array;
  }

  bool dalGroup::setAttribute_char (std::string const &name,
				    char const &data)
  {
    return HDF5Attribute::setAttribute (itsGroupID,
					name,
					&data,
					1);
  }
  bool dalGroup::setAttribute_short (std::string const &name,
				     short const &data)
  {
    return HDF5Attribute::setAttribute (itsGroupID,
					name,
					&data,
					1);
  }
  bool dalGroup::setAttribute_int (std::string const &name,
				   int const &data)
  {
    return HDF5Attribute::setAttribute (itsGroupID,
					name,
					&data,
					1);
  }
  bool dalGroup::setAttribute_uint (std::string const &name,
				    uint const &data)
  {
    return HDF5Attribute::setAttribute (itsGroupID,
					name,
					&data,
					1);
  }
  bool dalGroup::setAttribute_long (std::string const &name,
				    long const &data)
  {
    return HDF5Attribute::setAttribute (itsGroupID,
					name,
					&data,
					1);
  }
  bool dalGroup::setAttribute_float (std::string const &name,
				     float const &data)
  {
    return HDF5Attribute::setAttribute (itsGroupID,
					name,
					&data,
					1);
  }
  bool dalGroup::setAttribute_double (std::string const &name,
				      double const &data)
  {
    return HDF5Attribute::setAttribute (itsGroupID,
					name,
					&data,
					1);
  }
  bool dalGroup::setAttribute_string (std::string const &name,
				      std::string const &data)
  {
    return HDF5Attribute::setAttribute (itsGroupID,
					name,
					&data,
					1);
  }
  bool dalGroup::setAttribute_char_vector (std::string attrname, bpl::list data )
  {
    int size = bpl::len(data);
    std::vector<char> mydata;

    for (int ii=0; ii<bpl::len(data); ii++)
      mydata.push_back(bpl::extract<char>(data[ii]));

    return setAttribute (attrname, reinterpret_cast<char*>(&mydata[0]), size );
  }
  bool dalGroup::setAttribute_short_vector (std::string attrname, bpl::list data )
  {
    int size = bpl::len(data);
    std::vector<short> mydata;

    for (int ii=0; ii<bpl::len(data); ii++)
      mydata.push_back(bpl::extract<short>(data[ii]));

    return setAttribute (attrname, reinterpret_cast<short*>(&mydata[0]), size );
  }
  bool dalGroup::setAttribute_int_vector (std::string attrname, bpl::list data )
  {
    int size = bpl::len(data);
    std::vector<int> mydata;

    for (int ii=0; ii<bpl::len(data); ii++)
      mydata.push_back(bpl::extract<int>(data[ii]));

    return setAttribute (attrname, reinterpret_cast<int*>(&mydata[0]), size );
  }
  bool dalGroup::setAttribute_uint_vector (std::string attrname, bpl::list data )
  {
    int size = bpl::len(data);
    std::vector<uint> mydata;

    for (int ii=0; ii<bpl::len(data); ii++)
      mydata.push_back(bpl::extract<uint>(data[ii]));

    return setAttribute (attrname, reinterpret_cast<uint*>(&mydata[0]), size );
  }
  bool dalGroup::setAttribute_long_vector (std::string attrname, bpl::list data )
  {
    int size = bpl::len(data);
    std::vector<long> mydata;

    for (int ii=0; ii<bpl::len(data); ii++)
      mydata.push_back(bpl::extract<long>(data[ii]));

    return setAttribute (attrname, reinterpret_cast<long*>(&mydata[0]), size );
  }
  bool dalGroup::setAttribute_float_vector (std::string attrname, bpl::list data )
  {
    int size = bpl::len(data);
    std::vector<float> mydata;

    for (int ii=0; ii<bpl::len(data); ii++)
      mydata.push_back(bpl::extract<float>(data[ii]));

    return setAttribute (attrname, reinterpret_cast<float*>(&mydata[0]), size );
  }
  bool dalGroup::setAttribute_double_vector (std::string attrname, bpl::list data )
  {
    int size = bpl::len(data);
    std::vector<double> mydata;

    for (int ii=0; ii<bpl::len(data); ii++)
      mydata.push_back(bpl::extract<double>(data[ii]));

    return setAttribute (attrname, reinterpret_cast<double*>(&mydata[0]), size );
  }
  bool dalGroup::setAttribute_string_vector (std::string attrname, bpl::list data )
  {
    int size = bpl::len(data);
    std::vector<std::string> mydata;

    for (int ii=0; ii<bpl::len(data); ii++)
      mydata.push_back(bpl::extract<std::string>(data[ii]));

    return setAttribute (attrname, reinterpret_cast<std::string*>(&mydata[0]), size );
  }

//  bool setAttribute_char_vector( std::string attrname, bpl::list data );
//  bool setAttribute_short_vector( std::string attrname, bpl::list data );
//  bool setAttribute_int_vector( std::string attrname, bpl::list data );
//  bool setAttribute_uint_vector( std::string attrname, bpl::list data );
//  bool setAttribute_long_vector( std::string attrname, bpl::list data );
//  bool setAttribute_float_vector( std::string attrname, bpl::list data );
//  bool setAttribute_double_vector( std::string attrname, bpl::list data );
//  bool setAttribute_string_vector( std::string attrname, bpl::list data )
//    {
//    std::vector<std::string> mydata;
//
//    for (int ii=0; ii<bpl::len(data); ii++)
//      mydata.push_back(bpl::extract<std::string>(data[ii]));
//
//    return setAttribute_string( attrname, mydata );
//  }

  bpl::numeric::array dalGroup::getAttribute_float_boost ( std::string attrname )
  {
  	 std::vector<float> value;
     h5get_attribute( itsGroupID, attrname.c_str(), value );
     std::cerr << value << std::endl;
     std::vector<int> dims;
     dims.push_back( value.size() );
  	 
//  	 float * values = NULL;


//     std::vector<int> mydims;
//     mydims.push_back( value.size() );
//
//     for (int idx=0; idx++; idx<value.size() )
//        values
//     
//      bpl::numeric::array narray = num_util::makeNum( values, mydims );
//      delete [] values;
//      values = NULL;
//      return narray;


     bpl::numeric::array arr = num_util::makeNum( reinterpret_cast<float*>(&value[0]), dims );
     return arr;
  }
  bpl::numeric::array dalGroup::getAttribute_double_boost ( std::string attrname )
  {
  	 std::vector<double> value;
     h5get_attribute( itsGroupID, attrname.c_str(), value );
     std::cerr << value << std::endl;
     std::vector<int> dims;
     dims.push_back( value.size() );
  	 
     bpl::numeric::array arr = num_util::makeNum( reinterpret_cast<double*>(&value[0]), dims );
     return arr;
  }
  bpl::numeric::array dalGroup::getAttribute_long_boost ( std::string attrname )
  {
  	 std::vector<long> value;
     h5get_attribute( itsGroupID, attrname.c_str(), value );
     std::cerr << value << std::endl;
     std::vector<int> dims;
     dims.push_back( value.size() );
  	 
     bpl::numeric::array arr = num_util::makeNum( reinterpret_cast<long*>(&value[0]), dims );
     return arr;
  }
  bpl::numeric::array dalGroup::getAttribute_short_boost ( std::string attrname )
  {
  	 std::vector<short> value;
     h5get_attribute( itsGroupID, attrname.c_str(), value );
     std::cerr << value << std::endl;
     std::vector<int> dims;
     dims.push_back( value.size() );
  	 
     bpl::numeric::array arr = num_util::makeNum( reinterpret_cast<short*>(&value[0]), dims );
     return arr;
  }
  bpl::numeric::array dalGroup::getAttribute_int_boost ( std::string attrname )
  {
  	 std::vector<int> value;
     h5get_attribute( itsGroupID, attrname.c_str(), value );
     std::cerr << value << std::endl;
     std::vector<int> dims;
     dims.push_back( value.size() );
  	 
     bpl::numeric::array arr = num_util::makeNum( reinterpret_cast<int*>(&value[0]), dims );
     return arr;
  }
  bpl::numeric::array dalGroup::getAttribute_uint_boost ( std::string attrname )
  {
  	 std::vector<uint> value;
     h5get_attribute( itsGroupID, attrname.c_str(), value );
     std::cerr << value << std::endl;
     std::vector<int> dims;
     dims.push_back( value.size() );
  	 
     bpl::numeric::array arr = num_util::makeNum( reinterpret_cast<uint*>(&value[0]), dims );
     return arr;
  }
  bpl::list dalGroup::getAttribute_string_boost ( std::string attrname )
  {
    bpl::list data;
    std::vector<std::string> value;
    h5get_attribute( itsGroupID, attrname.c_str(), value );
    std::cerr << value << std::endl;
    std::vector<int> dims;
    dims.push_back( value.size() );
    
    for ( uint ii=0; ii < value.size() ; ii++ )
      data.append( value[ii].c_str() );
    
    return data;
  }
  
#endif // end #ifdef PYTHON
  
} // end namespace DAL

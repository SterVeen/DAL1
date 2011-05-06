/***************************************************************************
 *   Copyright (C) 2008                                                    *
 *   Joseph Masters <J.S.Masters@uva.nl>                                   *
 *   Lars B"ahren <bahren@astron.nl>                                       *
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

#include "dalCommon.h"

#ifdef DAL_WITH_CASA
using casa::MPosition;
#endif

namespace DAL {
  
  // ============================================================================
  //
  //  Conversion routines
  //
  // ============================================================================

  //_____________________________________________________________________________
  //                                                                    swapbytes
  
  void swapbytes (char *addr,
		  int8_t nbytes)
  {
    char buf;
    int i;
    
    /* byte swap routine - more cryptic - but it is */
    /* generic. Does the swap shown below */

    //buf1[0] = buf2[7];  buf1[1] = buf2[6];
    //buf1[2] = buf2[5];  buf1[3] = buf2[4];
    //buf1[4] = buf2[3];  buf1[5] = buf2[2];
    //buf1[6] = buf2[1];  buf1[7] = buf2[0];

    //  buf3=buf2[0];buf2[0]=buf2[7];buf2[7]=buf3;
    //  buf3=buf2[1];buf2[1]=buf2[6];buf2[6]=buf3;
    //  buf3=buf2[2];buf2[2]=buf2[5];buf2[5]=buf3;
    //  buf3=buf2[3];buf2[3]=buf2[4];buf2[4]=buf3;
    
    for (i=0;i<nbytes/2;i++) {
      buf=addr[i];
      addr[i] = addr[nbytes - (i+1)];
      addr[nbytes - (i+1)] = buf;
    }
  }
  
  //_____________________________________________________________________________
  //                                                                        crc16
  
  /*!
    Generic CRC16 method working on 16-bit unsigned data adapted from Python
    script by Gijs Schoonderbeek.

    \param buffer -- Pointer to the data
    \param length -- Length of the data in 16-bit words.
    
    \return crc -- Value of the CRC
  */
  uint16_t crc16 (uint16_t * buffer,
		  uint32_t length)
  {
    uint16_t CRC            = 0;
    const uint32_t CRC_poly = 0x18005;
    const uint16_t bits     = 16;
    uint32_t data           = 0;
    const uint32_t CRC_div  = (CRC_poly & 0x7fffffff) << 15;
    
    data = (buffer[0] & 0x7fffffff) << 16;
    for (uint32_t i=1; i<length; i++) {
      data += buffer[i];
      for (uint16_t j=0; j<bits; j++) {
	if ((data & 0x80000000) != 0) {
	  data ^= CRC_div;
	}
	data &= 0x7fffffff;
	data <<= 1;
      }
    }
    CRC = data >> 16;
    return CRC;
  }
  
  // ============================================================================
  //
  //  Service routines
  //
  // ============================================================================

  //_____________________________________________________________________________
  //                                                                    BigEndian
  
  bool BigEndian ( void )
  {
    unsigned char SwapTest[2] = { 1, 0 };
    
    if ( *(short *) SwapTest == 1 ) {
      return false;   /* Little endian */
    }
    else {
      return true;    /* Big endian    */
    }
  }
  
  //_____________________________________________________________________________
  //                                                                   h5get_name

  /*!
    \retval name     -- Name of the object (if set or defined).
    \param object_id -- Identifier for the objects of which to retrieve the
           name.
    \retval absolute -- Return the absolute path to the object? If
            <tt>absolute=true</tt> the full path w.r.t. the file root is
	    returned.

    \return status -- Status of the operation; returns <tt>false</tt> in case
            an error was encountered
  */
  bool h5get_name (std::string &name,
                   hid_t const &object_id,
		   bool const &absolute)
  {
    bool status (true);
    H5I_type_t objectType = H5Iget_type (object_id);

    //________________________________________________________________
    // Check: does the object ID point to a valid object type?

    if (objectType == H5I_BADID) {
      return false;
    }
    
    //________________________________________________________________
    // Determine the proper length of the name string

    ssize_t name_length = 0;
    size_t buffer_size  = 10;
    char *buffer        = new char[buffer_size];

    switch (objectType) {
    case H5I_FILE:
      name_length = H5Fget_name (object_id,
				 buffer,
				 buffer_size);
      break;
    default:
      name_length = H5Iget_name (object_id,
				 buffer,
				 buffer_size);
      break;
    };
    
    delete [] buffer;
    
    //________________________________________________________________
    // Retrive the name of the object within the HDF5 file
    
    if (name_length > 0) {
      
      // ... and readjust it to the proper values retrieved above
      buffer_size = name_length+1;
      char *buffer = new char[buffer_size];
      
      // retrieve the name of the object
      switch (objectType) {
      case H5I_FILE:
	name_length = H5Fget_name (object_id,
				   buffer,
				   buffer_size);
	break;
      default:
	name_length = H5Iget_name (object_id,
				   buffer,
				   buffer_size);
	break;
      }
      
      // (optional) Conversion to relative path ____________

      std::string tmp = buffer;
      
      if (absolute) {
	name = tmp;
      } else {
	if (!tmp.compare(0,1,"/")) {
	  tmp.erase(0,1);
	}
	name = tmp;
      }
      
      // Release allocated memory
      delete [] buffer;
      buffer = 0;
      
    }
    else {
      std::cerr << "[h5get_name] Object name of zero characters!" << endl;
      status = false;
    }
    
    return true;
  }
  
  //_____________________________________________________________________________
  //                                                                   h5get_name
  
  /*!
    \retval name     -- Name of the object (if set or defined).
    \param object_id -- Identifier for the objects of which to retrieve the
           name.
    \param index     -- Transient index identifying the object whose name to
           retrieve.
    
    \return status -- Status of the operation; returns <tt>false</tt> in case
    an error was encountered
  */
  bool h5get_name (std::string &name,
                   hid_t const &object_id,
                   hsize_t const &index)
  {
    hsize_t nofObjects = 0;
    herr_t h5error     = 0;

    // Check if the transient index is within the valid range

    h5error = H5Gget_num_objs(object_id,
                              &nofObjects);

    if (index > nofObjects) {
      std::cerr << "[h5get_name] Running index outside valid range!"
		<< endl;
      return false;
    }
    
    /*
      Get the name of the object identified by the transient index; first
      function call is to retrieve retrieve the number of characters in the
      object's name.
    */

    ssize_t name_length = 0;
    size_t buffer_size  = 10;

    // first function call to get the number of characters in the object's name
    try {
      char *buffer = new char[buffer_size];
      name_length = H5Gget_objname_by_idx (object_id,
					   index,
					   buffer,
					   buffer_size);
      delete [] buffer;
    }
    catch (std::string message) {
      std::cerr << "[h5get_name] Error calling H5Gget_objname_by_idx; "
		<< message
		<< endl;
      return false;
    }
    
    if (name_length > 0) {
      buffer_size  = name_length+1;
      char *buffer = new char[buffer_size];
      // second function call to retrieve the object's name
      name_length = H5Gget_objname_by_idx (object_id,
					   index,
					   buffer,
					   buffer_size);
      std::string tmp = buffer;
      name = tmp;
      delete [] buffer;
    }
    else {
      std::cerr << "[h5get_name] Object name of zero characters!" << endl;
      return false;
    }
    
    return true;
  }
  
  //_____________________________________________________________________________
  //                                                               h5get_filename
  
  /*!
    \retval filename -- Name of the HDF5 file within which the object is
            contained
    \param object_id -- Identifier for the objects of which to retrieve the
           name.

    \return status -- Status of the operation; returns <tt>false</tt> in case
            an error was encountered
  */
  bool h5get_filename (std::string &filename,
                       hid_t const &object_id)
  {
    bool status (true);
    H5I_type_t objectType;

    //________________________________________________________________
    // Basic check for the provided object ID
    
    if (H5Iis_valid(object_id)) {
      objectType = H5Iget_type(object_id);
      
      if (objectType == H5I_FILE) {
	status = h5get_name (filename,
			     object_id);
      }
      else {
	hid_t file_id = H5Iget_file_id (object_id);
	status = h5get_name (filename,
			     file_id);
	H5Fclose (file_id);
      }
    }
    else {
      status = false;
    }
    
    return status;
  }

  //_____________________________________________________________________________
  //                                                                  h5get_names

  /*!
    \retval names -- Vector with the names of the groups/datasets attached to the
            object identified by \e location_id.
    \param location_id -- Identifier of the object for which to retrieve the list
           of names.
    \param type -- The type field contains the type of the object, one of
           H5G_GROUP, H5G_DATASET, H5G_LINK, or H5G_TYPE. By default this
	   function will look for groups.

    \return status -- Status of the operation; returns \e false in case an error
            was encountered.
  */
  bool h5get_names (std::vector<std::string> &names,
		    hid_t const &location_id,
		    int const &type)
  {
    /* Forwarding of the function call */
    std::set<std::string> namesSet;
    bool status = h5get_names (namesSet,
			       location_id,
			       type);
    /* Adjust the size of the vector to return the data */
    names.resize(namesSet.size());
    /* Set up iterators to access the underlying data */
    std::set<std::string>::iterator itSet=namesSet.begin();
    std::vector<std::string>::iterator itVect=names.begin();
    
    for (; itSet!=namesSet.end(); ++itSet) {
      *itVect = *itSet;
      ++itVect;
    }
    
    return status;
  }
  
  //_____________________________________________________________________________
  //                                                                  h5get_names
  
  /*!
    <i>Note:</i> This function might need to be updated at some point, as the 
    internally used HDF5 function <tt>H5Gget_num_objs()</tt> is considered
    depricated.

    \retval names -- Set with the names of the groups/datasets attached to the
            object identified by \e location_id. If no objects were found, an 
	    empty list is returned.
    \param location_id -- Identifier of the object for which to retrieve the list
           of names.
    \param type -- Type of attached objects for which to get the list of names;
           can be either \e H5G_GROUP or \e H5G_DATASET. By default this function
	   will look for groups.

    \return status -- Status of the operation; returns \e false in case an error
            was encountered.
  */
  bool h5get_names (std::set<std::string> &names,
		    hid_t const &locationID,
		    int const &type)
  {
    bool status           = true;
    hsize_t nofObjects    = 0;
    herr_t h5error        = 0;

    /* Get the number of objects attached to this object */
    h5error = H5Gget_num_objs(locationID,
                              &nofObjects);
    
    if (h5error > 0) {
      std::cerr << "[h5get_groupnames] Error retrieving number of attached groups!"
		<< std::endl;
      return false;
    }
    
    if (nofObjects>0) {
      std::string tmp;
      
      for (hsize_t idx (0); idx<nofObjects; idx++) {
	/* Get the type of the attached object */
	if (type == H5Gget_objtype_by_idx (locationID,idx)) {
	  /* If the attached object is a group, retrieve its name */
	  status = DAL::h5get_name (tmp,
				    locationID,
				    idx);
	  /* If retrieval of the name was successful, add it to the list */
	  if (status) {
	    names.insert(tmp);
	  }
	}
      }
    } else {
      status = false;
    }
    
    return status;
  }

  // ============================================================================
  //
  //  Get HDF5 attributes
  //
  // ============================================================================
  
  //_____________________________________________________________________________
  //                                                          h5attribute_summary
  
  /*!
    \param location_id -- Identifier of the structure within the file, to which
           the attribut is attached to.
    \param name        -- Name of the attribute, as which it is attached to
           another structure within the file
    \param os          -- Output stream to which the summary is written
  */
  void h5attribute_summary (hid_t const &location_id,
			    std::string const &name,
			    std::ostream &os)
  {
    if (H5Aexists(location_id, name.c_str())) {
      // Open the attribute
      hid_t attribute_id = H5Aopen (location_id,
				    name.c_str(),
				    H5P_DEFAULT);
      // Summary of the attribute
      h5attribute_summary (attribute_id,
			   os);
      // Close the attribute
      if (H5Iis_valid(attribute_id)) {
	H5Aclose (attribute_id);
      }
    }
  }
  
  //_____________________________________________________________________________
  //                                                          h5attribute_summary
  
  /*!
    \param os           -- Output stream to which the summary is written
    \param attribute_id -- HDF5 identifier pointing at the attribute to show
  */
  void h5attribute_summary (hid_t const &attribute_id,
			    std::ostream &os)
  {
    if (H5Iget_type (attribute_id) == H5I_ATTR) {

      bool status (true);
      
      /*
       * Datatype of the attribute
       */
      hid_t datatype_id        = H5Aget_type (attribute_id);
      hsize_t datatype_size    = H5Tget_size (datatype_id);
      bool datatype_is_integer = H5Tdetect_class (datatype_id,H5T_INTEGER);
      bool datatype_is_float   = H5Tdetect_class (datatype_id,H5T_FLOAT);
      bool datatype_is_string  = H5Tdetect_class (datatype_id,H5T_STRING);

      /*
       * Dataspace of the attribute
       */
      int rank (0);
      hid_t dataspace_id       = H5Aget_space (attribute_id);
      bool dataspace_is_simple = H5Sis_simple(dataspace_id);
      
      try {
	rank = H5Sget_simple_extent_ndims (dataspace_id);
      }
      catch (std::string message) {
	cerr << "[h5attribute_summary] " << message << endl;
	status = false;
      }

      // Display properties
      os << "\t-- Datatype ID             = " << datatype_id         << endl;
      os << "\t-- Datatype size [Bytes]   = " << datatype_size       << endl;
      os << "\t-- Datatype is H5T_INTEGER = " << datatype_is_integer << endl;
      os << "\t-- Datatype is H5T_FLOAT   = " << datatype_is_float   << endl;
      os << "\t-- Datatype is H5T_STRING  = " << datatype_is_string  << endl;
      os << "\t-- Dataspace ID            = " << dataspace_id        << endl;
      os << "\t-- Dataspace is simple?    = " << dataspace_is_simple << endl;
      os << "\t-- Rank of the data array  = " << rank                << endl;

      // close HDF5 objects
      if (H5Iis_valid(datatype_id))  { H5Tclose (datatype_id);  }
      if (H5Iis_valid(dataspace_id)) { H5Sclose (dataspace_id); }

    }  // end -- (H5Iget_type () == H5I_ATTR)
    else {
      os << "[h5attribute_summary] Provided HDF5 object is not an attribute!" 
	 << std::endl;
    }
    
  }
  
  //_____________________________________________________________________________
  //                                                                    attr_info
  
  /*!
    \param loc_id -- Identifier for the HDF5 object - file, group, dataset,
           array - the attribute is attached to.
    \param name   -- Name of the attribute
    \param opdata -- not used, but necessary as a placeholder
    \return error status
  */
  herr_t attr_info (hid_t loc_id,
                    const char * name,
                    void * opdata)
  {
    /*  Open the attribute using its name.  */
    hid_t attr = H5Aopen (loc_id,
                          name,
                          H5P_DEFAULT);

    /* Get attribute datatype, dataspace, rank, and dimensions.  */
    hid_t atype  = H5Aget_type(attr);
    hid_t aspace = H5Aget_space(attr);

    hsize_t * sdim = new hsize_t[ 64 ];
    herr_t ret     = H5Sget_simple_extent_dims(aspace, sdim, 0);
    // release allocated memory
    delete [] sdim;
    sdim = 0;

    opdata = opdata;  // avoid compiler warnings of unused parameter

    if (H5T_INTEGER == H5Tget_class(atype)) {
      int point_out = 0;
      ret  = H5Aread(attr, H5T_NATIVE_INT, &point_out);
      cout << name << " = " << point_out << endl;
    }
    else if (H5T_FLOAT == H5Tget_class(atype)) {
      size_t npoints = H5Sget_simple_extent_npoints(aspace);
      float * float_array = new float[ npoints ];
      if ( H5Aread( attr, H5T_NATIVE_FLOAT, float_array ) < 0 )
	return 1;
      
      std::cerr << name << " = ";
      for (int i = 0; i < (int)npoints; i++) printf("%f ", float_array[i]);
      printf("\n");
      delete [] float_array;
      float_array = 0;
    }
    else if (H5T_STRING == H5Tget_class (atype)) {
      char * string_attr;
      hid_t type = H5Tget_native_type( atype, H5T_DIR_ASCEND);
      if ( H5Aread( attr, type, &string_attr) < 0 )
	return 1;
      
      fprintf(stderr, "%s = %s\n", name, string_attr );
      free(string_attr);
    }
    
    // close opened HDF5 objects
    if (H5Iis_valid(atype))  ret = H5Tclose(atype);
    if (H5Iis_valid(aspace)) ret = H5Sclose(aspace);
    if (H5Iis_valid(attr))   ret = H5Aclose(attr);
    
    return 0;
  }
  
  //_____________________________________________________________________________
  //                                                        h5setAttribute_string
  
  /*!
    \brief Add a string attribute.
    
    \param obj_id The hdf5 object identifier.
    \param attrname The name of the attribute you want to add.
    \param data The value of the attribute you want to add.
    \param size The dimension of the attribute.
    \return DAL::FAIL or DAL::SUCCESS
   */
  bool h5setAttribute_string (hid_t const &obj_id,
                              std::string attrname,
                              std::string const * data,
                              int size )
  {
    hid_t att       = 0;
    hid_t dataspace = 0;
    hsize_t dims[1] = { size };

    char ** string_attr = (char**)malloc( size * sizeof(char*) );
    for ( int ii = 0; ii < size; ii++ ) {
      string_attr[ii] = (char*)malloc(MAX_COL_NAME_SIZE * sizeof(char));
      strcpy( string_attr[ii], data[ii].c_str() );
    }
    
    hid_t type = H5Tcopy (H5T_C_S1);
    if ( type < 0 ) {
      std::cerr << "ERROR: Could not set attribute '" << attrname
		<< "' type.\n";
      return DAL::FAIL;
    }
    
    if ( H5Tset_size(type, H5T_VARIABLE) < 0 ) {
      std::cerr << "ERROR: Could not set attribute '" << attrname
		<< "' size.\n";
      return DAL::FAIL;
    }
    
    dataspace = H5Screate_simple(1, dims, 0);
    if ( dataspace < 0 ) {
      std::cerr << "ERROR: Could not set attribute '" << attrname
		<< "' dataspace.\n";
      return DAL::FAIL;
    }
    
    att = H5Acreate( obj_id, attrname.c_str(), type, dataspace, 0, 0 );
    if ( att < 0 ) {
      std::cerr << "ERROR: Could not create attribute '" << attrname << "'.\n";
      return DAL::FAIL;
    }
    
    if ( H5Awrite( att, type, string_attr ) < 0 ) {
      std::cerr << "ERROR: Could not write attribute '" << attrname << "'.\n";
      return DAL::FAIL;
    }
    
    for ( int ii = 0; ii < size; ii++ ) {
      free( string_attr[ii] );
    }
    free( string_attr );

    /* release HDF5 handlers */
    if (H5Iis_valid(att))       H5Aclose (att);
    if (H5Iis_valid(dataspace)) H5Sclose (dataspace);
    if (H5Iis_valid(type))      H5Tclose (type);
    
    return DAL::SUCCESS;
  }
  
  //_____________________________________________________________________________
  // Get the value of an attribute attached to a group or dataset
  
  /*!
    \param attribute_id -- Identifier of the attribute within the HDF5 file
    \retval value       -- Value of the attribute

    \return status -- Status of the operation; returns <tt>false</tt> in case
            an error was encountered
  */
  template <>
  bool h5get_attribute (hid_t const &attribute_id,
                        std::string &value)
  {
    bool status (true);
    herr_t h5error (0);
    hid_t datatype_id (0);
    hid_t native_datatype_id (0);

    if (H5Iis_valid(attribute_id)) {
      datatype_id        = H5Aget_type (attribute_id);
      native_datatype_id = H5Tget_native_type(datatype_id, H5T_DIR_ASCEND);
    } else {
      return false;
    }

    if (datatype_id > 0) {
      
      H5T_class_t datatype_class_id = H5Tget_class (datatype_id);
      
      if (datatype_class_id == H5T_STRING) {
	htri_t is_variable_string = H5Tis_variable_str(datatype_id);
	
	// Buffer to temporarily hold string while reading
	char *buffer;
	
	// additional variables
	hsize_t datatype_size         = H5Tget_size (datatype_id);
	
#ifdef DAL_DEBUGGING_MESSAGES
	// output
	std::cout << "[DAL::h5get_attribute]" << endl;
	std::cout << "-- Attribute ID       = " << attribute_id  << endl;
	std::cout << "-- Datatype ID        = " << datatype_id   << endl;
	std::cout << "-- Datatype size      = " << datatype_size << endl;
	std::cout << "-- Is variable length = " << is_variable_string << endl;
#endif
	
	if (is_variable_string) {
	  // read the contents of the attribute into the buffer
	  h5error = H5Aread(attribute_id,
			    native_datatype_id,
			    &buffer);
	  // copy retrieved value to return variable
	  if (h5error == 0) {
	    std::string tmp = buffer;
	    value = tmp;
	  }
	}
	else {
    // Allocate memory for buffer
    buffer = new char[datatype_size];
	
	  h5error = H5Aread(attribute_id,
			    datatype_id,
			    buffer);

	  std::string tmp(buffer, datatype_size);
    value = tmp;

    // Free memory buffer
    delete buffer;
	}

	// // release allocated memory
	// if (data!=NULL) delete data;
	
      } // END -- (datatype_class_id == H5T_STRING)
      
    }
    
    /* release HDF5 handlers */
    if (H5Iis_valid(native_datatype_id)) { H5Tclose (native_datatype_id); }
    if (H5Iis_valid(datatype_id))        { H5Tclose (datatype_id);        }
    // clean up the error message buffer
    h5error = H5Eclear1();

    return status;
  }
  
  //_____________________________________________________________________________
  //                                                              h5get_attribute
  
  /*!
    As HDF5 does not have a datatype <tt>bool</tt>, we are using <tt>int</tt> 
    for storage instead; as a consequence of this an explicit implentation is
    required to deal with the backwards conversion when reading from a file.
  */
  template <>
  bool h5get_attribute (hid_t const &attribute_id,
			std::vector<bool> &value)
  {
    bool status              = true;
    herr_t h5error           = 0;
    hid_t datatype_id        = H5Aget_type (attribute_id);
    hid_t native_datatype_id = H5Tget_native_type(datatype_id, H5T_DIR_ASCEND);
    std::vector<hsize_t> shape;
    
    status = HDF5Dataspace::shape (attribute_id,shape);
    
    if (shape.size() > 0) {
      // Buffer for the underlying HDF5 library call
      int *buffer = new int [shape[0]];
      // read attribute value into buffer
      h5error = H5Aread (attribute_id,
			 native_datatype_id,
			 buffer);
      // copy retrieved data to returned vector
      if (h5error == 0) {
	// adjust size of vector returning the result
	value.resize(shape[0]);
	// copy the contents of the buffer
	for (uint n(0); n<shape[0]; n++) {
	  value[n] = bool(buffer[n]);
	}
      }
      else {
	cerr << "[h5get_attribute] Error reading value of attribute." << endl;
	status = false;
      }
      // release memory allocated for temporary buffer
      delete [] buffer;
      buffer = 0;
    }
    else {
      cerr << "[h5get_attribute] Unsupported shape of attribute dataspace!"
	   << endl;
      status = false;
    }

    /* release HDF5 handlers */
    if (H5Iis_valid(datatype_id))        H5Tclose (datatype_id);
    if (H5Iis_valid(native_datatype_id)) H5Tclose (native_datatype_id);
    
    return status;
  }
  
  //_____________________________________________________________________________
  //                                                              h5get_attribute
  
  /*!
    \param attribute_id -- Identifier of the attribute within the HDF5 file
    \retval value       -- Value of the attribute
    
    \return status -- Status of the operation; returns <tt>false</tt> in case
            an error was encountered
  */
  template <>
  bool h5get_attribute (hid_t const &attribute_id,
                        std::vector<std::string> &value)
  {
    bool status (true);
    herr_t h5error (0);
    hid_t datatype_id;
    hid_t native_datatype_id;
    std::vector<hsize_t> shape;
    char *buffer;
    
    if (H5Iis_valid(attribute_id)) {
      datatype_id        = H5Aget_type (attribute_id);
      native_datatype_id = H5Tget_native_type(datatype_id, H5T_DIR_ASCEND);
      status             = HDF5Dataspace::shape (attribute_id,shape);
    } else {
      return false;
    }
    
    if (shape.size() > 0) {
	    // additional variables
	    hsize_t datatype_size         = H5Tget_size (datatype_id);
      buffer = new char[shape[0]*datatype_size];

      // Read the attribute data from the file
      h5error = H5Aread(attribute_id,
			native_datatype_id,
			buffer);

      // Copy the retrieved data to the returned variable
      if (h5error == 0) {
	value.resize(shape[0]);
  std::string str = std::string(buffer);
	for (uint n(0); n<shape[0]; n++) {
	  value[n] = str.substr(n*datatype_size, datatype_size);
	}
      }
    }
    else {
      cerr << "[h5get_attribute] Unsupported shape of attribute dataspace!"
	   << endl;
      status = false;
    }
    
    /* release HDF5 handlers */
    if (H5Iis_valid(datatype_id)) H5Tclose (datatype_id);
    if (H5Iis_valid(native_datatype_id)) H5Tclose (native_datatype_id);
    
    return status;
  }
  
  // ============================================================================
  //
  //  Set HDF5 attributes
  //
  // ============================================================================

  //_____________________________________________________________________________
  // Set the value of an attribute attached to a group or dataset

  /*!
    \param datatype    -- HDF5 datatype of the attribute
    \param location_id -- HDF5 identifier of the attribute within the file
    \param name        -- Name of the attribute
    \param value       -- Value of the attribute
    \param size        -- The size of the attribute
  */
  template <>
  bool h5set_attribute (hid_t const &datatype,
                        hid_t const &location_id,
                        std::string name,
                        std::string * value,
                        int size)
  {
    hid_t   attribute_id = 0;  /* Attribute identifier */
    hid_t   dataspace_id = 0;  /* Attribute dataspace identifier */
    hsize_t dims[1]      = { size };

    /* Check object identifier and data type */

    if (H5Iis_valid(location_id)) {
      if (datatype != H5T_STRING) {
	std::cerr << "[dalCommon::h5set_attribute] Wrong input datatype!"
		  << std::endl;
	return false;
      }
    } else {
      std::cerr << "[dalCommon::h5set_attribute]"
		<< " Unable to set attribute - invalid object identifier!"
		<< std::endl;
      return false;
    }

    char ** string_attr = (char**)malloc( size * sizeof(char*) );
    for ( int ii = 0; ii < size; ii++ ) {
      string_attr[ii] = (char*)malloc(MAX_COL_NAME_SIZE * sizeof(char));
      strcpy( string_attr[ii], value[ii].c_str() );
    }
    
    hid_t type = H5Tcopy (H5T_C_S1);
    if ( type < 0 ) {
      std::cerr << "ERROR: Could not set attribute '" << name << "' type."
		<< std::endl;
      return false;
    }
    
    if ( H5Tset_size(type, H5T_VARIABLE) < 0 ) {
      std::cerr << "ERROR: Could not set attribute '" << name
		<< "' size." << std::endl;
      return false;
    }

    if (H5Aexists(location_id,name.c_str())) {
      attribute_id = H5Aopen (location_id,
			      name.c_str(),
			      H5P_DEFAULT);
    }
    else {
      dataspace_id = H5Screate_simple(1, dims, 0);
      if ( dataspace_id < 0 ) {
	std::cerr << "[h5set_attribute] Failed to create dataspace for attribute "
		  << name
		  << std::endl;
	return false;
      }
      
      attribute_id = H5Acreate( location_id,
				name.c_str(),
				type,
				dataspace_id,
				0, 0 );
      if ( attribute_id < 0 ) {
	std::cerr << "ERROR: Could not create attribute '" << name << "'.\n";
	return false;
      }
    }
    
    if ( H5Awrite( attribute_id, type, string_attr ) < 0 ) {
      std::cerr << "ERROR: Could not write attribute '" << name << "'.\n";
      return false;
    }
    
    if (  H5Aclose( attribute_id ) < 0 ) {
      std::cerr << "ERROR: Could not close attribute '" << name << "'.\n";
      return false;
    }
    
    for ( int ii = 0; ii < size; ii++ ) {
      free( string_attr[ii] );
    }
    free( string_attr );

    /* release HDF5 handlers */
    if (H5Iis_valid(dataspace_id)) H5Sclose (dataspace_id);
    if (H5Iis_valid(attribute_id)) H5Aclose (attribute_id);
    
    return true;
  }
  
  //_____________________________________________________________________________
  // Set the value of an attribute attached to a group or dataset

  /// @cond TEMPLATE_SPECIALIZATIONS

  /*!
    As HDF5 does not have a datatype <tt>boolean</tt>, we to cast such variables
    to a different type; we are using <tt>integer</tt> to store boolean
    variables.

    \param location_id -- HDF5 identifier of the attribute within the file
    \param name        -- Name of the attribute
    \param value       -- Value of the attribute
    \param size        -- The size of the attribute
  */
  template <>
  bool h5set_attribute (hid_t const &location_id,
                        std::string name,
                        bool * value,
                        int size)
  {
    bool status;
    int * data  = new int [size];
    
    for (int n(0); n<size; n++) {
      data[n] = int(value[n]);
    }
    /* Forward the function call */
    hid_t datatype = H5T_NATIVE_INT;
    status = h5set_attribute (datatype,
			      location_id,
			      name,
			      data,
			      size);
    // release allocated memory ...
    delete [] data;
    // ... before exiting function
    return status;
  }

  /*!
    \param location_id -- HDF5 identifier of the attribute within the file
    \param name        -- Name of the attribute
    \param value       -- Value of the attribute
    \param size        -- The size of the attribute
  */
  template <>
  bool h5set_attribute (hid_t const &location_id,
                        std::string name,
                        int * value,
                        int size)
  {
    hid_t datatype = H5T_NATIVE_INT;
    return h5set_attribute (datatype,
                            location_id,
                            name,
                            value,
                            size);
  }

  /*!
    \param location_id -- HDF5 identifier of the attribute within the file
    \param name        -- Name of the attribute
    \param value       -- Value of the attribute
    \param size        -- The size of the attribute
  */
  template <>
  bool h5set_attribute (hid_t const &location_id,
                        std::string name,
                        uint * value,
                        int size)
  {
    hid_t datatype = H5T_NATIVE_UINT;
    return h5set_attribute (datatype,
                            location_id,
                            name,
                            value,
                            size);
  }

  /*!
    \param location_id -- HDF5 identifier of the attribute within the file
    \param name        -- Name of the attribute
    \param value       -- Value of the attribute
    \param size        -- The size of the attribute
  */
  template <>
  bool h5set_attribute (hid_t const &location_id,
                        std::string name,
                        short * value,
                        int size)
  {
    hid_t datatype = H5T_NATIVE_SHORT;
    return h5set_attribute (datatype,
                            location_id,
                            name,
                            value,
                            size);
  }

  /*!
    \param location_id -- HDF5 identifier of the attribute within the file
    \param name        -- Name of the attribute
    \param value       -- Value of the attribute
    \param size        -- The size of the attribute
  */
  template <>
  bool h5set_attribute (hid_t const &location_id,
                        std::string name,
                        long * value,
                        int size)
  {
    hid_t datatype = H5T_NATIVE_LONG;
    return h5set_attribute (datatype,
                            location_id,
                            name,
                            value,
                            size);
  }

  /*!
    \param location_id -- HDF5 identifier of the attribute within the file
    \param name        -- Name of the attribute
    \param value       -- Value of the attribute
    \param size        -- The size of the attribute
  */
  template <>
  bool h5set_attribute (hid_t const &location_id,
                        std::string name,
                        unsigned long long * value,
                        int size)
  {
    hid_t datatype = H5T_NATIVE_ULLONG;
    return h5set_attribute (datatype,
                            location_id,
                            name,
                            value,
                            size);
  }

  /*!
    \param location_id -- HDF5 identifier of the attribute within the file
    \param name        -- Name of the attribute
    \param value       -- Value of the attribute
    \param size        -- The size of the attribute
  */
  template <>
  bool h5set_attribute (hid_t const &location_id,
                        std::string name,
                        float * value,
                        int size)
  {
    hid_t datatype = H5T_NATIVE_FLOAT;
    return h5set_attribute (datatype,
                            location_id,
                            name,
                            value,
                            size);
  }

  /*!
    \param location_id -- HDF5 identifier of the attribute within the file
    \param name        -- Name of the attribute
    \param value       -- Value of the attribute
    \param size        -- The size of the attribute
  */
  template <>
  bool h5set_attribute (hid_t const &location_id,
                        std::string name,
                        double * value,
                        int size)
  /*!
    \param location_id -- HDF5 identifier of the attribute within the file
    \param name        -- Name of the attribute
    \param value       -- Value of the attribute
    \param size        -- The size of the attribute
  */
  {
    hid_t datatype = H5T_NATIVE_DOUBLE;
    return h5set_attribute (datatype,
                            location_id,
                            name,
                            value,
                            size);
  }

  /*!
    \param location_id -- HDF5 identifier of the attribute within the file
    \param name        -- Name of the attribute
    \param value       -- Value of the attribute
    \param size        -- The size of the attribute
  */
  template <>
  bool h5set_attribute (hid_t const &location_id,
                        std::string name,
                        std::string * value,
                        int size)
  {
    hid_t datatype = H5T_STRING;
    return h5set_attribute (datatype,
                            location_id,
                            name,
                            value,
                            size);
  }

  /// @endcond

  // ============================================================================
  //
  // Access to HDF5 attributes through casacore (array) classes
  //
  // ============================================================================

#ifdef DAL_WITH_CASA

  //_____________________________________________________________________________
  // Get the shape of a dataset
  
  /*!
    \param dataset_id -- Identifier of the dataset within the HDF5 file
    \reval shape -- The shape of the dataset i.e.the length of the array axes in
           case of multidimensional data
    \param maxdims -- Return maximum dimensions instead of present dimensions?

    \return status -- Status of the operation; returns <tt>false</tt> in case
            an error was encountered
  */
  bool h5get_dataset_shape (hid_t const &attribute_id,
                            casa::IPosition &shape,
                            bool const &maxdims)
  {
    bool status (true);
    herr_t h5error (0);
    hid_t dataset_id = H5Dget_space (attribute_id);
    int rank         = H5Sget_simple_extent_ndims (dataset_id);

    if (rank > 0) {
      shape.resize(rank);
      hsize_t * dataset_dims    = new hsize_t[rank];
      hsize_t * dataset_maxdims = new hsize_t[rank];
      // get dimensions of simple dataspace
      h5error = H5Sget_simple_extent_dims(dataset_id,
					  dataset_dims,
					  dataset_maxdims);
      // copy dataset information to return value
      if (maxdims) {
	for (int n(0); n<rank; n++) {
	  shape(n) = dataset_maxdims[n];
	}
      }
      else {
	for (int n(0); n<rank; n++) {
	  shape(n) = dataset_dims[n];
	}
      }
      
      // release allocated memory
      delete [] dataset_dims;
      delete [] dataset_maxdims;
    }
    else {
      shape.resize(1);
      shape(0) = 0;
      status = false;
    }
    
    // release allocated identifiers
    if (dataset_id > 0) {
      h5error = H5Sclose (dataset_id);
      h5error = H5Eclear1 ();
    }
    
    return status;
  }

  //_____________________________________________________________________________
  //                                                              h5get_attribute

  template <>
  bool h5get_attribute (hid_t const &attribute_id,
                        casa::Vector<casa::String> &value)
  {
    bool status              = true;
    herr_t h5error           = 0;
    hid_t datatype_id        = H5Aget_type (attribute_id);
    hid_t native_datatype_id = H5Tget_native_type(datatype_id, H5T_DIR_ASCEND);
    std::vector<hsize_t> shape;

    status = HDF5Dataspace::shape (attribute_id,shape);
    
    if (shape.size() > 0) {
      char * buffer[shape[0]];
      // Read the attribute data from the file
      h5error = H5Aread(attribute_id,
			native_datatype_id,
			&buffer);
      // Copy the retrieved data to the returned variable
      if (h5error == 0)
	{
	  value.resize(shape[0]);
	  for (uint n(0); n<shape[0]; n++)
	    {
	      value(n) = buffer[n];
	    }
	}
    }
    else {
      cerr << "[h5get_attribute] Unsupported shape of attribute dataspace!"
	   << endl;
      status = false;
    }
    
    /* release HDF5 handlers */
    H5Tclose (datatype_id);
    H5Tclose (native_datatype_id);
    
    return status;
  }
  
  // ------------------------------------------------------------- h5get_quantity
  
  /*!
    \param location_id -- Identifier of the structure within the file, to which
           the attribut is attached to.
    \param value -- Identifier for the attribute storing the numerical value of
           the quantity.
    \param unit  -- Identifier for the attribute storing the physical unit of
           the quantity

    \return quantity -- The physical quantity.
  */
  casa::Quantity h5get_quantity (hid_t const &location_id,
				 Attributes const &value,
				 Attributes const &unit)
  {
    if (location_id > 0)
      {
        bool status (true);
        double qValue;
        std::string qUnit;
        // retrieve the value of the quantity
        status *= h5get_attribute(location_id,
                                  attribute_name(value),
                                  qValue);
        // retrieve the unit of the quantity
        status *= h5get_attribute(location_id,
                                  attribute_name(unit),
                                  qUnit);
        // put together the Quantity object
        if (status) {
	  casa::Quantity val = casa::Quantity (qValue,
					       casa::Unit(qUnit));
	  return val;
	}
        else {
	  return casa::Quantity();
	}
      }
    else {
      cerr << "[h5get_quantity] Unusable ID for HDF5 object!" << endl;
      return casa::Quantity();
    }
  }
  
  // ------------------------------------------------------------ h5get_direction
  
  /*!
    \param location_id -- Identifier of the structure within the file, to which
           the attribut is attached to.
    \param value -- Identifier for the attribute storing the numerical value of
           the quantity.
    \param unit  -- Identifier for the attribute storing the physical unit of
           the quantity
    \param frame -- Identifier for the attribute storing the identifier for the
           reference frame within which the physical quantity is defined.

    \return direction -- The physical quantity.
  */
  casa::MDirection h5get_direction (hid_t const &location_id,
				    Attributes const &value,
				    Attributes const &unit,
				    Attributes const &frame)
  {
    return h5get_direction (location_id,
                            attribute_name(value),
                            attribute_name(unit),
                            attribute_name(frame));
  }

  /*!
    \param location_id -- Identifier of the structure within the file, to which
           the attribut is attached to.
    \param value -- Identifier for the attribute storing the numerical value of
           the quantity.
    \param unit  -- Identifier for the attribute storing the physical unit of
           the quantity
    \param frame -- Identifier for the attribute storing the identifier for the
           reference frame within which the physical quantity is defined.

    \return direction -- The physical quantity.
  */
  casa::MDirection h5get_direction (hid_t const &location_id,
				    std::string const &value,
				    std::string const &unit,
				    std::string const &frame)
  {
    casa::MDirection dir = casa::MDirection();
    
    if (location_id > 0) {
      casa::Vector<double> values;
      casa::Vector<casa::String> units;
      std::string refcode;
      casa::MDirection::Types tp;
      
      //______________________________________________________________
      // Retrieve components from dataset to construct MDirection
      
      if (!h5get_attribute(location_id,
			   value,
			   values))
	{
	  cerr << "[h5get_direction]"
	       << " Error retrieving the numerical values of the position"
	       << endl;
	  return dir;
	}
      
      if (!h5get_attribute(location_id,
			   unit,
			   units))
	{
	  cerr << "[h5get_direction]"
	       << " Error retrieving the physical units of the position"
	       << endl;
	  return dir;
	}
      
      if (!h5get_attribute(location_id,
			   frame,
			   refcode))
	{
	  cerr << "[h5get_direction]"
	       << " Error retrieving the frame of the position"
	       << endl;
	  return dir;
	}
      
      //______________________________________________________________
      // Combine components into MDirection
      
      if (casa::MDirection::getType (tp,refcode)) {
	if (values.nelements() == 2 && units.nelements() == 2) {
	  // create MDirection object
	  dir = casa::MDirection ( casa::Quantity( values(0), units(0)),
				   casa::Quantity( values(1), units(1)),
				   casa::MDirection::Ref(tp));
	  // return result
	  return dir;
	}
	else {
	  cerr << "[h5get_direction] Mismatching number of values and units!"
	       << endl;
	  cerr << "-- Values = " << values << endl;
	  cerr << "-- Units  = " << units  << endl;
	  dir = casa::MDirection();
	}
      }
      else {
	cerr << "[h5get_direction] Error translating reference code into type!"
	     << endl;
	cerr << "-- Refcode = " << refcode << endl;
	dir = casa::MDirection();
      }
    }
    else {
      cerr << "[h5get_direction] Unusable ID for HDF5 object!" << endl;
      dir = casa::MDirection();
    }
    
    return dir;
  }
  
  // ------------------------------------------------------------- h5get_position
  
  /*!
    \param location_id -- Identifier of the structure within the file, to which
           the attribut is attached to.
    \param value -- Identifier for the attribute storing the numerical value of
           the quantity.
    \param unit  -- Identifier for the attribute storing the physical unit of
           the quantity
    \param frame -- Identifier for the attribute storing the identifier for the
           reference frame within which the physical quantity is defined.

    \return position -- The position as casa::Measure.
  */
  MPosition h5get_position (hid_t const &location_id,
			    Attributes const &value,
			    Attributes const &unit,
			    Attributes const &frame)
  {
    return h5get_position (location_id,
                           attribute_name(value),
                           attribute_name(unit),
                           attribute_name(frame));
  }
  
  /*!
    \param location_id -- Identifier of the structure within the file, to which
           the attribut is attached to.
    \param value -- Identifier for the attribute storing the numerical value of
           the quantity.
    \param unit  -- Identifier for the attribute storing the physical unit of
           the quantity
    \param frame -- Identifier for the attribute storing the identifier for the
           reference frame within which the physical quantity is defined.

    \return position -- The position as casa::Measure.
  */
  MPosition h5get_position (hid_t const &location_id,
			    std::string const &value,
			    std::string const &unit,
			    std::string const &frame)
  {
    MPosition obs = MPosition();
    
    if (location_id > 0) {
      bool status (true);
      std::vector<double> values;
      std::vector<std::string> units;
      std::string refcode;

      // retrieve the numerical values of the position
      status *= h5get_attribute(location_id,
				value,
				values);

      // retrieve the physical units of the position
      status *= h5get_attribute(location_id,
				unit,
				units);

      // retrieve the frame of the position
      status *= h5get_attribute(location_id,
				frame,
				refcode);
      
      if (status) {
	if (values.size() == 3 && units.size() == 3) {
	  MPosition::Types tp;
	  // Translate reference code into MPosition::Type
	  status *= MPosition::getType (tp,refcode);
	  
	  switch (tp) {
	  case MPosition::ITRF:
	    obs = MPosition ( casa::MVPosition( values[0],
						values[1],
						values[2]),
			      MPosition::Ref(MPosition::ITRF));
	    break;
	  case MPosition::WGS84:
	    obs = MPosition ( casa::MVPosition( casa::Quantity( values[0], casa::String(units[0])),
						casa::Quantity( values[1], casa::String(units[1])),
						casa::Quantity( values[2], casa::String(units[2]))),
			      MPosition::Ref(MPosition::WGS84));
	    
	    break;
	  default:
	    std::cerr << "[h5get_position] Unknown MPosition::Type!" << endl;
	    break;
	  };
	  
	  return obs;
	}
	else {
	  cerr << "[h5get_position] Mismatching number of values and units!"
	       << endl;
	  obs = MPosition();
	}
      }
      else {
	cerr << "[h5get_position] Error retrieving components for MPosition!"
	     << endl;
	cerr << "-- Values  = " << values  << endl;
	cerr << "-- Units   = " << units   << endl;
	cerr << "-- Refcode = " << refcode << endl;
	obs = MPosition();
      }
    }
    else {
      cerr << "[h5get_position] Unusable ID for HDF5 object!" << endl;
      obs = MPosition();
    }
    
    return obs;
  }
  
#endif
  
} // namespace DAL

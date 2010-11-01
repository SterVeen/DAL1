/*-------------------------------------------------------------------------*
 | $Id::                                                                 $ |
 *-------------------------------------------------------------------------*
 ***************************************************************************
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

#ifndef COORDINATE_H
#define COORDINATE_H

/* Standard header files */
#include <iostream>
#include <set>
#include <string>

#ifdef HAVE_CASA
#ifndef WCSLIB_GETWCSTAB
#define WCSLIB_GETWCSTAB 1
#endif
#include <coordinates/Coordinates/Projection.h>
#include <measures/Measures/MDirection.h>
#endif

/* DAL header files */
#include <dalCommon.h>

namespace DAL {   // Namespace DAL -- begin

  /*!
    \class Coordinate

    \ingroup DAL
    \ingroup coordinates

    \brief A basic container for a coordinate object

    \author Lars B&auml;hren

    \date 2009/06/23

    <h3>Prerequisite</h3>

    <ul type="square">
      <li>Representations of World Coordinates. LOFAR-USG-ICD-002.
    </ul>

    <h3>Synopsis</h3>

    This class defines the basic interface and data contents for a coordinate
    object as being part of a HDF5-based LOFAR data-set (most prominently an
    image).

    <h3>Example(s)</h3>

  */
  class Coordinate {
    
  public:
    
    //! Type of the coordinate; for definition and attributes see LOFAR-USG-ICD-002
    enum Type {
      //! Direction coordinate
      Direction,
      //! Linear coordinate
      Linear,
      //! Tabulated coordinate
      Tabular,
      //! Stokes parameters coordinate
      Stokes,
      //! Spectral coordinate,
      Spectral,
      //! Undefined coordinate type
      NONE
    };
    
  protected:

    //! The type of coordinate
    Coordinate::Type coordinateType_p;
    //! Attributes attached to structure storing coordinate data
    std::set<std::string> attributes_p;

    //! The number of coordinate axes
    unsigned int nofAxes_p;
    //! World axis names
    std::vector<std::string> axisNames_p;
    //! World axis units
    std::vector<std::string> axisUnits_p;
    //! Reference value (CRVAL)
    std::vector<double> refValue_p;
    //! Reference pixel (CRPIX)
    std::vector<double> refPixel_p;
    //! Coordinate axis increment (CDELT)
    std::vector<double> increment_p;
    //! Transformation matrix (PC)
    std::vector<double> pc_p;
    
  public:
    
    // === Construction =========================================================
    
    //! Default constructor
    Coordinate ();
    //! Argumented constructor
    Coordinate (Coordinate::Type const &coordinateType,
		unsigned int const &nofAxes=0);
    //! Argumented constructor
    Coordinate (Coordinate::Type const &coordinateType,
		unsigned int const &nofAxes,
		std::vector<std::string> const &axisNames,
		std::vector<std::string> const &axisUnits,
		std::vector<double> const &refValue,
		std::vector<double> const &refPixel,
		std::vector<double> const &increment,
		std::vector<double> const &pc);
    //! Copy constructor
    Coordinate (Coordinate const &other);
    
    // === Destruction ==========================================================
    
    //! Destructor
    virtual ~Coordinate ();
    
    // === Operators ============================================================
    
    //! Overloading of the copy operator
    Coordinate& operator= (Coordinate const &other);
    
    // === Parameter access =====================================================
    
    //! Get the coordinate type
    inline Coordinate::Type type () const {
      return coordinateType_p;
    }
    //! Get the coordinate type as name
    std::string name ();

    //! Get the number of coordinate axes
    inline int nofAxes () const {
      return nofAxes_p;
    }
    
    //! Get the world axis names
    std::vector<std::string> axisNames () const {
      return axisNames_p;
    }
    //! Set the world axis names
    void setAxisNames (std::vector<std::string> const &axisNames) {
      if (axisNames.size() == nofAxes_p) {
	axisNames_p = axisNames;
      }
    }
    
    //! Get the world axis units
    std::vector<std::string> axisUnits () const {
      return axisUnits_p;
    }
    //! Set the world axis units
    void setAxisUnits (std::vector<std::string> const &axisUnits) {
      if (axisUnits.size() == nofAxes_p) {
	axisUnits_p = axisUnits;
      }
    }
    
    //! Get the reference value
    std::vector<double> refValue () const {
      return refValue_p;
    }
    //! Set the reference value
    virtual void setRefValue (std::vector<double> const &refValue) {
      if (refValue.size() == nofAxes_p) {
	refValue_p = refValue;
      }
    }
    
    //! Get the reference pixel
    std::vector<double> refPixel () const {
      return refPixel_p;
    }
    //! Set the reference pixel
    virtual void setRefPixel (std::vector<double> const &refPixel) {
      if (refPixel.size() == nofAxes_p)
	{
	  refPixel_p = refPixel;
	}
    }
    
    /*!
      \brief Get the coordinate axis increment
      \return increment -- The increment along the coordinate axes
    */
    std::vector<double> increment () const {
      return increment_p;
    }
    /*!
      \brief Set the coordinate axis increment
      \param increment -- The increment along the coordinate axes
    */
    virtual void setIncrement (std::vector<double> const &increment) {
      if (increment.size() == nofAxes_p) {
	increment_p = increment;
      }
    }
    
    /*!
      \brief Get the transformation matrix
      \return pc -- The transformation matrix, in row-wise ordering, e.g.
              [00,01,10,11]
    */
    std::vector<double> pc () const {
      return pc_p;
    }
    /*!
      \brief Set the transformation matrix
      \param pc -- The transformation matrix, in row-wise ordering, e.g.
             [00,01,10,11]
    */
    virtual void setPc (std::vector<double> const &pc) {
      unsigned int nelem = nofAxes_p*nofAxes_p;
      if (pc.size() == nelem) {
	pc_p = pc;
      }
    }
    
    /*!
      \brief Get the name of the class
      \return className -- The name of the class, Coordinate.
    */
    inline std::string className () const {
      return "Coordinate";
    }

    //! Attributes attached to structure storing coordinate data
    inline std::set<std::string> attributes () const {
      return attributes_p;
    }
    
    // === Methods ==============================================================
    
    //! Provide a summary of the internal status
    inline void summary () {
      summary (std::cout);
    }
    
    //! Provide a summary of the internal status
    void summary (std::ostream &os);
    
    //! Get name type of the coordinate as name
    static std::string getName (Coordinate::Type const &type);
    
    //! Get the type of the coordinate from its name
    static Coordinate::Type getType (std::string const &name);
    
#ifdef HAVE_HDF5
    //! Write the coordinate object to a HDF5 file
    virtual void h5write (hid_t const &locationID) = 0;
    
    //! Write the coordinate object to a HDF5 file
    virtual void h5write (hid_t const &locationID,
			  std::string const &name) = 0;
    
    //! Read the coordinate object from a HDF5 file
    virtual void h5read (hid_t const &locationID) = 0;
    
    //! Read the coordinate object from a HDF5 file
    virtual void h5read (hid_t const &groupID,
			 std::string const &name) = 0;
#endif
    
#ifdef HAVE_CASA
    //! Get the type of a reference system from its name
    static casa::MDirection::Types systemType (casa::String const &refcode) {
      // Local variables
      bool ok (true);
      casa::MDirection md;
      casa::MDirection::Types tp;
      // Get the type of the MDirection
      ok = md.getType(tp,refcode);
      // Return the result
      return tp;
    }
    //! Get the type of a projection from its name
    static casa::Projection::Type projectionType (casa::String const &refcode) {
      casa::Projection prj;	
      return prj.type(refcode);
    }
#endif
    
  protected:
    
    //! Unconditional copying
    void copy (Coordinate const &other);
    
    //! Initilize the internal set of parameters
    void init (unsigned int const &nofAxes=0);

    //! Set the basic set of attributes attached to the coordinate
    void setAttributes ();
    
  private:

    //! Unconditional deletion
    void destroy(void);
    
  }; // Class Coordinate -- end
  
} // Namespace DAL -- end

#endif /* COORDINATE_H */

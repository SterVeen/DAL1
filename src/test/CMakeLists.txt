## ------------------------------------------------------------------------------
## $Id::                                                                        $
## ------------------------------------------------------------------------------

find_file (HAVE_RUNTEST_SH runTests.sh ${CMAKE_CURRENT_SOURCE_DIR})

if(HAVE_RUNTEST_SH)
  file (COPY ${HAVE_RUNTEST_SH} DESTINATION ${DAL_BINARY_DIR})
endif (HAVE_RUNTEST_SH)

## ==============================================================================
##
##  Build the test programs
##
## ==============================================================================

if (NOT HDF5_USE_16_API_DEFAULT)
  add_subdirectory (hdf5)
endif (NOT HDF5_USE_16_API_DEFAULT)

if (DAL_PYTHON_BINDINGS)
  add_subdirectory (python)
endif (DAL_PYTHON_BINDINGS)

## Set up list of source files

set (tests
  tdal
  tdal_fits
  tDAL_HDF5
  )

## Build instructions

foreach (src ${tests})
  ## executable and its source
  add_executable(${src} ${src}.cc)
  ## libraries against which to link
  if (CASA_FOUND OR CASACORE_FOUND)
    target_link_libraries(${src}
      ${CASA_LIBRARIES}
      dal
      ${CASA_LIBRARIES}
      ${dal_link_libraries}
      )
  else (CASA_FOUND OR CASACORE_FOUND)
    target_link_libraries(${src} dal)
  endif (CASA_FOUND OR CASACORE_FOUND)
endforeach (src)

## ------------------------------------------------------------------------------
## Test which require additional external libraries (and thereby should be
## considered optional)

if (CASACORE_FOUND AND GFORTRAN_LIBRARIES AND LAPACK_LIBRARIES)

  ## list of source files
  foreach (src
      tdal_MS 
      tcasacore_hdf5
      tcasacore_images
      tcasacore_images2
      tcasacore_ms
      )
    ## compiler instructions
    add_executable (${src} ${src}.cc)
    ## linker instructions
    target_link_libraries (${src}
      dal
      ${dal_link_libraries}
      ${GFORTRAN_LIBRARIES}
      )
    ## installation
    list (APPEND tests ${src})
    ## testing
    if (DAL_ENABLE_TESTING)
      add_test (${src} ${src})
    endif (DAL_ENABLE_TESTING)
  endforeach (src)

endif (CASACORE_FOUND AND GFORTRAN_LIBRARIES AND LAPACK_LIBRARIES)

## ==============================================================================
##
##  Testing
##
## ==============================================================================

if (DAL_ENABLE_TESTING)
  
  add_test (tdal tdal)
  add_test (tDAL_HDF5 tDAL_HDF5)
  
  ## -----------------------------------------------------------------
  ## [3] Processing tests which require sample datasets
  
  if (dataset_ms)
    add_test (tdal_MS tdal_MS ${dataset_ms} ${dataset_ms}.h5)
  else (dataset_ms)
    message (STATUS "Unable to find MS dataset; skipping tests.")
  endif (dataset_ms)
  
  ## -----------------------------------------------------------------
  ## [6] Tests using FITS file format data
  
  if (dataset_fits)
    add_test (tdal_fits tdal_fits ${dataset_fits})
  endif (dataset_fits)
  if (dataset_fits)
    add_test (tdal_fits tdal_fits)
  endif (dataset_fits)
  
endif (DAL_ENABLE_TESTING)

## ------------------------------------------------------------------------------
## Installation

## test programs

install (TARGETS ${tests}
  RUNTIME DESTINATION ${DAL_INSTALL_BINDIR}
  LIBRARY DESTINATION ${DAL_INSTALL_LIBDIR}
  )
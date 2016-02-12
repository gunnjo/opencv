# - Find PYLON
# This module finds if PYLON Software package is installed
# and determines where the binaries and header files are.
# This code sets the following variables:
#
#  PYLON_FOUND          - True if PYLON API found
#  PYLON_PATH:          - Path to the PYLON API folder
#  PYLON_LIBRARY_DIR    - PYLON libraries folder
#
# Created: 5 Aug 2011 by Marian Zajko (marian.zajko@ximea.com)
# Updated: 25 June 2012 by Igor Kuzmin (parafin@ximea.com)
# Updated: 22 October 2012 by Marian Zajko (marian.zajko@ximea.com)
# Used to generate PYLON cmake by Joseph Gunn (joe@josephgunn.com)
#

set(PYLON_FOUND)
set(PYLON_PATH)
set(PYLON_LIBRARY_DIR)

message(STATUS "Looking for Pylon ${SPHINX_VERSION}: ${SPHINX_BUILD}")

if(WIN32)
  # TODO - Try and find windows equiv.
    set(PYLON_FOUND 0)
else()
  if(EXISTS /opt/pylon4)
    set(PYLON_FOUND 1)
    # set folders
    set(PYLON_PATH /opt/pylon4)
    set(PYLON_LIBRARY_DIR /opt/pylon4/lib64)
    set(GENICAM_LIB_SUFFIX _gcc40_v2_3)
    set(XERCES_LIB_SUFFIX _gcc40_v2_7_1)
  else()
    set(PYLON_FOUND 0)
  endif()
endif()

mark_as_advanced(FORCE PYLON_FOUND)
mark_as_advanced(FORCE PYLON_PATH)
mark_as_advanced(FORCE PYLON_LIBRARY_DIR)

message(STATUS "Pylon Library dir: ${PYLON_LIBRARY_DIR}")

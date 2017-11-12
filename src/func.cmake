### func.cmake --- 
## 
## Filename: func.cmake
## Author: Alex BIGUET
## Created: Mon Mar 20 12:52:56 2017 (+0100)
## Last-Updated: Wed Apr  5 15:26:15 2017 (+0200)
##           By: Alex BIGUET
##     Update #: 31
## 
## 
## 



################################################################################
## This function takes a list in input and removes from the list the
## item that match _dir. _verbose can be TRUE or FALSE. If true, then
## the function talks
##
## Output: this functions defines a variables NEW_LIST which contain
## the output list. This variable can be used is the parent scope
## (i.e. where this function were called)

function ( glob_recurse_without_dir _list _dir _verbose )

  foreach( itr ${_list} )
    if ( "${_verbose}" )
      message( STATUS "iteration: ${itr}" )
    endif()

    if ( "${itr}" MATCHES "(.*)${_dir}(.*)" )
      if ( "${_verbose}" )
	message( STATUS "Item ${itr} is part of ${_dir}: removed from list" )
      endif()
      list( REMOVE_ITEM _list ${itr} )
    endif()
  endforeach()
  set( NEW_LIST ${_list} PARENT_SCOPE )
endfunction()


################################################################################
## This function installs the files in _list into _dest

function ( install_files _list _dest _verbose)

  ## test if _dest ends with a '/'
  string( LENGTH "${_dest}" leng )
  math( EXPR begin "${leng}-1" )
  string( SUBSTRING "${_dest}" "${begin}" "1" last_char )

  # if ( "${_verbose}" )
  #   message( "last char = '${last_char}'" )
  # endif()
  
  if ( "${last_char}" STREQUAL "/" )

    if ( "${_verbose}" )
      message( "in ${_dest} : last char is a '/' " )
    endif()

    string( SUBSTRING "${_dest}" 0 "${begin}" _dest )
  endif()
  
  foreach( file ${_list} )
    get_filename_component( dir ${file} DIRECTORY )
    string( REPLACE "${CMAKE_SOURCE_DIR}/" "" dir "${dir}" )

    if ( "${_verbose}" )
      message( " ${file} will be installed to ${_dest}/${dir}" )
      message ("" )
    endif()
    install( FILES ${file} DESTINATION "${_dest}/${dir}" )
  endforeach()
endfunction()

################################################################################
## This function installs the files in _list into _dest and removes
## _rpath in the path of the files

function( install_files_rpath _list _dest _rpath _verbose )
  ## test if _dest ends with a '/'
  string( LENGTH "${_dest}" leng )
  math( EXPR begin "${leng}-1" )
  string( SUBSTRING "${_dest}" "${begin}" "1" last_char )
  
  if ( "${last_char}" STREQUAL "/" )

    if ( "${_verbose}" )
      message( "in ${_dest} : last char is a '/' " )
    endif()

    string( SUBSTRING "${_dest}" 0 "${begin}" _dest )
  endif()
  
  foreach( file ${_list} )
    get_filename_component( dir ${file} DIRECTORY )
    string( REPLACE "${CMAKE_SOURCE_DIR}/" "" dir "${dir}" )
    string( REPLACE "${_rpath}" "" dir "${dir}" )
    
    if ( "${_verbose}" )
      message( " ${file} will be installed to ${_dest}/${dir}" )
      message ("" )
    endif()
    install( FILES ${file} DESTINATION "${_dest}/${dir}" )
  endforeach()  
endfunction()

################################################################################
### func.cmake ends here

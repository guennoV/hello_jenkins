#
#  CMakeLists.txt
#  NetworkUri
#
#  Created by Alexandre Biguet on 15/09/2017 at 10:13.
#  Copyright © 2017 UAVIA. All rights reserved.
#

file( GLOB SOURCES "*.cpp" )
add_library( ${NETWORK_OBJ} OBJECT ${SOURCES} )
set_property( TARGET ${NETWORK_OBJ} PROPERTY POSITION_INDEPENDENT_CODE 1 )

file( GLOB SOURCES "detail/*.cpp" )
add_library( ${DETAIL_OBJ} OBJECT ${SOURCES} )
set_property( TARGET ${DETAIL_OBJ} PROPERTY POSITION_INDEPENDENT_CODE 1 )



# CMakeLists.txt ends here
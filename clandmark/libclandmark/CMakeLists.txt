include_directories(${PROJECT_SOURCE_DIR}/3rd_party/rapidxml-1.13)
aux_source_directory(${PROJECT_SOURCE_DIR}/3rd_party/rapidxml-1.13 RAPIDXML)

include_directories(${PROJECT_SOURCE_DIR}/3rd_party/CImg-1.5.6)
aux_source_directory(${PROJECT_SOURCE_DIR}/3rd_party/CImg-1.5.6 CIMG)

# propagate precision definition
if(DOUBLE_PRECISION)
	add_definitions( -DDOUBLE_PRECISION=1 )
else(DOUBLE_PRECISION)
	add_definitions( -DDOUBLE_PRECISION=0 )
endif(DOUBLE_PRECISION)

set(clandmark_headers
	${CIMG}
	#${CIMG_PLUGINS}
	${RAPIDXML}
	msvc-compat.h
	base64.h
	CLandmark.h
	CAppearanceModel.h
	CDeformationCost.h
	CLoss.h
	CMaxSumSolver.h
	CXMLInOut.h
	CTimer.h
)

set(clandmark_srcs
	${clandmark_headers}
	CLandmark.cpp
	CAppearanceModel.cpp
	CDeformationCost.cpp
	CLoss.cpp
	CMaxSumSolver.cpp
	CXMLInOut.cpp
)

set(flandmark_headers
	Flandmark.h
	CSparseLBPAppearanceModel.h
	CExtendedSparseLBPAppearanceModel.h
	CHOGAppearanceModel.h
	CDisplacementDeformationCost.h
	CNormalizedEuclideanLoss.h
	CTableLoss.h
	CZeroLoss.h
	CTreeMaxSumSolver.h
)

set(flandmark_srcs
	${flandmark_headers}
	Flandmark.cpp
	CSparseLBPAppearanceModel.cpp
	CExtendedSparseLBPAppearanceModel.cpp
	CHOGAppearanceModel.cpp
	CDisplacementDeformationCost.cpp
	CNormalizedEuclideanLoss.cpp
	CTableLoss.cpp
	CZeroLoss.cpp
	CTreeMaxSumSolver.cpp
)

# CLANDMARK
add_library(clandmark ${clandmark_srcs})
if(NOT ${BUILD_SHARED_LIBS})
	set_target_properties(clandmark PROPERTIES PREFIX "lib")
endif()
target_link_libraries(clandmark)
set_target_properties(clandmark PROPERTIES POSITION_INDEPENDENT_CODE TRUE)	# -fPIC
set_target_properties(clandmark PROPERTIES
	SOVERSION "${clandmark_VERSION_MAJOR}"
	VERSION "${clandmark_VERSION_MAJOR}.${clandmark_VERSION_MINOR}"
)

#install(TARGETS clandmark DESTINATION bin)
#install(FILES ${clandmark_headers} DESTINATION include)

# FLANDMARK
add_library(flandmark ${flandmark_srcs})
if(NOT ${BUILD_SHARED_LIBS})
	set_target_properties(flandmark PROPERTIES PREFIX "lib")
endif()
target_link_libraries(flandmark clandmark)
set_target_properties(flandmark PROPERTIES POSITION_INDEPENDENT_CODE TRUE)	# -fPIC
set_target_properties(flandmark PROPERTIES
	SOVERSION "${clandmark_VERSION_MAJOR}"
	VERSION "${clandmark_VERSION_MAJOR}.${clandmark_VERSION_MINOR}"
)

#install(TARGETS flandmark DESTINATION bin)
#install(FILES ${flandmark_headers} DESTINATION include)

#setup Config.cmake
set(CLANDMARK_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/libclandmark" "${PROJECT_SOURCE_DIR}/3rd_party/rapidxml-1.13" "${PROJECT_SOURCE_DIR}/3rd_party/CImg-1.5.6")
get_property(CLANDMARK_LIBRARIES TARGET clandmark PROPERTY LOCATION)
configure_file(clandmarkConfig.cmake.in
	"${PROJECT_BINARY_DIR}/libclandmark/clandmarkConfig.cmake" @ONLY
)

export(PACKAGE clandmark)

set(FLANDMARK_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/libclandmark" "${PROJECT_SOURCE_DIR}/3rd_party/rapidxml-1.13" "${PROJECT_SOURCE_DIR}/3rd_party/CImg-1.5.6")
get_property(FLANDMARK_LIBRARIES TARGET flandmark PROPERTY LOCATION)
configure_file(flandmarkConfig.cmake.in
	"${PROJECT_BINARY_DIR}/libclandmark/flandmarkConfig.cmake" @ONLY
)

export(PACKAGE flandmark)

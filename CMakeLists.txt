cmake_minimum_required(VERSION 3.22)
project(RA3 LANGUAGES CXX)

### setting up compilation variables ###
set(COMMON_FLAGS -mpclmul -msse2 -msse4.1) # for some of the stuff used in clhash
set(RELEASE_FLAGS)
set(DEBUG_FLAGS)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    list(APPEND COMMON_FLAGS -Wall -Wextra -Wpedantic -g)
    List(APPEND RELEASE_FLAGS -O3 -march=native)
    #list(APPEND DEBUG_FLAGS -fsanitize=address,undefined,leak -static-libasan -g)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(APPEND COMMON_FLAGS -Wall -Wextra -Wpedantic) # flags not tested
    List(APPEND RELEASE_FLAGS -O3 -march=native)
else()
    message(WARNING "Unexpected compiler (ID=${CMAKE_CXX_COMPILER_ID}) used. No flags set.")
endif()

add_compile_options(
    ${COMMON_FLAGS}
	"$<$<CONFIG:Release>:${RELEASE_FLAGS}>"
    "$<$<CONFIG:Debug>:${DEBUG_FLAGS}>"
)
# add all compile options to link options as well just to make sure
add_link_options(
    ${COMMON_FLAGS}
    "$<$<CONFIG:Release>:${RELEASE_FLAGS}>"
    "$<$<CONFIG:Debug>:${DEBUG_FLAGS}>"
)

# note that the pre-defined cache variables are always used for libraries and executables
#####################################

add_executable(RunAll main.cpp clhash/clhash.cpp)
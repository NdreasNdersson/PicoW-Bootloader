set( CMAKE_SYSTEM_NAME Linux )

set( COMPILER_BIN /usr/bin/ )
set( CMAKE_C_COMPILER ${COMPILER_BIN}/gcc CACHE PATH "gcc" )
set( CMAKE_CXX_COMPILER ${COMPILER_BIN}/g++ CACHE PATH "g++" )
set(CMAKE_C_FLAGS "-m32 -g -ggdb -O0")
set(CMAKE_CXX_FLAGS "-m32 -g -ggdb -O0")

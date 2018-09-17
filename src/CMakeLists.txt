

SET(nifpgadir "${CMAKE_SOURCE_DIR}/3rdparty/nifpga-cpp/nifpga-cpp/")
SET(nifpga_src ${nifpgadir}/NiFpga.c ${nifpgadir}/nifpga-cpp.hpp ${nifpgadir}/NiFpga.h)

add_executable(latency latency.cpp ${nifpga_src})
target_include_directories(latency PUBLIC ${nifpgadir} ${CMAKE_SOURCE_DIR}/3rdparty/cxxopts/include)
if (UNIX)
  target_link_libraries(latency dl pthread)
endif(UNIX)

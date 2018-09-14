

SET(nifpgadir "${CMAKE_CURRENT_SOURCE_DIR}/../../nifpga-cpp/nifpga-cpp/nifpga-cpp/")
SET(nifpga_src ${nifpgadir}/NiFpga.c ${nifpgadir}/nifpga-cpp.hpp ${nifpgadir}/NiFpga.h)

add_executable(latency latency.cpp ${nifpga_src})
target_include_directories(latency PUBLIC ${nifpgadir})

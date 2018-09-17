#include <iostream>
#include <iterator>
#include <string>
#include <algorithm>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include <fstream>

#include <cstdlib>
#include <ctime>
#include <cstring>


#include "nifpga-cpp.hpp"

#include "cxxopts.hpp"

#include "bitfile.hpp"

void test_registers(NiFpga_Session session) {
  std::cout << "Testing FPGA registers... ";
  for(int i = 0; i < 100; i++) {
    nifpga::writeRegister(session, reg_I32in, i);
    nifpga::writeRegister(session, reg_u8in, (unsigned char)i);

    if (nifpga::readRegister(session, reg_I32out) != i)
      throw std::runtime_error("Register test failed!");
    if (nifpga::readRegister(session, reg_u8out) != i)
      throw std::runtime_error("Register test failed!");
  }
  std::cout << "Success." << std::endl;
}

void test_fifos(NiFpga_Session session) {
  std::cout << "Testing FPGA FIFOs... " << std::endl;

  const int bufSize = 1024*1024;

  nifpga::configureFifo(session, fifo_FIFO_I32H2T, bufSize);
  nifpga::configureFifo(session, fifo_FIFO_I32T2H, bufSize);

  const int numRuns = 100;


  int32_t* buffer_in = new int[bufSize];
  int32_t* buffer_out = new int[bufSize];

  std::generate(buffer_in, buffer_in + bufSize, std::rand);

  std::cout << "   Transferring " << numRuns * bufSize * sizeof(int32_t) / 1024 / 1024 << " MB..." << std::endl;
  for (int i = 0; i < numRuns; i++) {
    //    std::copy(buffer_in, buffer_in+bufSize, std::ostream_iterator<int>(std::cout, " "));

    nifpga::writeFifo(session, fifo_FIFO_I32H2T, buffer_in, bufSize, 1000, nullptr);
    nifpga::readFifo(session, fifo_FIFO_I32T2H , buffer_out, bufSize, 1000, nullptr);

    if(!std::equal(buffer_in, buffer_in+bufSize, buffer_out))
      throw std::runtime_error("FIFO test failed!");
  }

  std::cout << "Success." << std::endl;
}

template <class data_type>
std::vector<uint64_t> measure_latency(NiFpga_Session session,
				 nifpga::Fifo<data_type>& fifo_in,
				 nifpga::Fifo<data_type>& fifo_out,
				 int elements_per_block, int num_tries) {
  using namespace std::chrono_literals;

  std::vector<uint64_t> result;
  result.reserve(num_tries);


  std::unique_ptr<data_type[]> buffer_in(new data_type[elements_per_block]);
  std::unique_ptr<data_type[]> buffer_out(new data_type[elements_per_block]);

  std::generate(buffer_in.get(), buffer_in.get() + elements_per_block, std::rand);

  for (int i = 0; i < num_tries; i++) {
    auto start = std::chrono::high_resolution_clock::now();

    nifpga::writeFifo(session, fifo_in, buffer_in.get(), elements_per_block, 1000, nullptr);
    nifpga::readFifo(session, fifo_out , buffer_out.get(), elements_per_block, 1000, nullptr);

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::nanoseconds elapsed = finish - start;
    result.push_back(elapsed.count());
    std::this_thread::sleep_for(100us);
  }
  return result;
}

int main() {
  std::srand(std::time(nullptr));

  try {
    nifpga::initialize();
    std::string path = "c:/Users/maximilian.matthe/Documents/FifoLatency/src/";

    std::cout << "Opening FPGA... ";
    NiFpga_Session session = nifpga::open((path + bitfile_filepath).c_str(), bitfile_signature, "RIO0", 0);
    std::cout << "done." << std::endl;

    test_registers(session);
    test_fifos(session);

    std::vector<int32_t> numBytes{1, 8, 16, 32, 64, 128, 256, 1024, 2048, 4096, 2*4096, 4*4096, 8*4096, 16*4096};
    for(auto bytes: numBytes) {
      std::cout << "Transmitting " << bytes << " bytes...";

      std::string fn = "results/I32_" + std::to_string(bytes) + ".txt";
      std::vector<uint64_t> lat_i32 = measure_latency(session,
						      fifo_FIFO_I32H2T,
						      fifo_FIFO_I32T2H,
						      bytes, 10000);
      std::ofstream outfile(fn);
      std::copy(lat_i32.begin(), lat_i32.end(), std::ostream_iterator<int>(outfile, "\n"));
      std::cout << " done." << std::endl;
    }
  }
  catch(nifpga::fpga_exception& e){
    std::cerr << e.what() << std::endl;
  }
}

#include <iostream>
#include <iterator>
#include <string>
#include <algorithm>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>

#include <cstdlib>
#include <ctime>
#include <cstring>


#include "nifpga-cpp.hpp"

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
				 int bytes_per_block, int num_tries) {
  using namespace std::chrono_literals;
  
  std::vector<uint64_t> result;
  result.reserve(num_tries);


  size_t num_elements = bytes_per_block / sizeof(data_type);
  bytes_per_block = num_elements * sizeof(data_type);
  std::unique_ptr<data_type[]> buffer_in(new data_type[num_elements]);
  std::unique_ptr<data_type[]> buffer_out(new data_type[num_elements]);

  std::generate(buffer_in.get(), buffer_in.get() + num_elements, std::rand);

  for (int i = 0; i < num_tries; i++) {
    auto start = std::chrono::high_resolution_clock::now();

    nifpga::writeFifo(session, fifo_in, buffer_in.get(), num_elements, 1000, nullptr);
    nifpga::readFifo(session, fifo_out , buffer_out.get(), num_elements, 1000, nullptr);
    
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

    std::vector<uint64_t> lat_i32 = measure_latency(session, fifo_FIFO_I32H2T, fifo_FIFO_I32T2H, 1024, 100);
    std::copy(lat_i32.begin(), lat_i32.end(), std::ostream_iterator<int>(std::cout, " "));
  }
  catch(nifpga::fpga_exception& e){
    std::cerr << e.what() << std::endl;
  }
}

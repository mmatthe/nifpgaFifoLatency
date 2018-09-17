#include <iostream>
#include <iterator>
#include <string>
#include <algorithm>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include <fstream>
#include <thread>
#include <functional>

#include <cstdlib>
#include <ctime>
#include <cstring>


#include "nifpga-cpp.hpp"

#include "cxxopts.hpp"

#include "bitfile.hpp"

#include "latency.hpp"


template <class data_type>
std::vector<uint64_t> measure_latency(NiFpga_Session session,
				 nifpga::Fifo<data_type> fifo_in,
				 nifpga::Fifo<data_type> fifo_out,
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
    if (i > 0)  // do not store the first try, as it may contain some updates
      result.push_back(elapsed.count());
    std::this_thread::sleep_for(100us);
  }
  return result;
}

template <class data_type>
std::function<std::vector<uint64_t>()> measureFunc(NiFpga_Session session,
						   nifpga::Fifo<data_type> fifo_in,
						   nifpga::Fifo<data_type> fifo_out,
						   int elements_per_block, int num_tries) {
  return [=]() { return measure_latency(session, fifo_in, fifo_out, elements_per_block, num_tries); };
}

std::map<std::string, std::vector<uint64_t> > measure_latencies(NiFpga_Session session, int num_runs, int num_elements, int parallelism) {

  std::map<std::string, std::function<std::vector<uint64_t>()>> functions;
  functions["I32"] = measureFunc(session, fifo_I32H2T, fifo_I32T2H, num_elements, num_runs);
  functions["U64"] = measureFunc(session, fifo_U64H2T, fifo_U64T2H, num_elements, num_runs);

  std::map<std::string, std::vector<uint64_t> > result;
  std::vector<std::thread> threads;
  for (auto& kv: functions) {
    threads.push_back(std::thread([&result, kv]() { result[kv.first] = kv.second(); }));
    if (threads.size() >= parallelism) {
      for(auto& t: threads) t.join();
      threads.clear();
    }
  }
  for(auto& t: threads) t.join();  // Wait for remaining threads

  return result;
}


void configureFifos(NiFpga_Session session) {
    const int DEPTH=100000000;
    nifpga::configureFifo(session, fifo_I32H2T, DEPTH);
    nifpga::configureFifo(session, fifo_I32T2H, DEPTH);
    nifpga::configureFifo(session, fifo_U64H2T, DEPTH);
    nifpga::configureFifo(session, fifo_U64T2H, DEPTH);

}

void testSystem(NiFpga_Session session) {
    std::cout << "Testing registers sequentially... ";
    test_registers(session, reg_I32in, reg_I32out);
    test_registers(session, reg_u8in, reg_u8out);
    std::cout << "Done." << std::endl;

    std::cout << "Testing FIFOs sequentially... " << std::endl;
    test_fifos(session, fifo_I32H2T, fifo_I32T2H, 100, 1024*1024);
    test_fifos(session, fifo_U64H2T, fifo_U64T2H, 100, 1024*1024);
    std::cout << "Done." << std::endl;

    std::vector<std::thread> threads;
    std::cout << "Testing FIFOs in parallel... " << std::endl;
    threads.push_back(std::thread([&]() { test_fifos(session, fifo_I32H2T, fifo_I32T2H, 100, 1024*1024);}));
    threads.push_back(std::thread([&]() { test_fifos(session, fifo_U64H2T, fifo_U64T2H, 100, 1024*1024);}));
    for(auto& T: threads)
      T.join();
    std::cout << "Done" << std::endl;
}


int main() {
  std::srand(std::time(nullptr));

  try {
    nifpga::initialize();
    std::string path = "c:/Users/maximilian.matthe/Documents/nifpgaFifoLatency/src/";

    std::cout << "Opening FPGA... ";
    NiFpga_Session session = nifpga::open((path + bitfile_filepath).c_str(), bitfile_signature, "RIO0", 0);
    std::cout << "done." << std::endl;

    configureFifos(session);
    //    testSystem(session);

    std::vector<int32_t> numElements{1, 8, 16, 32, 64, 128, 256, 1024, 2048, 4096, 2*4096};
    for(auto elems: numElements) {
      std::cout << "Transmitting " << elems << " elements...";

      std::map<std::string, std::vector<uint64_t> > latencies = measure_latencies(session, 100, elems, 4);

      for (auto kv: latencies) {
	std::string fn = "results/ " + kv.first + "_" + std::to_string(elems) + ".txt";

	std::ofstream outfile(fn);
	std::copy(kv.second.begin(), kv.second.end(), std::ostream_iterator<uint64_t>(outfile, "\n"));
      }
      std::cout << " done." << std::endl;
    }
  }
  catch(nifpga::fpga_exception& e){
    std::cerr << e.what() << std::endl;
  }
}

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
//  using namespace std::chrono_literals;

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
    std::this_thread::sleep_for(std::chrono::microseconds(100));
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
  functions["I8"] = measureFunc(session, fifo_I8_H2T, fifo_I8_T2H, num_elements, num_runs);
  functions["I16"] = measureFunc(session, fifo_I16_H2T, fifo_I16_T2H, num_elements, num_runs);
  functions["I32"] = measureFunc(session, fifo_I32H2T, fifo_I32T2H, num_elements, num_runs);
  functions["I64"] = measureFunc(session, fifo_I64_H2T, fifo_I64_T2H, num_elements, num_runs);
  functions["U8"] = measureFunc(session, fifo_U8_H2T, fifo_U8_T2H, num_elements, num_runs);
  functions["U16"] = measureFunc(session, fifo_U16_H2T, fifo_U16_T2H, num_elements, num_runs);
  functions["U32"] = measureFunc(session, fifo_U32_H2T, fifo_U32_T2H, num_elements, num_runs);
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
    const int DEPTH=10000000;
    nifpga::configureFifo(session, fifo_I8_H2T, DEPTH);
    nifpga::configureFifo(session, fifo_I8_T2H, DEPTH);
    nifpga::configureFifo(session, fifo_I16_H2T, DEPTH);
    nifpga::configureFifo(session, fifo_I16_T2H, DEPTH);
    nifpga::configureFifo(session, fifo_I32H2T, DEPTH);
    nifpga::configureFifo(session, fifo_I32T2H, DEPTH);
    nifpga::configureFifo(session, fifo_I64_H2T, DEPTH);
    nifpga::configureFifo(session, fifo_I64_T2H, DEPTH);

    nifpga::configureFifo(session, fifo_U8_H2T, DEPTH);
    nifpga::configureFifo(session, fifo_U8_T2H, DEPTH);
    nifpga::configureFifo(session, fifo_U16_H2T, DEPTH);
    nifpga::configureFifo(session, fifo_U16_T2H, DEPTH);
    nifpga::configureFifo(session, fifo_U32_H2T, DEPTH);
    nifpga::configureFifo(session, fifo_U32_T2H, DEPTH);
    nifpga::configureFifo(session, fifo_U64H2T, DEPTH);
    nifpga::configureFifo(session, fifo_U64T2H, DEPTH);
}

void testSystem(NiFpga_Session session) {
    std::cout << "Testing registers sequentially... ";
    test_registers(session, reg_I32in, reg_I32out);
    test_registers(session, reg_u8in, reg_u8out);
    std::cout << "Done." << std::endl;

    std::cout << "Testing FIFOs sequentially... " << std::endl;
    test_fifos(session, fifo_I8_H2T, fifo_I8_T2H, 100, 1024*1024);
    test_fifos(session, fifo_I16_H2T, fifo_I16_T2H, 100, 1024*1024);
    test_fifos(session, fifo_I32H2T, fifo_I32T2H, 100, 1024*1024);
    test_fifos(session, fifo_I64_H2T, fifo_I64_T2H, 100, 1024*1024);
    test_fifos(session, fifo_U8_H2T, fifo_U8_T2H, 100, 1024*1024);
    test_fifos(session, fifo_U16_H2T, fifo_U16_T2H, 100, 1024*1024);
    test_fifos(session, fifo_U32_H2T, fifo_U32_T2H, 100, 1024*1024);
    test_fifos(session, fifo_U64H2T, fifo_U64T2H, 100, 1024*1024);
    std::cout << "Done." << std::endl;

    std::vector<std::thread> threads;
    std::cout << "Testing FIFOs in parallel... " << std::endl;
    threads.push_back(std::thread([&]() { test_fifos(session, fifo_I8_H2T, fifo_I8_T2H, 100, 1024*1024);}));
    threads.push_back(std::thread([&]() { test_fifos(session, fifo_I16_H2T, fifo_I16_T2H, 100, 1024*1024);}));
    threads.push_back(std::thread([&]() { test_fifos(session, fifo_I32H2T, fifo_I32T2H, 100, 1024*1024);}));
    threads.push_back(std::thread([&]() { test_fifos(session, fifo_I64_H2T, fifo_I64_T2H, 100, 1024*1024);}));
    threads.push_back(std::thread([&]() { test_fifos(session, fifo_U8_H2T, fifo_U8_T2H, 100, 1024*1024);}));
    threads.push_back(std::thread([&]() { test_fifos(session, fifo_U16_H2T, fifo_U16_T2H, 100, 1024*1024);}));
    threads.push_back(std::thread([&]() { test_fifos(session, fifo_U32_H2T, fifo_U32_T2H, 100, 1024*1024);}));
    threads.push_back(std::thread([&]() { test_fifos(session, fifo_U64H2T, fifo_U64T2H, 100, 1024*1024);}));
    for(auto& T: threads)
      T.join();
    std::cout << "Done" << std::endl;
}

int main(int argc, char** argv) {
  std::srand(std::time(nullptr));

  cxxopts::Options options("FifoLatency", "Measure Latency of the nifpga FIFOs");
  options.add_options()
    ("d,directory", "Output directory", cxxopts::value<std::string>())
    ("p,parallel", "Number of parallely used FIFOs", cxxopts::value<unsigned int>())
    ("n,numruns", "Number of runs per FIFO", cxxopts::value<unsigned int>())
    ("r,rio", "RIO address to use", cxxopts::value<std::string>())
    ;

  try {
    auto arguments = options.parse(argc, argv);
    std::string outputDir = arguments["directory"].as<std::string>();
    unsigned int parallel = arguments["parallel"].as<unsigned int>();
    unsigned int numruns = arguments["numruns"].as<unsigned int>();
    std::string rio = arguments["rio"].as<std::string>();
    std::cout << "Running latency measurement into directory " << outputDir << std::endl;
    std::cout << "Using " << parallel << " parallel threads for measurement." << std::endl;
    std::cout << "Doing " << numruns << " per FIFO" << std::endl;
    system(("mkdir " + outputDir).c_str());

    nifpga::initialize();
    std::string path = "/home/root/programming/nifpgaFifoLatency/src/";
    
    std::cout << "Opening FPGA... ";
    NiFpga_Session session = nifpga::open((path + bitfile_filepath).c_str(), bitfile_signature, rio.c_str(), 0);
    std::cout << "done." << std::endl;

    configureFifos(session);
    testSystem(session);

    std::vector<int32_t> numElements{1, 8, 16, 32, 64, 128, 256, 1024, 2048, 4096, 2*4096, 4*4096, 8*4096, 16*4096};
    for(auto elems: numElements) {
      std::cout << "Transmitting " << elems << " elements...";

      std::map<std::string, std::vector<uint64_t> > latencies = measure_latencies(session, numruns, elems, parallel);

      for (auto kv: latencies) {
	std::string fn = outputDir + "/" + kv.first +
	  "_el" + std::to_string(elems) +
	  "_par" + std::to_string(parallel) +
	  ".txt";

	std::ofstream outfile(fn);
	std::copy(kv.second.begin(), kv.second.end(), std::ostream_iterator<uint64_t>(outfile, "\n"));
      }
      std::cout << " done." << std::endl;
    }
  }
  catch(nifpga::fpga_exception& e){
    std::cerr << "fpga exception: " << e.what() << std::endl;
  }
  catch(std::exception& e)  {
    std::cerr << "std exception: " << e.what() << std::endl;
  }
  catch(cxxopts::OptionException e) {
    std::cerr << "option exception: " << e.what() << std::endl;    
  }

}

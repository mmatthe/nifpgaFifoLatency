#ifndef LATENCY_H
#define LATENCY_H

template <class data_type>
void test_registers(NiFpga_Session session, const nifpga::Register<data_type>& reg_in, const nifpga::Register<data_type>& reg_out, int num_runs=100) {
  for(int i = 0; i < 100; i++) {
    nifpga::writeRegister(session, reg_in, (data_type)i);

    if (nifpga::readRegister(session, reg_out) != (data_type)i)
      throw std::runtime_error("Register test failed!");
  }
}


template<class data_type>
void test_fifos(NiFpga_Session session, nifpga::Fifo<data_type> fifo_in, nifpga::Fifo<data_type> fifo_out, const int num_runs, int num_elements) {

  std::unique_ptr<data_type[]> buffer_in(new data_type[num_elements]);
  std::unique_ptr<data_type[]> buffer_out(new data_type[num_elements]);

  std::generate(buffer_in.get(), buffer_in.get() + num_elements, std::rand);

  std::cout << "   Transferring " << num_runs * num_elements * sizeof(data_type) / 1024 / 1024 << " MB..." << std::endl;
  for (int i = 0; i < num_runs; i++) {
    //    std::copy(buffer_in, buffer_in+bufSize, std::ostream_iterator<int>(std::cout, " "));

    nifpga::writeFifo(session, fifo_in, buffer_in.get(), num_elements, 1000, nullptr);
    nifpga::readFifo(session, fifo_out , buffer_out.get(), num_elements, 1000, nullptr);

    if(!std::equal(buffer_in.get(), buffer_in.get()+num_elements, buffer_out.get()))
      throw std::runtime_error("FIFO test failed!");
  }
}

#endif /* LATENCY_H */

// Automatically generated at 2018-09-13 14:50:32.422014


// Bitfile
const char* bitfile_filepath  = "Bitfile-7975R-0002.lvbitx";
const char* bitfile_signature = "28EC3C5C7B6D124E15F517BE0C340D21";


// Registers
nifpga::Register<int32_t> reg_I32in(65536, "I32in");
nifpga::Register<int32_t> reg_I32out(65540, "I32out");
nifpga::Register<uint8_t> reg_u8in(65544, "u8in");
nifpga::Register<uint8_t> reg_u8out(65548, "u8out");

// FIFOS
nifpga::Fifo<uint64_t> fifo_FIFO_U64T2H(0, "FIFO_U64T2H");
nifpga::Fifo<int32_t> fifo_FIFO_I32T2H(1, "FIFO_I32T2H");
nifpga::Fifo<uint64_t> fifo_FIFO_U64H2T(2, "FIFO_U64H2T");
nifpga::Fifo<int32_t> fifo_FIFO_I32H2T(3, "FIFO_I32H2T");


// Automatically generated at 2018-09-17 15:37:40.845090


// Bitfile
const char* bitfile_filepath  = "Bitfile-7975R-0003.lvbitx";
const char* bitfile_signature = "59B5378421477AE74DBD3B6928CDE4F0";


// Registers
nifpga::Register<int32_t> reg_I32in(65536, "I32in");
nifpga::Register<int32_t> reg_I32out(65540, "I32out");
nifpga::Register<uint8_t> reg_u8in(65544, "u8in");
nifpga::Register<uint8_t> reg_u8out(65548, "u8out");

// FIFOS
nifpga::Fifo<uint32_t> fifo_U32_T2H(0, "U32_T2H");
nifpga::Fifo<uint16_t> fifo_U16_T2H(1, "U16_T2H");
nifpga::Fifo<uint8_t> fifo_U8_T2H(2, "U8_T2H");
nifpga::Fifo<uint32_t> fifo_U32_H2T(3, "U32_H2T");
nifpga::Fifo<uint16_t> fifo_U16_H2T(4, "U16_H2T");
nifpga::Fifo<uint8_t> fifo_U8_H2T(5, "U8_H2T");
nifpga::Fifo<int32_t> fifo_I32T2H(6, "I32T2H");
nifpga::Fifo<int16_t> fifo_I16_T2H(7, "I16_T2H");
nifpga::Fifo<int8_t> fifo_I8_T2H(8, "I8_T2H");
nifpga::Fifo<int32_t> fifo_I32H2T(9, "I32H2T");
nifpga::Fifo<int16_t> fifo_I16_H2T(10, "I16_H2T");
nifpga::Fifo<int8_t> fifo_I8_H2T(11, "I8_H2T");
nifpga::Fifo<uint64_t> fifo_U64T2H(12, "U64T2H");
nifpga::Fifo<int64_t> fifo_I64_T2H(13, "I64_T2H");
nifpga::Fifo<uint64_t> fifo_U64H2T(14, "U64H2T");
nifpga::Fifo<int64_t> fifo_I64_H2T(15, "I64_H2T");


#include "fecpp.h"
#include <iostream>
#include <fstream>
#include <sstream>

namespace {

size_t log2_ceil(size_t n)
   {
   size_t i = 0;
   while(n)
      {
      n >>= 1;
      ++i;
      }
   return i;
   }

template<typename T> inline byte get_byte(size_t byte_num, T input)
   {
   return (input >> ((sizeof(T)-1-(byte_num&(sizeof(T)-1))) << 3));
   }

void write_zfec_header(std::ostream& output,
                       size_t n, size_t k, size_t pad_bytes, size_t share_num)
   {
   // What a waste of effort to save, at best, 2 bytes. Blech.
   const size_t nbits = log2_ceil(n-1);
   const size_t kbits = log2_ceil(k-1);

   size_t out = (n - 1);
   out <<= nbits;
   out |= (k - 1);
   out <<= kbits;
   out |= pad_bytes;
   out <<= nbits;
   out |= share_num;

   size_t bitsused = 8 + kbits + nbits*2;

   if(bitsused <= 16)
      out <<= (16 - bitsused);
   else if(bitsused <= 24)
      out <<= (24 - bitsused);
   else if(bitsused <= 32)
      out <<= (32 - bitsused);

   printf("%08X %d\n", out, bitsused);

   for(size_t i = 0; i != (bitsused+7)/8; ++i)
      {
      unsigned char b = get_byte((bitsused+7)/8 + i, out);
      output.write((char*)&b, 1);
      printf("byte %d = %02X\n", i, b);
      }
   printf("\n");
   }

class zfec_file_writer
   {
   public:
      zfec_file_writer(const std::string& prefix,
                       size_t n, size_t k, size_t pad_bytes)
         {
         for(size_t i = 0; i != n; ++i)
            {
            std::ostringstream outname;
            outname << prefix << '.' << i << '_' << n << ".fec";

            std::ofstream* out = new std::ofstream(outname.str().c_str());
            write_zfec_header(*out, n, k, pad_bytes, i);
            outputs.push_back(out);
            }
         }

      ~zfec_file_writer()
         {
         for(size_t i = 0; i != outputs.size(); ++i)
            {
            outputs[i]->close();
            delete outputs[i];
            }
         }

      void operator()(size_t block, size_t /*max_blocks*/,
                      const byte buf[], size_t buflen)
         {
         outputs[block]->write((const char*)buf, buflen);
         }
   private:
      // Have to use pointers instead of obj as copy constructor disabled
      std::vector<std::ofstream*> outputs;
};

}

void zfec_encode(size_t k, size_t n,
                 const std::string& prefix,
                 std::istream& in,
                 size_t in_len)
   {
   const size_t chunksize = 4096;

   fec_code fec(k, n);

   std::vector<byte> buf(chunksize * fec.get_k());

   size_t pad_bytes = (in_len % k == 0) ? 0 : k - (in_len % k);

   zfec_file_writer file_writer(prefix, fec.get_n(), fec.get_k(), pad_bytes);

   while(in.good())
      {
      in.read((char*)&buf[0], buf.size());
      size_t got = in.gcount();

      if(got == buf.size())
         fec.encode(&buf[0], buf.size(), std::tr1::ref(file_writer));
      else
         {
         // Handle final block by padding up to k bytes with 0s
         for(size_t i = 0; i != pad_bytes; ++i)
            buf[i+got] = 0;
         fec.encode(&buf[0], got + pad_bytes, std::tr1::ref(file_writer));
         }
      }
   }

void zfec_encode(size_t k, size_t n,
                 const std::string& prefix,
                 std::ifstream& in)
   {
   in.seekg (0, std::ios::end);
   size_t in_length = in.tellg();
   in.seekg (0, std::ios::beg);

   zfec_encode(k, n, prefix, in, in_length);
   }

int main()
   {
   std::ifstream in("tests.txt");
   zfec_encode(3, 5, "tests.txt", in);
   }
#include "WireCellSigProc/Diagnostics.h"
#include "WireCellSigProc/Operations.h"
#include "WireCellUtil/Waveform.h"
#include "WireCellUtil/Testing.h"

#include <iostream>
#include <string>


// provides vectors "horig" and "hfilt"
// generate like:
// $ dump-root-hist-to-waveform sigproc/test/example-chirp.root horig hfilt > sigproc/test/example-chirp.h
#include <vector>
#include "example-noisy.h"

using namespace std;

using namespace WireCell;
using namespace WireCellSigProc;

int main(int argc, char* argv[])
{
  int ch = 0;
   Operations::SignalFilter(horig);
   bool is_noisy = Operations::NoisyFilterAlg(horig,ch);
   assert(is_noisy);
}
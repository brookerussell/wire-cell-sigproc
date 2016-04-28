#include "WireCellSigProc/Diagnostics.h"
#include "WireCellUtil/Testing.h"

#include <iostream>
#include <string>

// provides vectors "horig" and "hfilt"
// generate like:
// $ dump-root-hist-to-waveform sigproc/test/example-chirp.root horig hfilt > sigproc/test/example-chirp.h
#include <vector>
#include "example-chirp.h"

using namespace std;

using namespace WireCell;
using namespace WireCellSigProc;

int main(int argc, char* argv[])
{
    Waveform::signal_t wforig = Eigen::Map<Waveform::signal_t>(horig.data(), horig.size());
    Waveform::signal_t wfwant = Eigen::Map<Waveform::signal_t>(hfilt.data(), hfilt.size());

    int beg, end;
    Diagnostics::Chirp chirp;
    bool found = chirp(wforig, beg, end);
    Assert(found);

    // the function should find something starting at the beginning.
    Assert(beg == 0);
    // and the chirp does not extend to the end
    Assert(end !=0 && end != wforig.size());

    Assert (beg >= 0);
    Assert (end >= 0);
    Assert (beg < end);

    // The algorithm works in chunks of 20
    Assert ((end-beg)%20 == 0);

    for (int ind=beg; ind<end; ++ind) {
	Assert(wfwant[ind] == 0);
    }

    cerr << "chirp at " << beg << " --> " << end << endl;
    Assert (end == 4240);

    return 0;
}

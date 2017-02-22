#!/usr/bin/env python
'''

$ PYTHONPATH=`pwd`/sigproc/python python sigproc/python/wirecell/sigproc/paper/noise.py 


'''

from wirecell.sigproc import units, garfield, response, plots
import numpy

garfield_tarball = "/home/bviren/projects/wire-cell/garfield-data/ub_10.tar.gz"
garfield_tarball = "/opt/bviren/wct-dev/data/ub_10.tar.gz"

def filter_response_functions(dat, regions=None):
    if regions is None:
        return dat
    return [d for d in dat if abs(d.region) in regions]
    

def figure_adc(dat, regions=None):
    dat = filter_response_functions(dat, regions)
    nwires = 10
    if regions is not None:
        nwires = max(regions)

    norm = 16000*units.electron_charge    # was 13700
    uvw = response.line(dat, norm)

    gain = 1.2                            # was 1.1 for a while
    adc_bin_range = 4096.0
    adc_volt_range = 2000.0
    adc_per_mv = gain*adc_bin_range/adc_volt_range

    fig,data = plots.plot_digitized_line(uvw, 14.0, 2.0*units.us,
                                         adc_per_mv = adc_per_mv)

    fig.savefig('paper-noise-figure-adc-%dwires.pdf' % nwires)

    with open('paper-noise-figure-adc-%dwires.txt'%nwires,'w') as fp:
        for t,u,v,w in data:
            fp.write('%f %e %e %e\n' % (t,u,v,w))
            



if '__main__' == __name__:
    for nwires in range(1,11):
        dat = garfield.load(garfield_tarball)
        figure_adc(dat, range(nwires))
        
    

# mwa-xc-datacapture
## Dependencies
* CFITSIO (https://heasarc.gsfc.nasa.gov/fitsio/fitsio.html)
* pkg-config (this is to ensure psrdada configure script works correctly)
* hwloc (this is to enable the use of NUMA awareness in psrdada)
* csh (required for building psrdada)
* autoconf (required for building psrdada)
* psrdada (http://psrdada.sourceforge.net):
    ./bootstrap
    ./configure
    make
    make install
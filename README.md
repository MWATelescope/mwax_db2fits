# mwax-db2fits
MWA Correlator (mwax): PSRDADA ringbuffer to FITS file converter
## Dependencies
### CFITSIO 
- See https://heasarc.gsfc.nasa.gov/fitsio/fitsio.html
### psrdada prerequisites:
- pkg-config
- libhwloc-dev (this is to enable the use of NUMA awareness in psrdada)
- csh
- autoconf
- libtool
### psrdada (http://psrdada.sourceforge.net/)
- download the source from the cvs repo (http://psrdada.sourceforge.net/current/)
- build (http://psrdada.sourceforge.net/current/build.shtml)

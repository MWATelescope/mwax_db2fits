# mwax-db2fits

MWA Correlator (mwax): PSRDADA ringbuffer to FITS file converter

## Dependencies

### CFITSIO

- See <https://heasarc.gsfc.nasa.gov/fitsio/fitsio.html>

### psrdada prerequisites

- pkg-config
- libhwloc-dev (this is to enable the use of NUMA awareness in psrdada)
- csh
- autoconf
- libtool

### psrdada (<http://psrdada.sourceforge.net/>)

- download the source from the cvs repo (<http://psrdada.sourceforge.net/current/>)
- build (<http://psrdada.sourceforge.net/current/build.shtml>)

## Building

```bash
cmake CMakeLists.txt
make
```

## Running / Command line Arguments

Example from `mwax_db2fits --help`

```bash
mwax_db2fits v0.10.3

Usage: mwax_db2fits [OPTION]...

This code will open the dada ringbuffer containing raw
visibility data from the MWAX Correlator.
It will then write out a fits file to be picked up by the
archiver process.

  -k --key=KEY                      Hexadecimal shared memory key
  -d --destination-path=PATH        Destination path for gpubox files
  -n --health-netiface=INTERFACE    Health UDP network interface to send with
  -i --health-ip=IP                 Health UDP Multicast destination ip address
  -p --health-port=PORT             Health UDP Multicast destination port
  -l --file-size-limit=BYTES        FITS file size limit before splitting into a new file. Default=10737418240 bytes. 0=no splitting
  -v --version                      Display version number
  -? --help                         This help text
  ```

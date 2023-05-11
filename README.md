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

## Building Release version

```bash
./build.sh
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

## Testing an Debugging

### Build the Debug Binary

```bash
./build_debug.sh
```

### Run all tests

```bash
./run_tests.sh
```

## Health Packet format

Every 1 second, a UDP health packet is sent to the `health_ip` and `health_port` via the `health-netiface` interface.

The payload is a packed C struct with the following format:

  Type     | Name             | Example | Notes   |
|----------|------------------|---------|---------|
| int16    | version_major    |    1    |  mwax_db2fits major version number       |
| int16    | version_minor    |    2    |  mwax_db2fits minor version number        |
| int16    | version_revision |    3    |  mwax_db2fits revision number       |
| char[64] | hostname         | mwax01  |  hostanme of the server       |
| time_t   | start_time       | 1683779031        | UNIX Time when program was started        |
| time_t   | health_time      | 1683780123        | UNIX Time when health packet was assembled        |
| float64  | up_time          |  1092       | Number of seconds alive        |
| int16    | status           |    1     | 0 = Offline, 1= Running, 2= shutting down        |
| int32    | obs_id           | 1234567890        |  obs_id GPS time or 0 if no current observation       |
| int32    | subobs_id        | 1234567890        |  sub_obs_id GPS time or 0 if no current observation       |
| float32[256] | weights_per_tile_x| 1.0,0.9,0.92,1.0...        | Each element is tile 0..255 X pol weight. If ntiles is <256, unused tiles will have NaN. If no weights can be reported then the array will have 256 NaN elements. Tile order is MWAX order. |
| float32[256] | weights_per_tile_y| 1.0,0.9,0.92,1.0...        | Each element is tile 0..255 Y pol weight. If ntiles is <256, unused tiles will have NaN. If no weights can be reported then the array will have 256 NaN elements. Tile order is MWAX oder.  |

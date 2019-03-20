from astropy.io import fits
import os
import time


def fit_compress(path_in, path_out, compression_type, quantize_level):
    # Get size of input file
    input_size = os.path.getsize(path_in)

    print(f"Compressing {path_in} with {compression_type} and quantize level {quantize_level}... ")

    start_time = time.time()

    with fits.open(path_out, mode='append') as comphdulist:
        with fits.open(path_in, do_not_scale_image_data=True) as hdulist:
            n_hdu = 0
            for hdu in hdulist:
                n_hdu += 1
                print(f"Processing hdu {n_hdu} of {len(hdulist)}...")
                if isinstance(hdu, fits.PrimaryHDU):
                    comphdu = fits.PrimaryHDU(header=hdu.header, data=hdu.data)
                elif isinstance(hdu, fits.CompImageHDU):
                    comphdu = hdu
                else:
                    comphdu = fits.CompImageHDU(header=hdu.header,
                                                data=hdu.data,
                                                compression_type=compression_type,
                                                quantize_level=quantize_level,
                                                quantize_method=None)

                comphdulist.append(comphdu)

    time_sec = time.time() - start_time

    # Get size of output file
    output_size = os.path.getsize(path_out)

    # Work out rate (bytes per sec to MB per sec)
    rate = (output_size / time_sec) / (1000*1000)

    # Report compression
    ratio = input_size / output_size
    print(f"Done. File was {input_size} bytes and is "
          f"now {output_size} bytes, compression ratio {ratio:.2f}:1. Time taken {time_sec} secs which "
          f"is {rate} MB/sec.")


# run
def run_tests():
    comp_type = "RICE_1"

    for q in range(21, 0, -4):
        qlevel = -q
        fit_compress("1223270792_20181011052614_ch76_000.fits", f"compressed{q}.fits", comp_type, qlevel)


run_tests()
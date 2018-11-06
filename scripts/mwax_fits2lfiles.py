from astropy.io import fits
import argparse
import struct
import math
import numpy as np

def process_fits(input_filename, output_cc_filename, output_ac_filename):
    # constants
    values = 2  # real and imaginary
    pols = 4  # xx,xy,yx,yy

    fits_hdu_list = fits.open(input_filename)
    print("Read in {0} image HDUs...".format(len(fits_hdu_list)-1))

    # Get number of tiles based on baseline count
    bl = fits_hdu_list[1].header["NAXIS2"]
    input_data_tiles = int((-1 + math.sqrt(1 + (8 * bl))) / 2)

    # Get fine channel count
    chan_x_pols_x_vals = fits_hdu_list[1].header["NAXIS1"]
    channels = int((chan_x_pols_x_vals / pols) / values)

    timestep = 0

    with open(output_cc_filename, "wb") as output_cc_file:
        with open(output_ac_filename, "wb") as output_ac_file:
            for hdu in fits_hdu_list[1:]:
                baseline = 0

                for i in range(0, input_data_tiles):
                    for j in range(i, input_data_tiles):
                        data = hdu.data[baseline]

                        #fmt = 'f' * len(data)
                        #bin = struct.pack(fmt, *data)

                        a_data = np.empty(channels, dtype=float)

                        print("Expecting 0 - {0} step {1}  per baseline!".format(channels*pols*2, pols*2))
                        exit(0)

                        # Write to correct file if it is auto or cross
                        if i == j:
                            for c in range(0, channels*pols*2, pols*2):
                                # Just output every 4th real value (ie with 4 pols, output every 8th value)
                                np.append(a_data, data[c])
                            a_data.tofile(output_ac_file)
                        else:
                            data.tofile(output_cc_file)

                        baseline = baseline + 1

                print("finished timestep {0}".format(timestep))
                timestep = timestep + 1

    fits_hdu_list.close()


if __name__ == '__main__':
    description = "mwax correlator fits file to l_files utility"
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument("-i", "--input_mwax_filename", required=True, help="input mwax fits filename")
    parser.add_argument("-c", "--output_cross_filename", required=True, help="output cross correlations filename")
    parser.add_argument("-a", "--output_autos_filename", required=True, help="output auto correlations filename")

    args = vars(parser.parse_args())

    process_fits(args["input_mwax_filename"], args["output_cross_filename"], args["output_autos_filename"])

from astropy.io import fits
import argparse
import struct
import numpy


def process_fits(input_filename, output_cc_filename, output_ac_filename):
    input_data_tiles = 128
    pols = 4
    channels = 32

    fits_hdu_list = fits.open(input_filename)
    print("Read in {0} image HDUs...".format(len(fits_hdu_list)-1))

    timestep = 0

    with open(output_cc_filename, "wb") as output_cc_file:
        with open(output_ac_filename, "wb") as output_ac_file:
            for hdu in fits_hdu_list[1:]:
                baseline = 0

                for i in range(0, input_data_tiles):
                    for j in range(i, input_data_tiles):
                        data = hdu.data[baseline]

                        fmt = 'f' * len(data)
                        bin = struct.pack(fmt, *data)

                        # Write to correct file if it is auto or cross
                        if i == j:
                            output_ac_file.write(bin)
                            print("wrote auto baseline {0}".format(baseline))
                        else:
                            output_cc_file.write(bin)
                            print("wrote cross baseline {0}".format(baseline))

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

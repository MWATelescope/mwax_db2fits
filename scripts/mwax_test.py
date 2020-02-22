from astropy.io import fits
import argparse
import numpy as np

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("filename", help="fits filename")
    args = vars(parser.parse_args())

    filename = args["filename"]

    # Open fits
    print(f"Opening fits file {filename}...")
    fits_hdu_list = fits.open(filename)

    fits_tiles = int(fits_hdu_list[0].header["NINPUTS"] / 2)

    pols = 4    # xx,xy,yx,yy
    values = 2  # r,i

    ch_start = 0
    ch_end = 0

    sum_of_channels = 0
    timesteps = 0
    iterations = 0

    h = 0
    for hdu in fits_hdu_list:
        # Skip any even HDUs. 0 == Primary, Other even are the weights
        if h % 2 != 0:
            baseline = 0
            timesteps += 1

            data = np.array(hdu.data, dtype=np.float32)

            for i in range(0, fits_tiles):
                for j in range(i, fits_tiles):
                    for chan in range(ch_start, ch_end + 1):
                        index = chan * (pols * values)

                        xx_r = data[baseline][index]
                        xx_i = data[baseline][index + 1]

                        xy_r = data[baseline][index + 2]
                        xy_i = data[baseline][index + 3]

                        yx_r = data[baseline][index + 4]
                        yx_i = data[baseline][index + 5]

                        yy_r = data[baseline][index + 6]
                        yy_i = data[baseline][index + 7]

                        if i == 0 and j == 0 and timesteps == 1 and chan == 0:
                            print(xx_r, xx_i, xy_r, xy_i, yx_r, yx_i, yy_r, yy_i)

                        sum_of_channels = sum_of_channels + xx_r + xx_i + xy_r + xy_i + yx_r + yx_i + yy_r + yy_i
                        iterations += 1

                    baseline += 1
        h = h + 1

    print(f"After {baseline} baselines (for {timesteps} timesteps and  {iterations} iterations), "
          f"the sum of ch{ch_start}-ch{ch_end} == {sum_of_channels:g} == {sum_of_channels}"
          f" and mean is {sum_of_channels/iterations}")
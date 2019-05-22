from astropy.io import fits
import argparse


def dump(filename, hdu_number):
    fits_hdu_list = fits.open(filename)
    hdu_number = int(hdu_number)

    print(f"{repr(fits_hdu_list[hdu_number].header)}")

    # clean up
    fits_hdu_list.close()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("filename", help="fits filename")
    parser.add_argument("-hdu", "--hdu", required=False, help="Dump HDU", default=0)
    args = vars(parser.parse_args())

    dump(args["filename"], args["hdu"])

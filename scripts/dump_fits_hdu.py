from astropy.io import fits
import argparse


def dump(filename, hdu_number):
    fits_hdu_list = fits.open(filename)
    hdu_number = int(hdu_number)

    tile_array = []

    if fits_hdu_list[hdu_number].name == "TILEDATA":
        for t in range(0,256): 
            input = fits_hdu_list[hdu_number].data[t]["INPUT"]
            antenna = fits_hdu_list[hdu_number].data[t]["ANTENNA"]
            tile = fits_hdu_list[hdu_number].data[t]["TILE"]
            tilename = fits_hdu_list[hdu_number].data[t]["TILENAME"]
            pol =  fits_hdu_list[hdu_number].data[t]["POL"]

            tile_array.append([tile, input, antenna, tilename, pol])

        sorted_tile_array = sorted(tile_array)

        print("input, antenna, tile, tilename, pol\n") 
        for r in sorted_tile_array:
            print(r[1], r[2], r[0], r[3], r[4])
    else:
        print(f"{repr(fits_hdu_list[hdu_number].header)}")

    # clean up
    fits_hdu_list.close()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("filename", help="fits filename")
    parser.add_argument("-hdu", "--hdu", required=False, help="Dump HDU", default=0)
    args = vars(parser.parse_args())

    dump(args["filename"], args["hdu"])

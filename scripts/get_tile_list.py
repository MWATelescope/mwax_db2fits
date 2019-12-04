from astropy.io import fits
import argparse


def dump(filename):
    fits_hdu_list = fits.open(filename, mode="update")

    # Determine the tile order
    rf_list = []
    tile_list = []

    for row in fits_hdu_list[1].data:
        rf_order = -1
        tile_order = -1

        if row["Pol"] == "X":
            rf_order = row["Antenna"] * 2
            tile_list.append((row["Antenna"], row["Tile"], row["TileName"]))

        elif row["Pol"] == "Y":
            rf_order = (row["Antenna"] * 2) + 1

        rf_list.append((rf_order, row["Antenna"], row["Pol"], row["Tile"], row["TileName"]))

    rf_list.sort()
    tile_list.sort()

    print("Order, Tile, TileName")
    for tile in tile_list:
        print(tile)

    for x in fits_hdu_list[1].data:
        #x[8]="EL_0"
        print(x[8])

    # clean up
    fits_hdu_list.close()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("filename", help="metafits filename")
    args = vars(parser.parse_args())

    dump(args["filename"])

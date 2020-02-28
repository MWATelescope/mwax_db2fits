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

    #rf_list.sort()
    #tile_list.sort()

    print("Input, Antenna, TileName, Pol, VCS_ORDER, Rx, VCS Order, Calc VCS Order, Subfile order, Flag?")
    #for tile in tile_list:
    #    print(tile)

    for x in fits_hdu_list[1].data:
        #x[8]="EL_0"
        try:
           vcs_order = x["VCSOrder"]
        except:
           vcs_order = -1

        inum = x["Input"] 
        bob = (inum & 0xC0) | ((inum & 0x30) >> 4) | ((inum & 0x0F) << 2)
        bob2 = ((inum) & 0xc0) | (((inum) & 0x03) << 4) | (((inum) & 0x3c) >> 2)
        subfile_order = (x["Antenna"] << 1) + (1 if x["Pol"]=="Y" else 0)
        print(f'{x["Input"]}, {x["Antenna"]}, {x["TileName"]}, {x["Pol"]},{x["Rx"]}, {vcs_order}, {bob2}, {subfile_order}, {x["Flag"]}')

    # clean up
    fits_hdu_list.close()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("filename", help="metafits filename")
    args = vars(parser.parse_args())

    dump(args["filename"])

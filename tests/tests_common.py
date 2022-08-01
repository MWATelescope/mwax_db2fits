from astropy.io import fits
import numpy as np


def read_fits_hdu(filename: str, hdu: int) -> np.array:
    with fits.open(filename) as fits_file:
        return fits_file[hdu].data

def count_fits_hdus(filename: str) -> int:
    with fits.open(filename) as fits_file:
        return len(fits_file)


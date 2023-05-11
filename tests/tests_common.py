from astropy.io import fits
import numpy as np


def read_fits_hdu(filename: str, hdu: int) -> np.array:
    with fits.open(filename) as fits_file:
        return fits_file[hdu].data


def count_fits_hdus(filename: str) -> int:
    with fits.open(filename) as fits_file:
        return len(fits_file)


def assert_substring_in_file(filename: str, substring: str):
    with open(filename, encoding="UTF-8") as logfile:
        lines = logfile.readlines()

        # Find a match for the substring
        found = False

        for line in lines:
            if substring in line:
                found = True
                break

        assert found, f"{filename} did not include {substring}"

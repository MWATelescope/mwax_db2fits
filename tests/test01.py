#
# Test01: Analyse output files and/or logs from this test of mwax_db2fits
#
from astropy.io import fits
from math import isclose
import numpy as np
import os
from tests_common import read_fits_hdu, count_fits_hdus

TEST01_FITS_FILENAME = "test01/1324440018_20211225040000_ch148_000.fits"


def test01_fits_file_produced():
    # Check a FITS file was produced
    assert os.path.exists(TEST01_FITS_FILENAME)


def test01_fits_file_has_correct_hdus():
    # Check the output fits file has 1 primary + 8 HDUs
    # 1 V + 1 W per timestep == 4 x 2 = 8 + primary == 9
    assert 9 == count_fits_hdus(TEST01_FITS_FILENAME)


def test01_fits_file_has_correct_hdu_dimensions():
    with fits.open(TEST01_FITS_FILENAME) as fits_file:
        # Visibilities
        for h in range(1, 9, 2):
            d = fits_file[h].data

            assert d.shape[0] == 3
            assert d.shape[1] == 16

        # Weights
        for h in range(2, 9, 2):
            d = fits_file[h].data

            assert d.shape[0] == 3
            assert d.shape[1] == 4


def test01_check_hdu_values():    
    data1 = read_fits_hdu(TEST01_FITS_FILENAME, 1)
    assert 5928 == np.sum(data1)
    weights1 = read_fits_hdu(TEST01_FITS_FILENAME, 2)
    assert isclose(3.3, np.sum(weights1), rel_tol=1e-6)

    data2 = read_fits_hdu(TEST01_FITS_FILENAME, 3)
    assert 10728 == np.sum(data2)
    weights2 = read_fits_hdu(TEST01_FITS_FILENAME, 4)
    assert isclose(3.9, np.sum(weights2), rel_tol=1e-6)

    data3 = read_fits_hdu(TEST01_FITS_FILENAME, 5)
    assert 15528 == np.sum(data3)
    weights3 = read_fits_hdu(TEST01_FITS_FILENAME, 6)
    assert isclose(4.5, np.sum(weights3), rel_tol=1e-6)

    data4 = read_fits_hdu(TEST01_FITS_FILENAME, 7)
    assert 20328 == np.sum(data4)
    weights4 = read_fits_hdu(TEST01_FITS_FILENAME, 8)
    assert isclose(5.1, np.sum(weights4), rel_tol=1e-6)

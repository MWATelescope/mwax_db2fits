#
# Test01: Analyse output files and/or logs from this test of mwax_db2fits
#
from astropy.io import fits
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
    for t in range(0, 2):
        data1 = read_fits_hdu(TEST01_FITS_FILENAME, 1 + (t * 4))
        assert 1128 == np.sum(data1)

        data2 = read_fits_hdu(TEST01_FITS_FILENAME, 2 + (t * 4))
        assert 1266 == np.sum(data2)

        data3 = read_fits_hdu(TEST01_FITS_FILENAME, 3 + (t * 4))
        assert 10728 == np.sum(data3)

        data4 = read_fits_hdu(TEST01_FITS_FILENAME, 4 + (t * 4))
        assert 3666 == np.sum(data4)
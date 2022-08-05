#
# Test03: Analyse output files and/or logs from this test of mwax_db2fits
#
from astropy.io import fits
import numpy as np
import os
from tests_common import read_fits_hdu, count_fits_hdus

TEST03_FITS_FILENAME_1 = "test03/1324440018_20211225040000_ch148_000.fits"
TEST03_FITS_FILENAME_2 = "test03/1324440018_20211225040000_ch148_001.fits"


def test03_fits_file_1_produced():
    # Check FITS file was produced
    assert os.path.exists(TEST03_FITS_FILENAME_1)


def test03_fits_file_2_produced():
    # Check FITS file was produced
    assert os.path.exists(TEST03_FITS_FILENAME_2)


def test03_fits_file_1_has_correct_hdus():
    # Check the output fits file has 1 primary + 8 HDUs
    # 1 V + 1 W per timestep == 2 x 4 = 8 + primary == 9
    assert 9 == count_fits_hdus(TEST03_FITS_FILENAME_1)


def test03_fits_file_2_has_correct_hdus():
    # Check the output fits file has 1 primary + 4 HDUs
    # 1 V + 1 W per timestep == 2 x 2 = 4 + primary == 5
    assert 5 == count_fits_hdus(TEST03_FITS_FILENAME_2)


def test03_fits_file_1_has_correct_hdu_dimensions():
    with fits.open(TEST03_FITS_FILENAME_1) as fits_file:
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


def test03_fits_file_2_has_correct_hdu_dimensions():
    with fits.open(TEST03_FITS_FILENAME_2) as fits_file:
        # Visibilities
        for h in range(1, 5, 2):
            d = fits_file[h].data

            assert d.shape[0] == 3
            assert d.shape[1] == 16

        # Weights
        for h in range(2, 5, 2):
            d = fits_file[h].data

            assert d.shape[0] == 3
            assert d.shape[1] == 4


def test03_check_fits_file_1_hdu_values():
    for t in range(0, 2):
        data1 = read_fits_hdu(TEST03_FITS_FILENAME_1, 1 + (t * 4))
        assert 1128 == np.sum(data1)

        data2 = read_fits_hdu(TEST03_FITS_FILENAME_1, 2 + (t * 4))
        assert 1266 == np.sum(data2)

        data3 = read_fits_hdu(TEST03_FITS_FILENAME_1, 3 + (t * 4))
        assert 10728 == np.sum(data3)

        data4 = read_fits_hdu(TEST03_FITS_FILENAME_1, 4 + (t * 4))
        assert 3666 == np.sum(data4)


def test03_check_fits_file_2_hdu_values():
    data1 = read_fits_hdu(TEST03_FITS_FILENAME_2, 1)
    assert 1128 == np.sum(data1)

    data2 = read_fits_hdu(TEST03_FITS_FILENAME_2, 2)
    assert 1266 == np.sum(data2)

    data3 = read_fits_hdu(TEST03_FITS_FILENAME_2, 3)
    assert 10728 == np.sum(data3)

    data4 = read_fits_hdu(TEST03_FITS_FILENAME_2, 4)
    assert 3666 == np.sum(data4)

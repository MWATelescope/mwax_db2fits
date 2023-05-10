#
# Test02: Analyse output files and/or logs from this test of mwax_db2fits
#
from astropy.io import fits
from math import isclose
import numpy as np
import os
from tests_common import read_fits_hdu, count_fits_hdus

TEST02_OBS1_TMP_FITS_FILENAME = (
    "test02/1324440018_20211225040000_ch148_000.fits.tmp"
)
TEST02_OBS1_FITS_FILENAME = "test02/1324440018_20211225040000_ch148_000.fits"
TEST02_OBS2_FITS_FILENAME = "test02/1324440050_20211225040032_ch148_000.fits"


def test02_obs1_no_fits_file_produced():
    # Check a FITS file was not produced
    assert os.path.exists(TEST02_OBS1_TMP_FITS_FILENAME) is False
    assert os.path.exists(TEST02_OBS1_FITS_FILENAME) is False


def test02_obs2_fits_file_produced():
    # Check a FITS file was produced
    assert os.path.exists(TEST02_OBS2_FITS_FILENAME)


def test02_obs2_fits_file_has_correct_hdus():
    # Check the output fits file has 1 primary + 4 HDUs
    # 1 V + 1 W per timestep == 2 x 2 = 4 + primary == 5
    assert 5 == count_fits_hdus(TEST02_OBS2_FITS_FILENAME)


def test02_obs1_fits_file_has_correct_hdu_dimensions():
    with fits.open(TEST02_OBS2_FITS_FILENAME) as fits_file:
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


def test02_obs2_check_hdu_values():
    data1 = read_fits_hdu(TEST02_OBS2_FITS_FILENAME, 1)
    assert 5976 == np.sum(data1)
    weights1 = read_fits_hdu(TEST02_OBS2_FITS_FILENAME, 2)
    assert isclose(2.64, np.sum(weights1), rel_tol=1e-6)

    data2 = read_fits_hdu(TEST02_OBS2_FITS_FILENAME, 3)
    assert 10824 == np.sum(data2)
    weights2 = read_fits_hdu(TEST02_OBS2_FITS_FILENAME, 4)
    assert isclose(3.12, np.sum(weights2), rel_tol=1e-6)

#
# Test04: Analyse output files and/or logs from this test of mwax_db2fits
#
from astropy.io import fits
from math import isclose
import numpy as np
import os
from tests_common import (
    read_fits_hdu,
    count_fits_hdus,
    assert_substring_in_file,
)

test04_FITS_FILENAME = "test04/1324440018_20211225040000_ch148_000.fits"


def test04_fits_file_produced():
    # Check a FITS file was produced
    assert os.path.exists(test04_FITS_FILENAME)


def test04_fits_file_has_correct_hdus():
    # Check the output fits file has 1 primary + 8 HDUs
    # 1 V + 1 W per timestep == 4 x 2 = 8 + primary == 9
    assert 9 == count_fits_hdus(test04_FITS_FILENAME)


def test04_fits_file_has_correct_hdu_dimensions():
    with fits.open(test04_FITS_FILENAME) as fits_file:
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


def test04_check_hdu_values():
    data1 = read_fits_hdu(test04_FITS_FILENAME, 1)
    assert 5928 == np.sum(data1)
    weights1 = read_fits_hdu(test04_FITS_FILENAME, 2)
    assert isclose(3.3, np.sum(weights1), rel_tol=1e-6)

    data2 = read_fits_hdu(test04_FITS_FILENAME, 3)
    assert 10728 == np.sum(data2)
    weights2 = read_fits_hdu(test04_FITS_FILENAME, 4)
    assert isclose(3.9, np.sum(weights2), rel_tol=1e-6)

    data3 = read_fits_hdu(test04_FITS_FILENAME, 5)
    assert 15528 == np.sum(data3)
    weights3 = read_fits_hdu(test04_FITS_FILENAME, 6)
    assert isclose(4.5, np.sum(weights3), rel_tol=1e-6)

    data4 = read_fits_hdu(test04_FITS_FILENAME, 7)
    assert 20328 == np.sum(data4)
    weights4 = read_fits_hdu(test04_FITS_FILENAME, 8)
    assert isclose(5.1, np.sum(weights4), rel_tol=1e-6)


def test04_check_log_for_weights():
    #
    # NOTE this requires DEBUG build (build with ./build_debug.sh in root dir)
    # for the debug lines to be emitted
    #

    # since this is the first observation, the weights for each tile are x=0, y=0
    # so new weights getting cumulated are just those tile weights

    # timestep 1, tile 0
    assert_substring_in_file(
        "test04/mwax_db2fits.log",
        (
            "health_manager_set_weights_info(): counter = 1, tile = 0,"
            " weight.x = 0.000000, weight.y = 0.150000, cuml weight.x"
            " = 0.000000, cuml weight.y = 0.150000"
        ),
    )

    # timestep 1, tile 1
    assert_substring_in_file(
        "test04/mwax_db2fits.log",
        (
            "health_manager_set_weights_info(): counter = 1, tile = 1,"
            " weight.x = 0.400000, weight.y = 0.550000, cuml weight.x"
            " = 0.400000, cuml weight.y = 0.550000"
        ),
    )

    # now, if the interval between health packets still has not ticked,
    # we will be adding the new timesteps weights to the existing values
    # and incrementing our weights counter

    # timestep 2, tile 0
    assert_substring_in_file(
        "test04/mwax_db2fits.log",
        (
            "health_manager_set_weights_info(): counter = 2, tile = 0,"
            " weight.x = 0.050000, weight.y = 0.200000, cuml weight.x"
            " = 0.050000, cuml weight.y = 0.350000"
        ),
    )

    # timestep 2, tile 1
    assert_substring_in_file(
        "test04/mwax_db2fits.log",
        (
            "health_manager_set_weights_info(): counter = 2, tile = 1,"
            " weight.x = 0.450000, weight.y = 0.600000, cuml weight.x"
            " = 0.850000, cuml weight.y = 1.150000"
        ),
    )

    # timestep 3, tile 0
    assert_substring_in_file(
        "test04/mwax_db2fits.log",
        (
            "health_manager_set_weights_info(): counter = 3, tile = 0,"
            " weight.x = 0.100000, weight.y = 0.250000, cuml weight.x"
            " = 0.150000, cuml weight.y = 0.600000"
        ),
    )

    # timestep 3, tile 1
    assert_substring_in_file(
        "test04/mwax_db2fits.log",
        (
            "health_manager_set_weights_info(): counter = 3, tile = 1,"
            " weight.x = 0.500000, weight.y = 0.650000, cuml weight.x"
            " = 1.350000, cuml weight.y = 1.800000"
        ),
    )

    # timestep 4, tile 0
    assert_substring_in_file(
        "test04/mwax_db2fits.log",
        (
            "health_manager_set_weights_info(): counter = 4, tile = 0,"
            " weight.x = 0.150000, weight.y = 0.300000, cuml weight.x"
            " = 0.300000, cuml weight.y = 0.900000"
        ),
    )

    # timestep 4, tile 1
    assert_substring_in_file(
        "test04/mwax_db2fits.log",
        (
            "health_manager_set_weights_info(): counter = 4, tile = 1,"
            " weight.x = 0.550000, weight.y = 0.700000, cuml weight.x"
            " = 1.900000, cuml weight.y = 2.500000"
        ),
    )

import argparse
from pyuvdata import UVData
from astropy.time import Time
import numpy as np


class MWAData:
    def __init__(self, metafits_filename, gpubox_filename):
        self.metafits = metafits_filename
        self.gpubox = gpubox_filename
        self.UV = UVData()

        # Construct the list of files
        filelist = [metafits_filename, gpubox_filename]

        # Read data in
        self.UV.read_mwa_corr_fits(filelist, correct_cable_len=False, phase_data=False)

    def info(self):
        # show frequencies
        print(f"\nFrequencies: {self.UV.Nfreqs}")
        print(f"\nPolarisations: {self.UV.Npols}")

        # Show HDU's and times
        print(f"\nTimesteps: {self.UV.Ntimes}")
        i = 0
        for timestep_float64 in self.UV.get_times((0, 0)):
            t = Time(timestep_float64, format="jd")
            t.format = 'unix'
            print(f"hdu {i + 1} timestep {i} UNIX time {t}", end=" ")
            t.format = 'iso'
            print(f" Datetime {t}")
            i = i + 1

        # Show antenna names
        print(f"\nBaselines: {self.UV.Nbls}")
        print(f"\nAntennas:  {self.UV.Nants_data} / {self.UV.Nants_telescope}")
        i = 0
        for ant in self.UV._antenna_names.value:
            print(f"{i} = '{ant}'")
            i = i + 1

        print(self.UV.get_antpairs()[0],self.UV.get_antpairs()[128])

    def dump(self, timestep):
        XX = 0
        XY = 2
        YX = 3
        YY = 1

        t = self.UV.time_array[timestep]
        self.UV.select(times=[t])
        dump_data = self.UV.data_array

        n = self.UV.Nfreqs * self.UV.Nbls

        dump_data_2D = np.reshape(dump_data, (n, self.UV.Npols))

        with open("mwa_legacy.csv", "w") as dump_file:
            for row in dump_data_2D:
                dump_file.write("{0.real:f},{0.imag:f},"
                                "{1.real:f},{1.imag:f},"
                                "{2.real:f},{2.imag:f},"
                                "{3.real:f},{3.imag:f}\n".format(row[XX], row[XY], row[YX], row[YY]))

    def get_antenna_index_by_name(self, antenna_name):
        return self.UV._antenna_names.value.index(antenna_name)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-m", "--metafits_filename", required=False, help="metafits filename",
                        default="~/work/github/pyuvdata/data/mwa_corr_fits_testfiles/1131733552.metafits")
    parser.add_argument("-g", "--gpubox_filename", required=False, help="gpubox filename",
                        default="~/work/github/pyuvdata/data/mwa_corr_fits_testfiles/1131733552_20151116182537_mini_gpubox01_00.fits")
    parser.add_argument("-t", "--timestep", required=False, help="Timstep to dump", type=int)
    args = vars(parser.parse_args())

    metafits_file = args["metafits_filename"]
    gpubox_file = args["gpubox_filename"]

    timestep = args["timestep"]

    data = MWAData(metafits_file, gpubox_file)

    if timestep is None:
        print("No timestep provided, will show basic info only.")
        data.info()
    else:
        print(f"Requesting timestep {timestep} raw data")
        data.dump(timestep)

    print("Complete")

import argparse
from pyuvdata import UVData
from astropy.time import Time
import numpy as np
import math
import matplotlib.pyplot as plt

XX = 0
XY = 2
YX = 3
YY = 1


def jd_to_unix(jd):
    t_jd = Time(jd, format="jd")
    t_jd.format = 'unix'
    return t_jd


def jd_to_datetime(jd):
    t_jd = Time(jd, format="jd")
    t_jd.format = 'iso'
    return t_jd


class MWAData:
    def __init__(self, metafits_filename, gpubox_filename):
        self.metafits = metafits_filename
        self.gpubox = gpubox_filename
        self.UV = UVData()
        self.timestep_unix = None
        self.timestep_datetime = None

        # Construct the list of files
        filelist = [metafits_filename, gpubox_filename]

        # Read data in
        self.UV.read_mwa_corr_fits(filelist, correct_cable_len=False, phase_data=False,
                                   flag_init=False, edge_width=0, flag_dc_offset=False)

    def info(self):
        # show frequencies
        print(f"\nFrequencies: {self.UV.Nfreqs}")
        print(f"\nPolarisations: {self.UV.Npols}")

        # Show HDU's and times
        print(f"\nTimesteps: {self.UV.Ntimes}")
        i = 0
        for jd in self.UV.get_times((0, 0)):
            print(f"jd {jd} hdu {i + 1} timestep {i} UNIX time {jd_to_unix(jd)}", end=" ")
            print(f" Datetime {jd_to_datetime(jd)}")
            i = i + 1

        # Show antenna names
        print(f"\nBaselines: {self.UV.Nbls}")
        print(f"\nAntennas:  {self.UV.Nants_data} / {self.UV.Nants_telescope}")
        i = 0
        for ant in self.UV._antenna_names.value:
            print(f"{i} = '{ant}'")
            i = i + 1

    def get_data(self, timestep, baseline):
        # timestep is from t1 to tN or None
        # uv_data is in 4d - bl, time, freq, pol(r.i)
        if timestep is None:
            print("* All timesteps")
            jd = None
        else:
            unique_times = self.UV.get_times((0, 0, 'xx'))
            jd = unique_times[timestep - 1]  # This gets unique times for an antenna pair and polarisation
            self.timestep_unix = jd_to_unix(jd)
            self.timestep_datetime = jd_to_datetime(jd)

            print(f"Requesting raw data for:\n"
                  f"* JD {jd} / {self.timestep_unix} / {self.timestep_datetime}")

        if baseline is not None:
            print(f"* Baseline {baseline}", end=" ")
        else:
            print(f"* All antennas combined")

        if jd is None and baseline is None:
            pass
        else:
            self.UV.select(times=jd, bls=baseline)

        print(f"Data now has shape: {self.UV.data_array.shape}")

        return self.UV.data_array

    def dump(self, uv_data):
        n = self.UV.Nfreqs * self.UV.Nbls

        dump_data_2D = np.reshape(uv_data, (n, self.UV.Npols))

        with open("mwa_legacy.csv", "w") as dump_file:
            ch = 0

            for row in dump_data_2D:
                dump_file.write("ch {4:3} {0.real:>10.2f},{0.imag:>10.2f} | "
                                "{1.real:>10.2f},{1.imag:>10.2f} | "
                                "{2.real:>10.2f},{2.imag:>10.2f} | "
                                "{3.real:>10.2f},{3.imag:>10.2f}\n".format(row[XX], row[XY], row[YX], row[YY], ch))
                ch = ch + 1 % self.UV.Nfreqs

    def plot(self, uv_data, baseline, convert_to_db):
        # uv_data is in 4d - bl, time, freq, pol(r.i)
        # baseline = None for all or an index number
        # Produce X power and Y power
        # All baselines combined

        # Second dimension is PowerX, PowerY
        plot_array = np.zeros((self.UV.Nfreqs, 2))

        # Select the appropriate baseline (or leave as all baselines)
        for f in range(0, self.UV.Nfreqs):
            for bl in range(0, self.UV.Nbls):
                # X power
                plot_array[f, 0] += math.sqrt((uv_data[bl, 0, f, XX].real ** 2) +
                                              (uv_data[bl, 0, f, XX].imag ** 2))
                # Y power
                plot_array[f, 1] += math.sqrt((uv_data[bl, 0, f, YY].real ** 2) +
                                              (uv_data[bl, 0, f, YY].imag ** 2))

            # Convert to dB
            if convert_to_db:
                plot_array[f, 0] = (math.log10(plot_array[f, 0] + 1) * 10)
                plot_array[f, 1] = (math.log10(plot_array[f, 1] + 1) * 10)

        fig, ax = plt.subplots()

        baseline_text = "all baselines"
        if baseline:
            baseline_text = f"baseline: {baseline}"

        ax.set_title(f"PPD Legacy MWA ({baseline_text} at {self.timestep_unix}/{self.timestep_datetime})")
        ax.set_xlabel('Fine Channel')
        ax.set_ylabel('db')
        ax.plot(plot_array[:, 0], color='blue')
        ax.plot(plot_array[:, 1], color='green')
        plt.show()

    def get_antenna_indices_from_baseline(self, baseline):
        i = 0

        for a in range(0, self.UV.Nants_data):
            for b in range(a, self.UV.Nants_data):
                if i == baseline:
                    # Found
                    return (a, b)

                i = i + 1

        # Not found
        return None

    def get_antenna_index_by_name(self, antenna_name):
        return self.UV.antenna_names.value.index(antenna_name)

    def get_baseline_index_by_antennas(self, ant1, ant2):
        i = 0

        for a in range(0, self.UV.Nants_data):
            for b in range(a, self.UV.Nants_data):
                if a == ant1 and b == ant2:
                    # Found
                    return i

                i = i + 1

        # Not found
        return None


if __name__ == '__main__':
    print("MWA Legacy data reader")
    parser = argparse.ArgumentParser()
    parser.add_argument("-m", "--metafits_filename", required=False, help="metafits filename",
                        default="/home/gsleap/work/github/pyuvdata/pyuvdata/data/mwa_corr_fits_testfiles/1131733552.metafits")
    parser.add_argument("-g", "--gpubox_filename", required=False, help="gpubox filename",
                        default="/home/gsleap/work/github/pyuvdata/pyuvdata/data/mwa_corr_fits_testfiles/1131733552_20151116182537_mini_gpubox01_00.fits")
    parser.add_argument("-t", "--timestep", required=False, help="Timestep to select for plot or dump", type=int)
    parser.add_argument("-p", "--plot", required=False, help="Do a ppd plot", action='store_true')
    parser.add_argument("-d", "--dump", required=False, help="Dump data to a CSV file", action='store_true')
    parser.add_argument("-s", "--sum", required=False, help="Sum this timestep", action='store_true')
    parser.add_argument("-a1", "--antenna1", required=False,
                        help="Antenna1 to select for plot or dump (not valid with b)", type=int)
    parser.add_argument("-a2", "--antenna2", required=False,
                        help="Antenna2 to select for plot or dump (not valid with b)", type=int)
    parser.add_argument("-b", "--baseline", required=False,
                        help="Baseline to select for plot or dump (not valid with a1 and a2)", type=int)
    args = vars(parser.parse_args())

    arg_metafits_file = args["metafits_filename"]
    arg_gpubox_file = args["gpubox_filename"]

    arg_timestep = args["timestep"]
    arg_plot = args["plot"]
    arg_dump = args["dump"]
    arg_sum = args["sum"]
    arg_a1 = args["antenna1"]
    arg_a2 = args["antenna2"]
    arg_bl = args["baseline"]

    print("Initialising MWAData...", end="")
    data = MWAData(arg_metafits_file, arg_gpubox_file)
    print("...done!")

    # Get info no matter what
    print("Retrieving info...", end="")
    data.info()
    print("...done!")

    if arg_a1 is None and arg_a2 is None and arg_bl is not None:
        bl = data.get_antenna_indices_from_baseline(arg_bl)

    elif arg_a1 is not None and arg_a2 is not None and arg_bl is None:
        bl = (arg_a1, arg_a2)

    else:
        bl = None

    if bl is None and arg_timestep is None and arg_sum is True:
        print("Summing data only")
        dump_data = data.get_data(None, None)
    elif arg_timestep is None or (arg_plot is False and arg_dump is False and arg_sum is False):
        print("No timestep provided, will show basic info only.")
    else:
        print("Selecting specific data...", end="")
        dump_data = data.get_data(arg_timestep, bl)
        print("...done!")

        if arg_dump:
            print("Dumping to CSV file...")
            data.dump(dump_data)

        if arg_plot:
            print("Generating plot(s)...")
            #for i in range(0, 10):
            #    for j in range(i, 10):
            #        data.plot(dump_data, (i, j))
            data.plot(dump_data, bl, convert_to_db=True)

    if arg_sum:
        sum_of_data = dump_data.sum()

        print(f"Sum of data requested is: {sum_of_data.real + sum_of_data.imag}")

    print("Complete")

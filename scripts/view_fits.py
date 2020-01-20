from astropy.io import fits
import argparse
import matplotlib.pyplot as plt
import numpy as np
import math

dpi = 100
MODE_RANGE = "RANGE"
MODE_BASELINE = "BASELINE"


class ViewFITSArgs:
    def __init__(self, passed_args):
        self.filename = passed_args["filename"]

        self.time_step1 = passed_args["timestep1"]
        self.time_step2 = passed_args["timestep2"]

        self.tile1 = passed_args["ant1"]
        self.tile2 = passed_args["ant2"]

        self.autos_only = passed_args["autosonly"]

        self.channel1 = passed_args["channel1"]
        self.channel2 = passed_args["channel2"]

        self.ppd_plot = passed_args["ppdplot"]
        self.ppd_plot2 = passed_args["ppdplot2"]
        self.grid_plot = passed_args["gridplot"]
        self.phase_plot_all = passed_args["phaseplot_all"]
        self.phase_plot_one = passed_args["phaseplot_one"]

        self.weights = passed_args["weights"]
        self.mode = passed_args["mode"]

        # Are we plotting?
        self.any_plotting = (self.ppd_plot or self.ppd_plot2 or self.grid_plot or
                             self.phase_plot_all or self.phase_plot_one)

        # Some constants not found in fits file
        self.values = 2     # real and imaginary
        self.pols = 4       # xx,xy,yx,yy

        # Read fits file
        print(f"Opening fits file {self.filename}...")
        self.fits_hdu_list = fits.open(self.filename)

        # in v2 NAXIS1 == fine channels * pols * r,i
        # in v2 NAXIS2 == baselines
        # Get number of tiles based on the number of signal chains
        self.fits_tiles = int(self.fits_hdu_list[0].header["NINPUTS"] / 2)
        chan_x_pols_x_vals = self.fits_hdu_list[1].header["NAXIS1"]
        self.fits_channels = int((chan_x_pols_x_vals / self.pols) / self.values)
        self.fits_has_weights = True

        # Check mode
        if self.mode != MODE_RANGE and self.mode != MODE_BASELINE:
            print(f"Error, mode should be {MODE_RANGE} or {MODE_BASELINE}!")
            exit(-1)

        # Check tiles
        if self.tile1 == -1:
            self.tile1 = 0

        if self.tile2 == -1:
            self.tile2 = self.fits_tiles - 1

        if self.tile1 > self.fits_tiles - 1:
            print("Error ant1 is more than the last tile index!")
            exit(-1)

        if self.tile2 > self.fits_tiles - 1:
            print("Error ant2 is more than the last tile index!")
            exit(-1)

        if self.tile1 > self.tile2:
            print("Error ant1 is more than the ant2!")
            exit(-1)

        # Check time steps
        if self.fits_has_weights:
            # Count = hdus minus primary divided by 2 (we have data then weights)
            self.fits_time_steps = int((len(self.fits_hdu_list) - 1) / 2)
        else:
            self.fits_time_steps = len(self.fits_hdu_list) - 1

        if self.time_step1 == -1:
            self.time_step1 = 1

        if self.time_step2 == -1:
            self.time_step2 = self.fits_time_steps

        if self.time_step1 > self.fits_time_steps:
            print("Error t1 is more than the max time step!")
            exit(-1)

        if self.time_step2 > self.fits_time_steps:
            print("Error t2 is more than the max time step!")
            exit(-1)

        if self.time_step1 > self.time_step2:
            print("Error t1 is more than the t2!")
            exit(-1)

        # Check channels
        if self.channel1 == -1:
            self.channel1 = 0

        if self.channel2 == -1:
            self.channel2 = self.fits_channels - 1

        if self.channel1 > self.fits_channels - 1:
            print("Error c1 is more than the number of the last fine channel!")
            exit(-1)

        if self.channel2 > self.fits_channels - 1:
            print("Error c2 is more than the number of the last fine channel!")
            exit(-1)

        if self.channel1 > self.channel2:
            print("Error c1 is more than the c2!")
            exit(-1)

        # Some calculated fields
        self.tile_count = self.tile2 - self.tile1 + 1

        if self.mode == MODE_BASELINE:
            self.baseline_count = 1
        else:
            self.baseline_count = int((self.tile_count * (self.tile_count + 1)) / 2)

        self.fits_baseline_count = int((self.fits_tiles * (self.fits_tiles + 1)) / 2)
        self.channel_count = self.channel2 - self.channel1 + 1
        self.time_step_count = self.time_step2 - self.time_step1 + 1

        self.hdu_time1 = (self.time_step1 * 2) - 1  # e.g. t2 = 2*2 -1 = 3
        self.unix_time1 = self.fits_hdu_list[self.hdu_time1].header["TIME"] + \
                          (self.fits_hdu_list[self.hdu_time1].header["MILLITIM"] / 1000)

        self.hdu_time2 = (self.time_step2 * 2) - 1
        self.unix_time2 = self.fits_hdu_list[self.hdu_time2].header["TIME"] + \
                          (self.fits_hdu_list[self.hdu_time2].header["MILLITIM"] / 1000)

        # print params
        self.param_string = f"{self.filename} t={self.time_step1}-{self.time_step2} " \
                            f"t_hdu={self.hdu_time1}-{self.hdu_time2} \n" \
                            f"t_unix={self.unix_time1}-{self.unix_time2} " \
                            f"tile={self.tile1}-{self.tile2} " \
                            f"ch={self.channel1}-{self.channel2} autosonly?={self.autos_only} " \
                            f"{self.tile_count}t/{self.fits_tiles}t"
        print(self.param_string)


def meets_criteria(i, j, a1, a2, mode):
    # mode == RANGE | BASELINE
    if mode == MODE_RANGE:
        return i >= a1 and j <= a2
    elif mode == MODE_BASELINE:
        return i == a1 and j == a2
    else:
        return None


# v1 = baseline, freq, pol
# v2 = freq,baseline,pol
def peek_fits(program_args: ViewFITSArgs):
    print("Initialising data structures...")

    # ppd array will be [timestep][channel]
    plot_ppd_data_x = np.empty(shape=(program_args.time_step_count, program_args.channel_count))
    plot_ppd_data_x.fill(0)
    plot_ppd_data_y = np.empty(shape=(program_args.time_step_count, program_args.channel_count))
    plot_ppd_data_y.fill(0)

    # ppd plot 2 array will be [timestep][baseline][channel]
    plot_ppd2_data_x = np.empty(shape=(program_args.time_step_count, program_args.baseline_count, program_args.channel_count))
    plot_ppd2_data_x.fill(0)
    plot_ppd2_data_y = np.empty(shape=(program_args.time_step_count, program_args.baseline_count, program_args.channel_count))
    plot_ppd2_data_y.fill(0)

    # Grid plot
    plot_grid_data = np.empty(shape=(program_args.time_step_count, program_args.tile_count, program_args.tile_count))
    plot_grid_data.fill(0)

    # Phase plot
    plot_phase_data_x = np.empty(shape=(program_args.time_step_count, program_args.baseline_count, program_args.channel_count))
    plot_phase_data_x.fill(0)
    plot_phase_data_y = np.empty(shape=(program_args.time_step_count, program_args.baseline_count, program_args.channel_count))
    plot_phase_data_y.fill(0)

    # create a list of the hdus we want
    time_step_list = []
    weight_list = []

    # hdu 0 is primary
    if program_args.fits_has_weights:
        # hdu's then alternate between data and weghts. e.g. d,w,d,w, etc
        for h in range((program_args.time_step1 * 2)-1, ((program_args.time_step2 + 1) * 2) - 1, 2):
            print(f"Adding HDU {h} to data HDUs")
            time_step_list.append(program_args.fits_hdu_list[h])

        for h_weights in range(program_args.time_step1 * 2, (program_args.time_step2 + 1) * 2, 2):
            print(f"Adding HDU {h_weights} to weights HDUs")
            weight_list.append(program_args.fits_hdu_list[h_weights])
    else:
        for h in range(program_args.time_step1, (program_args.time_step2 + 1)):
            print(f"Adding HDU {h} to data HDUs")
            time_step_list.append(program_args.fits_hdu_list[h])

    # print the header of each HDU, including the primary
    print("Primary HDU:\n")
    print(repr(program_args.fits_hdu_list[0].header))

    # print a csv header if we're not plotting
    if not program_args.any_plotting:
        #print("time,baseline,chan,ant1,ant2, xx_r, xx_i, xy_r, xy_i,yx_r, yx_i, yy_r, yy_i, power_x, power_y")
        print("xx_r, xx_i, xy_r, xy_i,yx_r, yx_i, yy_r, yy_i")

    time_index = 0

    if not program_args.any_plotting:
        dump_file = open("mwax_dump.csv", "w")

    for hdu in time_step_list:
        time = hdu.header["MARKER"] + 1
        print(f"Processing timestep: {time} (time index: {time_index})...")

        data = np.array(hdu.data, dtype=float)

        baseline = 0
        selected_baseline = 0

        #
        # Correlator v2
        #
        print("HDU header for this timestep:")
        print(repr(hdu.header))

        # was:
        # for i in range(0, program_args.tile2 + 1):
        #   for j in range(i, program_args.fits_tiles):
        for i in range(0, program_args.tile2 + 1):
            for j in range(i, program_args.fits_tiles):
                # Explaining this if:
                # Line 1. Check for autos if that's what we asked for
                # Line 2. OR just be True if we didn't ask for autos only.
                # Line 3 (Applicable to cases 1 and 2 above): Check the selected tile1 and tile2 are in range
                if ((i == j and program_args.autos_only is True) or
                   (program_args.autos_only is False)) and \
                   (meets_criteria(i, j, program_args.tile1, program_args.tile2, program_args.mode)):
                    print(f"t {time}, baseline {baseline}, {i}, {j}")

                    for chan in range(program_args.channel1, program_args.channel2 + 1):
                        index = chan * (program_args.pols * program_args.values)

                        xx_r = data[baseline][index]
                        xx_i = data[baseline][index + 1]

                        xy_r = data[baseline][index + 2]
                        xy_i = data[baseline][index + 3]

                        yx_r = data[baseline][index + 4]
                        yx_i = data[baseline][index + 5]

                        yy_r = data[baseline][index + 6]
                        yy_i = data[baseline][index + 7]

                        power_x = math.sqrt(xx_i*xx_i + xx_r*xx_r)
                        power_y = math.sqrt(yy_i*yy_i + yy_r*yy_r)

                        if program_args.ppd_plot:
                            plot_ppd_data_x[time_index][chan] = plot_ppd_data_x[time_index][chan] + power_x
                            plot_ppd_data_y[time_index][chan] = plot_ppd_data_y[time_index][chan] + power_y

                        elif program_args.ppd_plot2:
                            plot_ppd2_data_x[time_index][selected_baseline][chan] = power_x
                            plot_ppd2_data_y[time_index][selected_baseline][chan] = power_y

                        elif program_args.grid_plot:
                            plot_grid_data[time_index][j][i] = plot_grid_data[time_index][j][i] + power_x + power_y

                        elif program_args.phase_plot_all or program_args.phase_plot_one:
                            phase_x = math.degrees(math.atan2(xx_i, xx_r))
                            phase_y = math.degrees(math.atan2(yy_i, yy_r))

                            plot_phase_data_x[time_index][selected_baseline][chan] = phase_x
                            plot_phase_data_y[time_index][selected_baseline][chan] = phase_y

                        else:
                            if not program_args.weights and not program_args.any_plotting:
                                #print(
                                #    "{0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13},{14}".format(
                                #        time, baseline, chan, i, j, xx_r, xx_i, xy_r, xy_i, yx_r, yx_i, yy_r, yy_i,
                                #       power_x, power_y))
                                dump_file.write(
                                    "{0},{1},{2},{3},{4},{5},{6},{7}\n".format(
                                        xx_r, xx_i, xy_r, xy_i, yx_r, yx_i, yy_r, yy_i,))

                    selected_baseline = selected_baseline + 1

                baseline = baseline + 1

        time_index = time_index + 1
        print(" done!")

    if not program_args.any_plotting:
        dump_file.close()

    print("Processing weights...", end="")
    for hdu in weight_list:
        time = hdu.header["MARKER"] + 1
        baseline = 0
        for i in range(program_args.tile1, program_args.tile2 + 1):
            for j in range(i, program_args.tile2 + 1):
                if ((i == j and program_args.autos_only is True)
                        or (program_args.autos_only is False)):
                    w_xx = hdu.data[baseline][0]
                    w_xy = hdu.data[baseline][1]
                    w_yx = hdu.data[baseline][2]
                    w_yy = hdu.data[baseline][3]

                    if program_args.weights:
                        print(f"{time} {baseline} {i} v {j}: {w_xx}, {w_xy}, {w_yx}, {w_yy}")

                baseline = baseline + 1

    # clean up
    print("Done.\nClosing fits file")
    program_args.fits_hdu_list.close()

    if program_args.ppd_plot:
        convert_to_db = True
        do_ppd_plot(program_args.param_string, program_args, plot_ppd_data_x, plot_ppd_data_y, convert_to_db)

    if program_args.ppd_plot2:
        convert_to_db = True
        do_ppd_plot2(program_args.param_string, program_args, plot_ppd2_data_x, plot_ppd2_data_y, convert_to_db)

    if program_args.grid_plot:
        do_grid_plot(program_args.param_string, program_args, plot_grid_data)

    if program_args.phase_plot_all or program_args.phase_plot_one:
        do_phase_plot(program_args.param_string, program_args, plot_phase_data_x, plot_phase_data_y)

    print("Done!\n")


def do_ppd_plot(title, program_args: ViewFITSArgs, plot_ppd_data_x, plot_ppd_data_y, convert_to_db):
    print("Preparing ppd plot...")

    # Work out layout of plots
    plots = program_args.time_step_count
    plot_rows = math.floor(math.sqrt(plots))
    plot_cols = math.ceil(plots / plot_rows)
    plot_row = 0
    plot_col = 0
    min_db = 0

    # Convert to a dB figure
    if convert_to_db:
        for t in range(0, program_args.time_step_count):
            for c in range(0, program_args.channel_count):
                plot_ppd_data_x[t][c] = math.log10(plot_ppd_data_x[t][c] + 1) * 10
                plot_ppd_data_y[t][c] = math.log10(plot_ppd_data_y[t][c] + 1) * 10

        # Get min dB value
        min_db = 0 # min(np.array(plot_ppd_data_x).min(), np.array(plot_ppd_data_y).min())

    fig, ax = plt.subplots(nrows=plot_rows, ncols=plot_cols, squeeze=False, sharey="all", dpi=dpi)
    fig.suptitle(title)

    for t in range(0, program_args.time_step_count):
        print(f"Adding data points for plot({t})...")

        # Step down the dB by the min so we have a 0 base
        for c in range(0, program_args.channel_count):
            plot_ppd_data_x[t][c] = plot_ppd_data_x[t][c] - min_db
            plot_ppd_data_y[t][c] = plot_ppd_data_y[t][c] - min_db

        # Get the current plot
        plot = ax[plot_row][plot_col]

        # Draw this plot
        plot.plot(plot_ppd_data_x[t], color='blue')
        plot.plot(plot_ppd_data_y[t], color='green')

        # Set labels
        if convert_to_db:
            plot.set_ylabel("dB", size=6)
        else:
            plot.set_ylabel("Raw value", size=6)

        plot.set_xlabel("fine channel", size=6)

        # Set plot title
        plot.set_title(f"t={t + program_args.unix_time1}", size=6)

        # Increment so we know which plot we are on
        if plot_col < plot_cols - 1:
            plot_col = plot_col + 1
        else:
            plot_row = plot_row + 1
            plot_col = 0

    print("Saving figure...")

    # Save the final plot to disk
    plt.savefig("ppd_plot.png", bbox_inches='tight', dpi=dpi)
    print("saved ppd_plot.png")
    plt.show()


def do_ppd_plot2(title, program_args: ViewFITSArgs, plot_ppd_data_x, plot_ppd_data_y, convert_to_db):
    print("Preparing ppd plot2...")

    # Work out layout of plots
    plots = program_args.time_step_count
    plot_rows = math.floor(math.sqrt(plots))
    plot_cols = math.ceil(plots / plot_rows)
    plot_row = 0
    plot_col = 0
    min_db = 0

    # Convert to a dB figure
    if convert_to_db:
        for t in range(0, program_args.time_step_count):
            for c in range(0, program_args.channel_count):
                for b in range(0, program_args.baseline_count):
                    plot_ppd_data_x[t][b][c] = math.log10(plot_ppd_data_x[t][b][c] + 1) * 10
                    plot_ppd_data_y[t][b][c] = math.log10(plot_ppd_data_y[t][b][c] + 1) * 10

        # Get min dB value
        min_db = min(np.array(plot_ppd_data_x).min(), np.array(plot_ppd_data_y).min())

    fig, ax = plt.subplots(nrows=plot_rows, ncols=plot_cols, squeeze=False, sharey="all", dpi=dpi)
    fig.suptitle(title)

    for t in range(0, program_args.time_step_count):
        print(f"Adding data points for plot({t})...")

        # Step down the dB by the min so we have a 0 base
        if min_db != 0:
            for c in range(0, program_args.channel_count):
                for b in range(0, program_args.baseline_count):
                    plot_ppd_data_x[t][b][c] = plot_ppd_data_x[t][b][c] - min_db
                    plot_ppd_data_y[t][b][c] = plot_ppd_data_y[t][b][c] - min_db

        # Get the current plot
        plot = ax[plot_row][plot_col]

        # Draw this plot
        for b in range(0, program_args.baseline_count):
            plot.plot(plot_ppd_data_x[t][b], 'o', markersize=1, color='blue')
            plot.plot(plot_ppd_data_y[t][b], 'o', markersize=1, color='green')

        # Set labels
        if convert_to_db:
            plot.set_ylabel("dB", size=6)
        else:
            plot.set_ylabel("Raw value", size=6)

        plot.set_xlabel("fine channel", size=6)

        # Set plot title
        plot.set_title(f"t={t + program_args.time_step1}", size=6)

        # Increment so we know which plot we are on
        if plot_col < plot_cols - 1:
            plot_col = plot_col + 1
        else:
            plot_row = plot_row + 1
            plot_col = 0

    print("Saving figure...")

    # Save the final plot to disk
    plt.savefig("ppd_plot2.png", bbox_inches='tight', dpi=dpi)
    print("saved ppd_plot2.png")
    plt.show()


def do_grid_plot(title, program_args: ViewFITSArgs, plot_grid_data):
    print("Preparing grid plot...")

    # Work out layout of plots
    plots = program_args.time_step_count
    plot_rows = math.floor(math.sqrt(plots))
    plot_cols = math.ceil(plots / plot_rows)
    plot_row = 0
    plot_col = 0

    if 0 == 1:
        for time_index in range(0, program_args.time_step_count):
            for t1 in range(0, program_args.tile_count):
                for t2 in range(t1, program_args.tile_count):
                    plot_grid_data[time_index][t2][t1] = math.log10(plot_grid_data[time_index][t2][t1] + 1) * 10

        # Get min dB value
        np_array_nonzero = plot_grid_data[plot_grid_data > 1]
        min_db = np_array_nonzero.min()

        # Apply min_db but only to values > 1
        plot_grid_data = np.where(plot_grid_data > 1, plot_grid_data - min_db, plot_grid_data)

    fig, ax = plt.subplots(nrows=plot_rows, ncols=plot_cols, squeeze=False, sharex="all", sharey="all", dpi=dpi)
    fig.suptitle(title)

    n_step = math.ceil(plots / 1.25)

    if n_step % 2 != 0:
        n_step = n_step + 1

    if n_step % 4 != 0:
        n_step = n_step - 2

    if n_step <= 1:
        n_step = 2
    else:
        n_step = n_step * 2 

    if n_step > 16:
        n_step = 16

    for time_index in range(0, program_args.time_step_count):
        for t1 in range(0, program_args.tile_count):
            for t2 in range(t1, program_args.tile_count):
                print(time_index, program_args.tile1 + t1, program_args.tile1 + t2, plot_grid_data[time_index][t2][t1])

        print(f"Adding data points for plot({time_index})...")

        # Get the current plot
        plot = ax[plot_row][plot_col]

        plot.imshow(plot_grid_data[time_index], cmap="inferno", interpolation="None")

        plot.set_title(f"t={time_index+program_args.time_step1}", size=6)
        plot.set_xticks(np.arange(program_args.tile_count, step=n_step))
        plot.set_yticks(np.arange(program_args.tile_count, step=n_step))
        plot.set_xticklabels(np.arange(program_args.tile1, program_args.tile2 + 1, step=n_step))
        plot.set_yticklabels(np.arange(program_args.tile1, program_args.tile2 + 1, step=n_step))

        # Set labels
        # Only do y label for first col
        if plot_col == 0:
            plot.set_ylabel("ant2", size=6)

        # Only do x label for final row
        if plot_row == plot_rows - 1:
            plot.set_xlabel("ant1", size=6)

        plt.setp(plot.get_xticklabels(), rotation=90, ha="right", rotation_mode="anchor")

        # Increment so we know which plot we are on
        if plot_col < plot_cols - 1:
            plot_col = plot_col + 1
        else:
            plot_row = plot_row + 1
            plot_col = 0

    print("Saving figure...")
    plt.savefig("grid_plot.png", bbox_inches='tight', dpi=dpi)
    print("saved grid_plot.png")
    plt.show()


def do_phase_plot(title, program_args: ViewFITSArgs, plot_phase_data_x, plot_phase_data_y):
    print("Preparing phase plot...")
    print(f"Timesteps: {program_args.time_step_count}, tiles: {program_args.tile_count}, channels: {program_args.channel_count}")

    # Work out layout of plots 
    if program_args.phase_plot_one:
        plots = 1
    else:
        plots = (program_args.tile2 - program_args.tile1) + 1

    plot_rows = math.floor(math.sqrt(plots))
    plot_cols = math.ceil(plots / plot_rows)
    plot_row = 0
    plot_col = 0
    baseline = 0

    fig, ax = plt.subplots(nrows=plot_rows, ncols=plot_cols, squeeze=False, sharex="all", sharey="all", dpi=dpi)
    fig.suptitle(title)

    for i in range(0, program_args.tile_count):
        for j in range(i, program_args.tile_count):
            if program_args.phase_plot_one:
               print(f"{i} vs {j}")
               if not (i == 0 and j == (program_args.tile_count - 1)):
                   # skip this plot 
                   print("skip") 
                   baseline = baseline + 1
                   continue
            else:
               if i != program_args.tile1:
                   print("skip")
                   continue

            print(f"Adding data points for plot({i},{j})...")
            channel_list = range(program_args.channel1, program_args.channel2 + 1)

            # Get the current plot
            plot = ax[plot_row][plot_col]

            # Do plots
            for t in range(0, program_args.time_step_count):
                plot.plot(channel_list, plot_phase_data_x[t][baseline], 'o', markersize=3, color='blue')
                plot.plot(channel_list, plot_phase_data_y[t][baseline], 'o', markersize=3, color='green')

            # Set labels
            # Only do y label for first col
            if plot_col == 0:
                plot.set_ylabel("phase (deg)", size=6)

            # Only do x label for final row
            if plot_row == plot_rows - 1:
                plot.set_xlabel("fine channel", size=6)

            # Ensure Y axis goes from -180 to 180
            plot.set_ylim([-200, 200])

            plot.set_title(f"{i + program_args.tile1} v {j + program_args.tile1}", size=6)

            # Increment so we know which plot we are on
            if plot_col < plot_cols - 1:
                plot_col = plot_col + 1
            else:
                plot_row = plot_row + 1
                plot_col = 0

            # Increment baseline
            baseline = baseline + 1

    print("Saving figure...")
    # Save final plot to disk
    plt.savefig("phase_plot.png", bbox_inches='tight', dpi=dpi)
    print("saved phase_plot.png")
    plt.show()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("filename", help="fits filename")
    parser.add_argument("-t1", "--timestep1", required=False, help="timestep start (1 based index)",
                        default=1, type=int)
    parser.add_argument("-t2", "--timestep2", required=False, help="timestep end (defaults to last index)",
                        default=-1, type=int)
    parser.add_argument("-a1", "--ant1", required=False, help="antenna (start)",
                        default=-1, type=int)
    parser.add_argument("-a2", "--ant2", required=False, help="antenna (end)",
                        default=-1, type=int)
    parser.add_argument("-c1", "--channel1", required=False, help="fine channel number (start)",
                        default=-1, type=int)
    parser.add_argument("-c2", "--channel2", required=False, help="fine channel number (end)",
                        default=-1, type=int)
    parser.add_argument("-a", "--autosonly", required=False, help="Only output the auto correlations",
                        action='store_true')
    parser.add_argument("-p", "--ppdplot", required=False, help="Create a ppd plot",
                        action='store_true')
    parser.add_argument("-p2", "--ppdplot2", required=False, help="Create a ppd plot that does not sum across all baselines. ie it plots all baselines",
                        action='store_true')
    parser.add_argument("-g", "--gridplot", required=False, help="Create a grid / baseline plot",
                        action='store_true')
    parser.add_argument("-ph", "--phaseplot_all", required=False, help="Will do a phase plot for all baselines for given antennas and timesteps",
                        action='store_true')

    parser.add_argument("-ph1", "--phaseplot_one", required=False, help="Will do a phase plot for given baseline and timesteps",
                        action='store_true')

    parser.add_argument("-m", "--mode", required=True, help="How to interpret a1 and a2: RANGE or BASELINE")

    parser.add_argument("-w", "--weights", required=False, help="Dump the weights", action='store_true')
    args = vars(parser.parse_args())

    parsed_args = ViewFITSArgs(args)

    peek_fits(parsed_args)

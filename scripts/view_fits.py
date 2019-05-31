from astropy.io import fits
import argparse
import matplotlib.pyplot as plt
import numpy as np
import math


# freq,baseline,pol
def peek_fits(filename, timestep1, timestep2, ant1, ant2, channel, autosonly, ppdplot, gridplot, phaseplot, maxtiles, weights):
    # constants
    values = 2 # real and imaginary
    pols = 4   # xx,xy,yx,yy

    fits_hdu_list = fits.open(filename)

    # Get number of tiles based on baseline count
    input_data_tiles = int(fits_hdu_list[0].header["NINPUTS"]/2)

    print("Primary HDU:")
    print(repr(fits_hdu_list[0].header))
    print("\n\nFirst image/visibilituy HDU:")
    print(repr(fits_hdu_list[1].header))
    print("\n\nFirst image/weights HDU:")
    print(repr(fits_hdu_list[2].header))
    print("\n")

    # Get fine channel count
    chan_x_pols_x_vals = fits_hdu_list[1].header["NAXIS1"]
    channels = int((chan_x_pols_x_vals / pols) / values)

    # Are we plotting?
    plot = (ppdplot or gridplot or phaseplot)

    # Sanitise input args
    ant1 = int(ant1)
    ant2 = int(ant2)
    channel = int(channel)
    ts1 = int(timestep1)
    ts2 = int(timestep2)
    max_tiles = int(maxtiles)

    if ts2 == -1:
        ts2 = int((len(fits_hdu_list) - 1) / 2)

    if max_tiles == -1:
        max_tiles = input_data_tiles

    # print params
    title = "{0} timesteps= {1} to {2} ant1= {3} ant2= {4} chan= {5} autosonly?= {6} maxtiles= {7} / {8}".format(
        filename, ts1, ts2, ant1, ant2, channel, autosonly, max_tiles, input_data_tiles)
    print(title)

    # basic error check
    valid = True
    valid_msg = ""

    if valid and ts1 > ts2:
        valid = False
        valid_msg = "timestep1 must be less than or equal to timestep2"

    if valid and ts1 < 1:
        valid = False
        valid_msg = "timestep1 must be greater than or equal to 1"

    if valid and ant1 > ant2:
        valid = False
        valid_msg = "ant1 must be less than or equal to ant2"

    if valid and channel > channels:
        valid = False
        valid_msg = "channel must be between 0 and {0}".format(channels)

    if valid and phaseplot and (channel != -1 or ant1 == -1 or ant2 == -1 or ts2 - ts1 != 0):
        valid = False
        valid_msg = "phaseplot requires 1 baseline, 1 timestep and all fine channels"

    if valid and max_tiles > input_data_tiles:
        valid = False
        valid_msg = "maxtiles cannot be larger than input tiles {0}".format(input_data_tiles)

    if not valid:
        print("Error: {0}".format(valid_msg))
        exit(-1)

    # initialise the bins for out plot
    plot_ppd_data = [0] * channels

    plot_grid_data = np.empty(shape=(max_tiles, max_tiles))
    plot_grid_data.fill(0)

    plot_phase_data_x = [0] * channels
    plot_phase_data_y = [0] * channels

    # create a list of the hdus we want
    hdu_list = []
    hdu_list_weights = []

    # hdu 0 is primary
    # hdu's then alternate between data and weghts. e.g. d,w,d,w, etc
    for h in range((ts1*2)-1, ((ts2+1)*2)-1, 2):
        print(f"Adding HDU {h} to data HDUs")
        hdu_list.append(fits_hdu_list[h])

    for h_weights in range(ts1*2, (ts2+1)*2, 2):
        print(f"Adding HDU {h_weights} to weights HDUs")
        hdu_list_weights.append(fits_hdu_list[h_weights])

    # print a csv header if we're not plotting
    if not plot:
        print("time,baseline,chan,ant1,ant2, xx_r, xx_i, xy_r, xy_i,yx_r, yx_i, yy_r, yy_i, power")

    for hdu in hdu_list:
        time = hdu.header["MARKER"] + 1
        baseline = 0

        for i in range(0, input_data_tiles):
            for j in range(i, input_data_tiles):
                if (i < max_tiles and j < max_tiles) and ((i == j and autosonly == True) or (
                        autosonly == False and ((ant1 == -1 or ant1 == i) and (ant2 == -1 or ant2 == j)))):
                    for chan in range(0, channels):
                        if chan == channel or channel == -1:
                            index = chan * (pols * 2)

                            xx_r = hdu.data[baseline][index]
                            xx_i = hdu.data[baseline][index + 1]

                            xy_r = hdu.data[baseline][index + 2]
                            xy_i = hdu.data[baseline][index + 3]

                            yx_r = hdu.data[baseline][index + 4]
                            yx_i = hdu.data[baseline][index + 5]

                            yy_r = hdu.data[baseline][index + 6]
                            yy_i = hdu.data[baseline][index + 7]

                            power = (xx_r * xx_r) + (yy_r * yy_r) + (xx_i * xx_i) + (yy_i * yy_i)
                            phase_x = math.degrees(math.atan2(xx_i, xx_r))
                            phase_y = math.degrees(math.atan2(yy_i, yy_r))

                            plot_ppd_data[chan] = plot_ppd_data[chan] + power
                            plot_grid_data[j][i] = plot_grid_data[j][i] + power
                            plot_phase_data_x[chan] = phase_x
                            plot_phase_data_y[chan] = phase_y

                            if not plot and not weights:
                                print(
                                    "{0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13}".format(
                                        time, baseline, chan, i, j, xx_r, xx_i, xy_r, xy_i, yx_r, yx_i, yy_r, yy_i,
                                        power))

                baseline = baseline + 1

    for hdu in hdu_list_weights:
        time = hdu.header["MARKER"] + 1
        baseline = 0
        for i in range(0, input_data_tiles):
            for j in range(i, input_data_tiles):
                if (i < max_tiles and j < max_tiles) and ((i == j and autosonly == True) or (
                        autosonly == False and ((ant1 == -1 or ant1 == i) and (ant2 == -1 or ant2 == j)))):
                    w_xx = hdu.data[baseline][0]
                    w_xy = hdu.data[baseline][1]
                    w_yx = hdu.data[baseline][2]
                    w_yy = hdu.data[baseline][3]

                    if weights:
                        print(f"{time} {baseline} {i} v {j}: {w_xx}, {w_xy}, {w_yx}, {w_yy}")

                baseline = baseline + 1

    # clean up
    fits_hdu_list.close()

    if ppdplot:
        do_ppd_plot(title, channels, plot_ppd_data)

    if gridplot:
        do_grid_plot(title, max_tiles, plot_grid_data)

    if phaseplot:
        do_phase_plot(title, channels, plot_phase_data_x, plot_phase_data_y)

    print("Done!\n")


def do_ppd_plot(title, channels, plot_ppd_data):
    for c in range(0, channels):
        plot_ppd_data[c] = math.log10(plot_ppd_data[c] + 1) * 10
        print(c, plot_ppd_data[c])

    plt.plot(plot_ppd_data)
    plt.ylabel("dB")
    plt.xlabel("fine channel")

    nstep = int(channels / 20)
    if nstep < 1:
        nstep = 1

    plt.xticks(np.arange(0, channels, step=nstep))
    plt.title(title)
    #plt.tight_layout()
    plt.grid(True)
    plt.savefig("ppd_plot.png", bbox_inches='tight')
    print("saved ppd_plot.png")
    plt.show()


def do_grid_plot(title, tiles, plot_grid_data):
    for t1 in range(0, tiles):
        for t2 in range(t1, tiles):
            plot_grid_data[t2][t1] = math.log10(plot_grid_data[t2][t1] + 1) * 10
            print(t1, t2, plot_grid_data[t2][t1])

    plt.ylabel("ant2")
    plt.xlabel("ant1")

    nstep = int(tiles / 16)
    if nstep < 1:
        nstep = 1

    #plt.xticks(np.arange(0.5, tiles, step=nstep), np.arange(0, tiles, step=nstep))
    #plt.yticks(np.arange(0.5, tiles, step=nstep), np.arange(0, tiles, step=nstep))

    ax = plt.gca()
    tickpos = np.arange(0, tiles) + 0.5   # eg [0.5, 1.5, 2.5, ...]
    tickstrings = [str(x) for x in np.arange(0, tiles)]
    gridvals = np.arange(0, tiles)

    ax.set_xticks(gridvals, minor=True)   # define the minor tick coords
    ax.set_yticks(gridvals, minor=True)

    ax.set_xticks(tickpos, minor=False)   # define the major tick coords
    ax.set_yticks(tickpos, minor=False)

    ax.set_xticklabels(tickstrings, minor=True)  # strings to display there
    ax.set_yticklabels(tickstrings, minor=True)
    ax.set_xticklabels([], minor=False)  # strings to display there
    ax.set_yticklabels([], minor=False)

    ax.grid(b=False, which='minor', axis='both')  # Display grid at the minor tickmarks
    ax.grid(b=True, which='major', axis='both')

    plt.title(title)
    #plt.tight_layout()
    plt.imshow(plot_grid_data, cmap="inferno", interpolation="nearest")
    plt.savefig("grid_plot.png", bbox_inches='tight')
    print("saved grid_plot.png")
    plt.show()


def do_phase_plot(title, channels, plot_phase_data_x, plot_phase_data_y):
    for c in range(0, channels):
        print(c, plot_phase_data_x[c], plot_phase_data_y[c])

    channel_list = range(0, channels)

    plt.scatter(channel_list, plot_phase_data_x)
    plt.scatter(channel_list, plot_phase_data_y)

    plt.ylabel("phase (deg)")
    plt.xlabel("fine channel")
    
    nstep = int(channels / 20)
    if nstep < 1:
        nstep = 1 

    plt.xticks(np.arange(0, channels, step=nstep))
    plt.yticks(np.arange(-180, 180, step=20))

    plt.title(title)
    #plt.tight_layout()
    plt.grid(True)
    plt.savefig("phase_plot.png", bbox_inches='tight')
    print("saved phase_plot.png")
    plt.show()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("filename", help="fits filename")
    parser.add_argument("-t1", "--timestep1", required=False, help="start timestep (1 based index)", default=1)
    parser.add_argument("-t2", "--timestep2", required=False, help="end timestep (defaults to last index)", default=-1)
    parser.add_argument("-a1", "--ant1", required=False, help="antenna1 of baseline", default=-1)
    parser.add_argument("-a2", "--ant2", required=False, help="antenna2 of baseline", default=-1)
    parser.add_argument("-c", "--channel", required=False, help="fine channel number", default=-1)
    parser.add_argument("-a", "--autosonly", required=False, help="Only output the auto correlations", action='store_true')
    parser.add_argument("-p", "--ppdplot", required=False, help="Also create a ppd plot", action='store_true')
    parser.add_argument("-g", "--gridplot", required=False, help="Also create a grid / baseline plot", action='store_true')
    parser.add_argument("-ph", "--phaseplot", required=False, help="Will do a phase plot for a baseline and timestep",
                        action='store_true')
    parser.add_argument("-mt", "--maxtiles", required=False,
                        help="Maximum tiles to use when displaying data or plots. Default is all", default=-1)
    parser.add_argument("-w", "--weights", required=False, help="Dump the weights", action='store_true')
    args = vars(parser.parse_args())

    peek_fits(args["filename"], args["timestep1"], args["timestep2"], args["ant1"], args["ant2"], args["channel"],
              args["autosonly"], args["ppdplot"], args["gridplot"], args["phaseplot"], args["maxtiles"], args["weights"])

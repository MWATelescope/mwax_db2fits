from astropy.io import fits 
import argparse

# freq,baseline,pol
def peek_fits(filename, ant1, ant2, channel, autosonly):
   hdul = fits.open(filename)

   # look at first real hdu
   hdu = hdul[1]
   hdul.info()

   # print hdu stats
   print("time,baseline,chan,ant1,ant2, xx_r, xx_i, xy_r, xy_i,yx_r, yx_i, yy_r, yy_i, power") 
   ant1 = int(ant1)
   ant2 = int(ant2)
   channel = int(channel)
 
   # calculate index
   tiles = 128
   baselines = int((tiles * (tiles + 1)) / 2 )
   pols = 4
   time = 0
   channels = 32

   for hdu in hdul[1:5]:
      baseline = 0

      for i in range(0, tiles):
         for j in range(i, tiles):
            if (i == j and autosonly == True) or (autosonly == False and ((ant1 == -1 or ant1 == i) and (ant2 == -1 or ant2 == j))):
               
               for chan in range(0, channels): 
                   if chan == channel or channel == -1: 
                      index = chan * (pols*2)
                      
                      xx_r = hdu.data[baseline][index]
                      xx_i = hdu.data[baseline][index+1]

                      xy_r = hdu.data[baseline][index+2]
                      xy_i = hdu.data[baseline][index+3]

                      yx_r = hdu.data[baseline][index+4]
                      yx_i = hdu.data[baseline][index+5]

                      yy_r = hdu.data[baseline][index+6]
                      yy_i = hdu.data[baseline][index+7]

                      power = (xx_r * xx_r) + (yy_r * yy_r)

                      print("{0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13}".format(time, baseline, chan, i, j, xx_r, xx_i, xy_r, xy_i, yx_r, yx_i, yy_r, yy_i, power))

            baseline = baseline + 1
      time = time + 1   

   # clean up
   hdul.close() 

   print("Done!\n")

parser = argparse.ArgumentParser()
parser.add_argument("-f", "--filename", required=True, help="fits filename")
parser.add_argument("-a1", "--ant1", required=False, help="antenna1 of baseline", default=-1)
parser.add_argument("-a2", "--ant2", required=False, help="antenna2 of baseline", default=-1)
parser.add_argument("-c", "--channel", required=False, help="fine channel number", default=-1)
parser.add_argument("-a", "--autosonly", required=False, help="Only output the auto correlations", action='store_true')
args = vars(parser.parse_args())

peek_fits(args["filename"],args["ant1"],args["ant2"],args["channel"],args["autosonly"])

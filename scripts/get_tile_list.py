import argparse
from pymwalib.context import Context

def dump(filename):
    context = Context(filename, [])

    print("Input", "Antenna", "Tile_id", "TileName", "Pol", "Rx", "slot")
    for r_index, r in enumerate(context.rf_inputs):
        print(r_index, r.antenna, r.tile_id, r.tile_name, r.pol, r.receiver_number, r.receiver_slot_number)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("filename", help="metafits filename")
    args = vars(parser.parse_args())

    dump(args["filename"])

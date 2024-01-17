import argparse
import io
import struct
import matplotlib
matplotlib.use('Agg')
from matplotlib import pyplot as plt
plt.rcParams['font.family'] = 'Noto Serif CJK JP'

def main():
    parser = argparse.ArgumentParser(description = 'Generate histogram')
    parser.add_argument('--input', '-i', type=argparse.FileType('rb'), required=True)
    parser.add_argument('--output', '-o', required=True)
    args = parser.parse_args()
    input: io.FileIO = args.input
    pixel_value_list = []
    value_count_limit = 3840 * 2160
    for _ in range(value_count_limit):
        value_of_file = input.read(2)
        if len(value_of_file) == 0:
            break
        if len(value_of_file) != 2:
            raise Exception(f"Unexpected value length {len(value_of_file)}")
        pixel_value = struct.unpack('<H', value_of_file)[0]
        pixel_value_list.append(pixel_value)
    fig, ax1 = plt.subplots(figsize=(8, 6), dpi=120)
    ax1.hist(pixel_value_list)
    fig.savefig(args.output, format = 'png')

if __name__ == "__main__":
    main()

import argparse
import io
import matplotlib
import numpy as np
matplotlib.use('Agg')
from matplotlib import pyplot as plt
plt.rcParams['font.family'] = 'Noto Serif CJK JP'

class PixelFormat:
    def __init__(self, pixel_format: str) -> None:
        self.pixel_format = pixel_format

def read_and_bin(input: io.BufferedReader, value_count: int, bin_edges: np.ndarray) -> np.ndarray:
    value_of_file = input.read(value_count * 2)
    if len(value_of_file) < value_count:
        raise Exception(f"Unexpected value length {len(value_of_file)}")
    pixel_value_list: np.ndarray = np.frombuffer(value_of_file, np.dtype('<H'))
    hist, _ = np.histogram(pixel_value_list, bins=bin_edges)
    return hist

def main():
    parser = argparse.ArgumentParser(description = 'Generate histogram')
    parser.add_argument('--input', '-i', type=argparse.FileType('rb'), required=True)
    parser.add_argument('--output', '-o', type=argparse.FileType('wb'), required=True)
    parser.add_argument('--width', type=int, default=3840)
    parser.add_argument('--height', type=int, default=2160)
    parser.add_argument('--frame-offset', type=int, default=0)
    parser.add_argument('--frame-count', type=int, default=1)
    parser.add_argument('--pixel-format', choices=['yuv420p10le'], default='yuv420p10le')
    parser.add_argument('--bit-depth', type=int, choices=[10], default=10)
    parser.add_argument('--bin-count', type=int, default=256)
    args = parser.parse_args()
    input: io.BufferedReader = args.input
    output: io.FileIO = args.output
    width: int = args.width
    height: int = args.height
    frame_offset: int = args.frame_offset
    frame_count: int = args.frame_count
    bit_depth: int = args.bit_depth
    bin_count: int = args.bin_count
    if width % 2 != 0 or height % 2 != 0:
        raise Exception('Both width and height must be even.')
    bin_edges = np.linspace(0, 2 ** bit_depth, bin_count + 1) - 1
    y_value_count_per_frame = width * height
    u_value_count_per_frame = int(width * height / 4)
    v_value_count_per_frame = int(width * height / 4)
    value_count_per_frame = y_value_count_per_frame + u_value_count_per_frame + v_value_count_per_frame
    input.seek(frame_offset * value_count_per_frame * 2)
    yhist = np.zeros(bin_count, np.uint)
    uhist = np.zeros(bin_count, np.uint)
    vhist = np.zeros(bin_count, np.uint)
    for _ in range(frame_count):
        yhist += read_and_bin(input, y_value_count_per_frame, bin_edges).astype(np.uint)
        uhist += read_and_bin(input, u_value_count_per_frame, bin_edges).astype(np.uint)
        vhist += read_and_bin(input, v_value_count_per_frame, bin_edges).astype(np.uint)
    fig, axes = plt.subplots(nrows=3, ncols=1, figsize=(8, 12), dpi=300)
    axy, axu, axv = axes
    common_plot_param = {
        'histtype': 'stepfilled',
        'edgecolor': 'black',
        'color': 'salmon'
    }
    xticks = np.linspace(0, 2 ** bit_depth, 17)
    axy.hist(bin_edges[:-1], bin_edges, weights=yhist, **common_plot_param)
    axy.set_title("Y成分ヒストグラム")
    axy.set_xticks(xticks)
    axy.set_xlabel('画素値')
    axy.set_ylabel('度数')
    axu.hist(bin_edges[:-1], bin_edges, weights=uhist, **common_plot_param)
    axu.set_title("U成分ヒストグラム")
    axu.set_xticks(xticks)
    axu.set_xlabel('画素値')
    axu.set_ylabel('度数')
    axv.hist(bin_edges[:-1], bin_edges, weights=vhist, **common_plot_param)
    axv.set_title("V成分ヒストグラム")
    axv.set_xticks(xticks)
    axv.set_xlabel('画素値')
    axv.set_ylabel('度数')
    plt.subplots_adjust(hspace=0.3, bottom=0.05, top=0.95)
    fig.savefig(output, format = 'png')

if __name__ == "__main__":
    main()

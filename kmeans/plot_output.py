#!/usr/bin/env python

import array
import struct
import sys


import numpy
import matplotlib
matplotlib.use('AGG')

import matplotlib.pyplot as plt

DIM = 2



def split_axes(data):
    return data[0::2], data[1::2]



class DataPlotter:
    def __init__(self, input_file, output_file, png_file):
        self.data = self.read_input(input_file)
        self.n_centroids, self.n_data, self.centroid, self.cluster = self.read_output(output_file)    
        self.png_file = png_file
                
    def plot(self):
        plt.figure(figsize=(8, 8))
        self.set_color_dist()

        self.plot_data()
        self.plot_centroid()

        plt.savefig(self.png_file)

    def set_color_dist(self):
        self.cent_c = range(self.n_centroids)
        self.data_c = self.cluster
        
    def plot_centroid(self):
        c_x, c_y = split_axes(self.centroid)
        plt.scatter(c_x, c_y, c=self.cent_c, s=50, marker='x')

    def plot_data(self):
        d_x, d_y = split_axes(self.data)
        plt.scatter(d_x, d_y, c=self.data_c, s=4, edgecolor='none', alpha=0.8)
        
    def read_output(self, file_name):
        with open(file_name, 'rb') as input_f:
            cls = struct.unpack('I', input_f.read(struct.calcsize('I')))[0]
            cnt = struct.unpack('I', input_f.read(struct.calcsize('I')))[0]
            centroids = array.array('f')
            data = array.array('i')
            centroids.fromfile(input_f, cls * DIM)
            data.fromfile(input_f, cnt)
        return cls, cnt, centroids, data

    def read_input(self, file_name):
        with open(file_name, 'rb') as input_f:
            cls = struct.unpack('I', input_f.read(struct.calcsize('I')))[0]
            cnt = struct.unpack('I', input_f.read(struct.calcsize('I')))[0]
            centroids = array.array('f')
            data = array.array('f')
            centroids.fromfile(input_f, cls * DIM)
            data.fromfile(input_f, cnt * DIM)
        return data



if __name__ == '__main__':
    if len(sys.argv) < 4:
        print('{0} <input file> <output file> <output image>'.format(sys.argv[0]))
    plotter = DataPlotter(sys.argv[1], sys.argv[2], sys.argv[3])
    plotter.plot()

    


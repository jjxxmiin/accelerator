#!/usr/bin/env python

import array
import struct
import sys
import math

DIM = 2
eps = 1e-3

class Verify:
    def __init__(self, answer_file, output_file):
        self.n_centroids, self.n_data, self.centroid, self.cluster = zip(self.read_output(answer_file), self.read_output(output_file))
 
    def read_output(self, file_name):
        with open(file_name, 'rb') as input_f:
            cls = struct.unpack('I', input_f.read(struct.calcsize('I')))[0]
            cnt = struct.unpack('I', input_f.read(struct.calcsize('I')))[0]
            centroids = array.array('f')
            data = array.array('i')
            centroids.fromfile(input_f, cls * DIM)
            data.fromfile(input_f, cnt)
        return cls, cnt, centroids, data

    def verify(self):
        assert self.n_centroids[0] == self.n_centroids[1]
        assert self.n_data[0] == self.n_data[1]

        result_c, result_d = True, True
        
        xy1 = zip(self.centroid[0][0::2], self.centroid[0][1::2])
        xy2 = zip(self.centroid[1][0::2], self.centroid[1][1::2])
        for idx, ((x1, y1), (x2, y2)) in enumerate(zip(xy1, xy2)):
            if math.fabs(x1 - x2) > eps or math.fabs(y1 - y2) > eps:
                print 'centroids[%d] : ans=(%e, %e), out=(%e, %e)' % (idx, x1, y1, x2, y2)
                result_c = False

        for idx, (c1, c2) in enumerate(zip(self.cluster[0], self.cluster[1])):
            if c1 != c2:
                print 'partitioned[%d] : ans=%d, out=%d' % (idx, c1, c2)
                result_d = False
        
	if result_c and result_d:
	    print 'Verification Success'
	else:
	    print 'Verification Failed'

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('{0} <answer file> <output file>'.format(sys.argv[0]))
        sys.exit(1)
    test = Verify(sys.argv[1], sys.argv[2])
    test.verify()


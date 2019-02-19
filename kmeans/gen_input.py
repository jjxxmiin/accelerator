#!/usr/bin/env python

import array
import struct
import random
import sys



DIM = 2
DATA_MIN = 0
DATA_MAX = 100


def gen_data_uniform(n_data, output_f):
    data = array.array('f')

    for i in range(n_data):
        for j in range(DIM):
            d = random.uniform(DATA_MIN, DATA_MAX)
            data.append(d)

    data.tofile(output_f)



def gen_data_normal(n_data, cluster_num, output_f):
    data = array.array('f')
    count = 0

    sigma = 1.0
    perturbation = 0.05

    cluster_size = n_data / cluster_num
        
    while count < n_data:
        start_p = []
        new_p = []
        for d in range(DIM):
            start_p.append(random.uniform(DATA_MIN, DATA_MAX))
            new_p.append(0.0)
            data.append(start_p[d])
        count += 1
        

        for i in range(int(random.gauss(cluster_size, 5.0))):
            for d in range(DIM):
                new_p[d] = start_p[d] + random.gauss(0, sigma)
                data.append(new_p[d])
            if random.random() < perturbation:
                start_p = new_p
            count += 1
            

            if count >= n_data:
                break
                                    
    data.tofile(output_f)
    


if __name__ == '__main__':
    
    if len(sys.argv) < 4:
        print('{0} <# of points> <# of clusters> <output file>'.format(sys.argv[0]))
        sys.exit()

    random.seed()

   
    n_data = int(sys.argv[1])
    n_cluster = int(sys.argv[2])
    output_f = open(sys.argv[3], 'wb')
    output_f.write(struct.pack('I', n_cluster))
    output_f.write(struct.pack('I', n_data))
    gen_data_uniform(n_cluster, output_f)
    gen_data_normal(n_data, n_cluster, output_f)

    output_f.close()
            

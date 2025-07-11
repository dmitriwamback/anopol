//
//  cl_batch_compute.cl
//  anopol
//
//  Created by Dmitri Wamback on 2025-05-19.
//

__kernel void add(__global int* a, __global int* b, __global int* result) {
    int i = get_global_id(0);
    result[i] = a[i] + b[i];
}

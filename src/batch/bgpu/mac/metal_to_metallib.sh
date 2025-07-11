xcrun -sdk macosx metal -c mtl_batch_compute.metal -o mtl_batch_compute.air
xcrun -sdk macosx metallib mtl_batch_compute.air -o mtl_batch_compute.metallib
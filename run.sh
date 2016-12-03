umount ./t && cmake -DCMAKE_BUILD_TYPE=Debug .  && make -j2 &&  ./bin/fuse-example -f -s -d ./tx

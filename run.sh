clear
killall qemu-system-riscv64
make veryclean
make APPLICATION=tests_p1 run
rm img/result.txt
tcpdump -qns 0 -x -r img/tests_p1.pcap > img/result.txt

clear
killall qemu-system-riscv64
make veryclean
make APPLICATION=tests_ea2 run
rm img/test.txt
tcpdump -qns 0 -x -r img/tests_ea2.pcap > img/test.txt

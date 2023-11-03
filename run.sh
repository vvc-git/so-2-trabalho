clear
killall qemu-system-riscv64
make veryclean
echo "\033[31mRemovendo a pasta results\033[0m"
#rm img/result.txt
echo [1;32m Executando o epos![0m
make APPLICATION=tests_p4 run
##tcpdump -qns 0 -x -r img/tests_p2.pcap && code img/tests_p2.txt
## tcpdump -qns 0 -x -r img/p-tests_p2.pcap && code img/p-tests_p2.txt
 


clear
killall qemu-system-riscv64
make veryclean
echo "\033[31mRemovendo a pasta results\033[0m"
rm img/result.txt
echo [1;32m Executando o epos![0m
make APPLICATION=tests_p2 run && tcpdump -qns 0 -x -r img/tests_p2.pcap > img/result.txt && code img/result.txt



clear
killall qemu-system-riscv64
make veryclean
echo "\033[31mRemovendo a pasta results\033[0m"
rm img/sender.txt img/receiver.txt
echo [1;32m Executando o epos![0m
make APPLICATION=tests_p5 run
 


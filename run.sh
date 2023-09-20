clear
killall qemu-system-riscv64
make veryclean
make APPLICATION=tests_ea2 run

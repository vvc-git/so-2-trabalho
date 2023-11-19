
tcpdump -qns 0 -x -r img/tests_p5.pcap -n -vvv > img/receiver.txt && code img/receiver.txt
tcpdump -qns 0 -x -r img/p-tests_p5.pcap -n -vvv > img/sender.txt && code img/sender.txt 
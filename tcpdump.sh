
tcpdump -qns 0 -x -r img/tests_p2.pcap > img/receiver.txt && code img/receiver.txt
tcpdump -qns 0 -x -r img/p-tests_p2.pcap > img/sender.txt && code img/sender.txt 
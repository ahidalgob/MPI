default: A_O3 A_no_O3 A_NO_BCast_O3 A_NO_BCast_no_O3

A_no_O3: A.cpp
	mpic++ A.cpp -o A_no_O3

A_O3: A.cpp
	mpic++ -O3 A.cpp -o A_O3

A_NO_BCast_no_O3: A_NO_BCast.cpp
	mpic++ A_NO_BCast.cpp -o A_NO_BCast_no_O3

A_NO_BCast_O3: A_NO_BCast.cpp
	mpic++ -O3 A_NO_BCast.cpp -o A_NO_BCast_O3

clean:
	rm -f A_no_O3
	rm -f A_O3
	rm -f A_NO_BCast_no_O3
	rm -f A_NO_BCast_O3

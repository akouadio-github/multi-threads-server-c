echo Started

	for i in {1..5} 
	do
		./wclient localhost 30000 /spin.cgi?$i &
		./wclient localhost 30000 /tes$i.html &
	done

echo Finished

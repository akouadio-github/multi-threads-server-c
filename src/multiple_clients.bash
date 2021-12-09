echo Started

	for i in {1..10} 
	do
		./wclient localhost 30000 /spin.cgi?$i &
	done

echo Finished



# echo Starting requests

# for j in $(seq 1 6)
# do
# 	for ((i=5;i>0;i-=1)) 
# 	do
# 		./wclient localhost 30000 /test$i.html &
# 	done
# done

# echo Requests completed
echo Starting requests

	for ((i=5;i>0;i-=1)) 
	do
		./wclient localhost 30000 /spin.cgi?$i &
	done

echo Requests completed

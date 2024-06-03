# !/bin/bash
echo "========================== Starting Functional Testing In Block=============="
# Check if the number of parameters is correct
if [ "$#" -eq 2 ]; then
    is_log=1
    log_path=$2
else
    echo "Usage: $0 <num_iterations> [<path_to_log>]"
    exit 1
fi
num_iterations=$1
tot_iterations=$((num_iterations * 4))
taas=$(python3 -c 'from scripts.conf import taas;')

for ((i=1; i<=$num_iterations; i++)); do    
	for https in 0 1; do
   		for db in 0 1; do
            if [[ "$taas" -eq true && "$db" -eq 1 ]]; then
                continue
            fi
            # Open file in write mode
            exec 3>$log_path
            curr_iteration=$((($i - 1) * 4 + $https * 2 + $db + 1))
            # Redirecting output to sample_log.txt
            {
                echo " "
                echo "===================== TEST NUMBER $curr_iteration OUT OF $tot_iterations ======================="
                echo "Invoking Python Script"
                echo "--------------------- Eliminating Processes -------------------"
                echo "Processing model_owner: " 
                ps -ef| grep -i '*model_owner*' | grep -v grep| awk '{print "kill  -9 "$2}' | sh
                echo "Processing client: "
                ps -ef| grep -i '*build/client*' | grep -v grep| awk '{print "kill  -9 "$2}' | sh
                echo "Processing super dealer: "
                ps -ef| grep -i '*super_dealer*' | grep -v grep| awk '{print "kill  -9 "$2}' | sh
                echo "Processing dealer: "
                ps -ef| grep -i '*dealer*' | grep -v grep| awk '{print "kill  -9 "$2}' | sh
                echo "--------------------------"
                sleep 3
                echo "Running Python script with parameters: $https, $db"
                python3 scripts/end2end_test.py "$https" "$db"
                # Check the exit status of the Python script
                python_exit_status=$?
                # If the exit status is non-zero (indicating an error), exit the Bash script
                if [ $python_exit_status -ne 0 ]; then
                    echo "Python script failed with exit status $python_exit_status"
                    exit $python_exit_status
                fi
            } >&3
            # Close the file descriptor
            exec 3>&-
		done
    done
done
# Open file in write mode
exec 3>$2
# Redirecting output to sample_log.txt
{
    echo " "
    echo "------------------------------------------------------------------------------"
    echo "===SUCCESSFULLY FINISHED ALL $tot_iterations ITERATIONS======================="
    echo "------------------------------------------------------------------------------"
} >&3
# Close the file descriptor
exec 3>&-
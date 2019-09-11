Instructions:

To compile, cd into the directory and run 'make'
To clean up, run 'make clean'

To run the program on a wpi linux server:
- cd into the directory and run 'make'
- first, run './http_server <port number>'
- press CTRL+Z to pause and push the process to the background so the terminal can continue being used
- run 'bg' to start the http_server process in the background
- run './http_client [-p] <url> <port number>' to run the http client
- to stop the server, first run 'jobs' to see the active jobs
    - the number in brackets is the process num, ie "[1]+ Running ./http_server 5555 &"
    - run 'kill %1' to kill process number [1]

NOTE: The TMDG.html file is accessed at <hostname or localhost:port>/files/TMFG.html
example: in browser, type "localhost:3000/files/TMDG.html" or whatever your port number is
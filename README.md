## Multi-threading web server 
An awesome web server :) called **tobi toba server**

## Authors
- KOUAKOU Kouadio Aristide
- SILUE Salimata Ouawortie

## Executables
To create executable you should run the Makefile as follow 
```
prompt > make all
```
To delete all executable do

```
prompt > make clean
```

## Usage 
The server (suppoort only get requests)
```
prompt > ./wserver [-d <base_dir>] [-p <port_num>] [-t s_threads_num] [-b buffer_size]
```
Where 
base_dir: this is the root directory from which the web server should
operate. 

port: the port number that the web server should listen on; Default: 10000.

s_threads_num: the number of worker threads that should be created within the web
server. Must be a positive integer. Default: 1.

buffer_size: the number of request connections that can be accepted at one
time. Must be a positive integer. Note that it is not an error for more or
less threads to be created than buffers. Default: 1.

For example, you could run your program as:
```
prompt > ./wserver -d ./ -p 30000 -t 4 -b 15
```

The client
```
prompt > ./wclient [ <server_ip>] [-p <server_port>] [file_to_fetch_or_execute]
```

## Contributing 
Contributions, issues and feature requests are welcome.
Feel free to check issues page if you want to contribute.

## Credits 
https://gitlab.imt-atlantique.fr/ue-os/2021/concurrency-webserver

## Project status
Slowed down for the moment.

## Licence
Licence [MIT](https://gitlab.imt-atlantique.fr/k21kouak/tobi-toba-server/-/blob/main/LICENSE)


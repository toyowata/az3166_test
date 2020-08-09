# IoT DevKit AZ3166 test on native Mbed OS

## How to build

### Mbed Studio

* Install Mbed Studio
* Launch Mbed Studio
* Menu [File] - [Import Program...]
* Set URL to `https://github.com/toyowata/az3166_test`
* Press `Add Program` button
* Connect IoT DevKit AZ3166 board
* Target automatically set
* Build program

### Mbed CLI
* Install Mbed CLI
  
```
$ git clone https://github.com/toyowata/az3166_test
$ cd az3166_test
$ mbed compile -m az3166 -t gcc_arm --flash
```


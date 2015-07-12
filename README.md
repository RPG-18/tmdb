# tmdb Time Measurement Database


## Requirements

* C++11 compiler support
* Boost.Asio
* [glog](https://code.google.com/p/google-glog/)

## Build

    git clone https://github.com/RPG-18/tmdb.git
    cd tmdb
    mkdir build
    cd build
    cmake ..
    make
    
## Run

    tmdb -d /var/tmdb -p 8080
    
## Protocol

### Put measurement
    
    add measurement_name timestamp value\n
    
```add mymeas 1400000000 300```

### Get measurements

    get_sequence mymeas measurement_name from_timestamp to_timestamp\n
    
```get_sequence mymeas 1400060000 1400079800\n```

Ansver:
    
     timestamp1 value1
     timestamp2 value2
     ...
     done

### Close connection

    exit
    
    
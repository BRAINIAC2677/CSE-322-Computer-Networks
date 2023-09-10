# CSE-322 | Computer Networks 

[Here](/assets/course_outline.pdf) is the course outline.

## Offline-1 | File Server System
`socket programming` `tcp` 
- [Problem Specification](https://github.com/BRAINIAC2677/CIS-Caring-Is-Sharing/blob/main/problem_specification.pdf)
- [Solution](https://github.com/BRAINIAC2677/CIS-Caring-Is-Sharing/tree/main/src)

## Offline-2 | Simulating Static and Mobile Dumbbell Topology
`ns3` 
- [Problem Specification](/simulation%20basic%20|%20ns3/problem_spec.pdf)
- [Static Dumbbell Topology Solution](/simulation%20basic%20|%20ns3/wireless-static-dumbbell)

- #### Running Static Dumbbell Topology
    - Copy the `wireless-static-dumbbell.cc` file to `scratch` directory of ns3.
    - Copy the `wireless-static-dumbbell.sh` and `gnuplotter.gp` file to ns3 directory (`ns-allinone-[version]/ns-[version]`).
    - Run the `wireless-static-dumbbell.sh` file using the following command from ns3 directory:
        ```
         bash ./wireless-static-dumbbell.sh
        ```
    - A `wireless-static-dumbbell-metrics` folder will be created in ns3 directory. It will contain the metrics and graphs of the simulation.
- [Mobile Dumbbell Topology Solution](/simulation%20basic%20|%20ns3/wireless-mobile-dumbbell/)
- #### Running Mobile Dumbbell Topology 
    - Copy the `wireless-mobile-dumbbell.cc` file to `scratch` directory of ns3.
    - Copy the `wireless-mobile-dumbbell.sh` and `gnuplotter.gp` file to ns3 directory (`ns-allinone-[version]/ns-[version]`).
    - Run the `wireless-mobile-dumbbell.sh` file using the following command from ns3 directory:
        ```
         bash ./wireless-mobile-dumbbell.sh
        ```
    - A `wireless-mobile-dumbbell-metrics` folder will be created in ns3 directory. It will contain the metrics and graphs of the simulation.

## Offline-3 | Simulating Congestion Control Algorithms
`ns3` `tcp adaptive-reno`
- [Problem Specification](/congestion%20control%20|%20ns3/problem_spec.pdf)
- [Solution](/congestion%20control%20|%20ns3/)
- #### Running Congestion Control Simulation
    - Copy `tcp-adaptive-reno.cc` and `tcp-adaptive-reno.h` files to `src/internet/model` directory of ns3. Follow the instructions at the beginning of task-2 in problem specification.
    - Make a `congestion control` directory in `scratch` directory of ns3. Copy `congestion-control.cc`, `packet-source.h` and `packet-source.cc` files to `congestion control` directory.
    - Copy all `.gp` and `.sh` files to ns3 directory. 
    - Run the `congestion-control.sh` file using the following command from ns3 directory:
        ```
         bash ./congestion-control.sh
        ```
    - All the metrics and graphs will be created in `congestion-control-metrics` directory of ns3 directory.

## Offline-4 | Error Detection and Correction
`data link layer` `hamming code` `crc`
- [Problem Specification](/error%20detection%20and%20correction/problem_spec.docx)
- [Solution](/error%20detection%20and%20correction/main.cpp)

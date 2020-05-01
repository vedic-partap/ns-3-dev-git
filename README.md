
Network Traffic Dynamics Using Temporal Point Process
================================

## Table of Contents:

1) An overview
2) Building NTPP
2) Building ns-3 model
3) Running NTPP Application
4) Augurnet
5) Working with the development version of ns-3

Note:  Much more substantial information about ns-3 can be found at
http://www.nsnam.org

## An Open Source project

Network Transmission
Point Process (NTPP), a probabilistic deep machinery
that models the traffic characteristics of hosts on a network and
effectively forecasts the network traffic patterns, such as load
spikes. Existing stochastic models relied on the network traffic
being self-similar in nature, thus failing to account for traffic
anomalies. These anomalies, such as short-term traffic bursts,
are very prevalent in certain modern-day traffic conditions, e.g.
datacenter traffic, thus refuting the assumption of self-similarity.
Our model is robust to such anomalies since it effectively
leverages the self-exciting nature of the bursty network traffic
using a temporal point process model.

## Building NTPP

The ns–3 simulator is an open-source networking simulation tool 
implemented by C++ and wildly used for network research and education.
Currently, more and more researchers are willing to apply AI algorithms
to network research. Most AI algorithms are likely to rely on open source
frameworks such as TensorFlow and PyTorch. These two parts are developed 
independently and extremely hard to merge, so it is more reasonable and 
convenient to connect these two tasks with data interaction.

We have build the Deep Learning model using Pytorch in python. 
After training the model we learn the paramters and using
those paramters we build this model in NS-3. 

Currently we haven't integrated the DL model directly with ns-3 code.
Although there are some projects available that can integrate them
Example: [hust-diangroup/ns3-ai](https://github.com/hust-diangroup/ns3-ai)

This module does not provide any AI algorithms or rely on any frameworks 
but instead is providing a Python module that enables AI interconnect, so
the AI framework needs to be separately installed. You only need to clone
or download this work, then import the Python modules, you could use this
work to exchange data between ns-3 and your AI algorithms

## Building ns-3 Model

Follow [How to make and use new application?](https://www.nsnam.org/wiki/HOWTO_make_and_use_a_new_application)

NTPP Model Files

------------

    ├── src               
    │   │
    │   └── applications         
    │       ├── helper
    |       |     │                
    │       |     ├── NTPP-helper.h
    │       |     └── NTPP-helper.cc
    |       |
    |       |
    |       └── model                 
    │             ├── NTPP-application.h
    │             └── NTPP-application.cc
    |      
    └── scratch
          └── Examples
                └──NTPP-application-example.cc
   


--------

## Running NTPP Application
### Installation
#### 1. Install this module in ns-3
##### Get ns-3:  
This module needs to be built within ns-3, so you need to get a ns-3-dev or other ns-3 codes first.

Check [ns-3 installation wiki](https://www.nsnam.org/wiki/Installation) for detailed instructions.
But insteaad of downloading the ns-3-dev-git. Install this repo.
```
cd ns-3-allinone/
git clone https://github.com/vedic-partap/ns-3-dev-git.git
```

##### 2. Build the new application
```
./waf build
```

#### 3. Run NTPP Application Example
```
./waf --run Examples
```

## Augurnet

To see the Pytorch model for NTPP. Visit [vedic-partap/augurnet](https://github.com/vedic-partap/augurnet)
## Getting access to the ns-3 documentation

Once you have verified that your build of ns-3 works by running
the simple-point-to-point example as outlined in 3) above, it is
quite likely that you will want to get started on reading
some ns-3 documentation.

All of that documentation should always be available from
the ns-3 website: http:://www.nsnam.org/documentation/.

This documentation includes:

  - a tutorial

  - a reference manual

  - models in the ns-3 model library

  - a wiki for user-contributed tips: http://www.nsnam.org/wiki/

  - API documentation generated using doxygen: this is
    a reference manual, most likely not very well suited
    as introductory text:
    http://www.nsnam.org/doxygen/index.html

## Working with the development version of ns-3

If you want to download and use the development version of ns-3, you
need to use the tool `git`. A quick and dirty cheat sheet is included
in the manual, but reading through the git
tutorials found in the Internet is usually a good idea if you are not
familiar with it.

If you have successfully installed git, you can get
a copy of the development version with the following command:
```shell
git clone https://gitlab.com/nsnam/ns-3-dev.git
```

However, we recommend to follow the Gitlab guidelines for starters,
that includes creating a Gitlab account, forking the ns-3-dev project
under the new account's name, and then cloning the forked repository.
You can find more information in the manual [link].

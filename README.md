Libforwardsec. Forward secure encryption for asynchronous messaging. [![Build Status](https://travis-ci.org/imichaelmiers/libforwardsec.svg?branch=master)](https://travis-ci.org/imichaelmiers/libforwardsec)
============================

Libforwardsec provides efficient forward-secure public key encryption that tolerates clock skew and delayed or lost messages. Using it, one can build forward secure O-RTT key-exchange or  send forward secure messages even when the recipient is offline. The cryptographic details and performance are examined in the paper "Forward Secure Asynchronous Messaging from Puncturable Encryption" in IEEE Security and Privacy 2015 (see [here](http://cs.jhu.edu/~imiers/pdfs/forwardsec.pdf) for a copy of the paper.)

see example/example.cpp for usage.

The crypto
----------
Authenticated ephemeral key exchange (e.g TLS, TextSecure/OTR) provides forward security if both parties are online. Unless the receiving party has a server acting as a proxy (e.g. as in TextSecure), messages can't be sent when the receiver is offline. This is a problem for email and even chat protocols which do store and forward delivery. Encryption schemes don't have this problem. However, until now the only approaches to forward secure encryption required senders to know  when, according to the recipient's clock, their message would arrive. Given clock skew and network jitter this approach is impractical.

The existing approach to forward secure encryption assigns a public/private key-pair to each time interval (e.g. one key pair per hour) and once a time interval is over, deletes the private key for that interval. Advanced cryptographic techniques  compress this list of keys, resulting in a constant size public key (as opposed to n public keys for n time intervals) and a logarithmic sized private key, so the scheme is actually practical. But once a recipient deletes the key for an interval, no future messages sent to that interval can be decrypted. This forces the use of long intervals to minimize the chance that delays or clock skew cause messages to arrive after their interval expires. Since long intervals expose more messages in the event of compromise, this is a major issue.

Puncturable encryption, instead of deleting keys when time intervals expire, updates ("punctures") keys so that they can't decrypt already received ciphtertexts. Any other ciphertexts (e.g. that arrived late) are still decryptable. Combining this technique with existing approaches gives a scheme which is efficient under normal conditions and tolerant of late messages and clock skew. Key material should still be deleted after a long period (e.g. 24 hours) to ensure actively intercepted and not delivered messages are also forward secure, but this is optional and crucially that time interval for this is determined merely by the user's threat model and not by network conditions and clock accuracy.

The library
-----------
This library provides a high level solution (GMPfse) to forward secure encryption which is a combination of the puncturable encryption scheme (GMPpke) and the one key per time interval approach (BBHHibe). Each scheme can be used separately.  At the moment, the library only provides public key encryption of 256 bit values. This needs to be combined manually with a symmetric scheme or better yet a symmetric ratcheting scheme such as Axolotl. 

While the code is in decent shape and certainly better than many academic libraries, We haven't even gone over it thoroughly ourselves.  If you are interested in using it, please contact us. We'd love to use it used and  willing to do a more thorough review and other work. At the moment, however, it might be secure enough to send cat pictures over the Internet, but don't count on it unless you are using it merely to add forward security and you're messages are encrypted with something else already.

Security concerns:
* This library uses RELIC for pairings which itself uses GMP. Both may  have timing issues  and buffer overflows/ other exploit issues.  
* Cereal(used for serialization) may or may not be secure for untrusted input.


TODOs:
* profile and optimize code. It's slower than we expect (i.e. the cost of pairings doesn't dominate)
* remove extra group element in ciphertext for combined scheme
* add symmetric encryption and ratcheting

Dependencies
-----------------------------
* For math: 
    * relic (https://github.com/relic-toolkit/relic) 
    * gmp (assuming relic is configured to use it) 
* For serialization:
    *  cereal(https://github.com/USCiLab/cereal) 
* For testing:
    * Google test (https://github.com/google/googletest) 
    

Build
----------------------------
This project builds with cmake.

    #For everything:
    cd build/ 
    cmake ../
    make 
    make check # (optional) builds and runs tests.
    make bench # (optional) builds and runs benchmarks
    make install 

It is known to build and pass all tests on OSX 10.9.5  with Apple LLVM version 6.0 (clang-600.0.54) (based on LLVM 3.5svn) 
and Ubuntu 14.04 with gcc version 4.8.2 (Ubuntu 4.8.2-19ubuntu1). It is not known explicitly to fail on anything.

Note, Cmake supports a bunch of other build systems via "cmake -G 'generator name' ../"
Among them Ninja and project files for your favorite/
least favorite IDEs (xcode, eclipse,visualstudio, etc)

For a full list of supported build systems run:
cmake --help



Installing  dependencies
----------------------------

### Relic (note, depends on gmp)
    #from a suitable directory
    git clone https://github.com/relic-toolkit/relic.git
    cd relic
    git checkout relic-toolkit-0.4.0 
    cmake -G "Unix Makefiles" -DMULTI=OPENMP -DCOMP="-O2 -funroll-loops -fomit-frame-pointer" -DARCH="X64"  -DRAND="UDEV" -DWITH="BN;DV;FP;FPX;EP;EPX;PP;PC;MD" -DCHECK=off -DVERBS=off -DDEBUG=on -DBENCH=0 -DTESTS=1 -DARITH=gmp -DFP_PRIME=256 -DFP_QNRES=off -DFP_METHD="BASIC;COMBA;COMBA;MONTY;LOWER;SLIDE" -DFPX_METHD="INTEG;INTEG;LAZYR" -DPP_METHD="LAZYR;OATEP"
    make 
    make install 
    
### GMP
Install using your package manager.
    
### Cereal
Cereal is a header only library.  Try your local package manager or 
    
    #from a suitable directory
    git clone https://github.com/USCiLab/cereal.git
    cd cereal
    git checkout v1.0.0
    cp -R ./include/* /usr/local/include/

### Google test
    #from the root of the libforwardsec project
    git submodule init
    git submodule update

Android BUILD
----------------------------
1.  Make a standalone toolchain using the script in the sdk
      
        #pick install location
        export STANDALONE_TOOLCHAIN="/PATH/TO/TOOLCHAIN"
        ./make-standalone-toolchain.sh  --platform=android-21 --install-dir=$STANDALONE_TOOLCHAIN --ndk-dir=/PATH/TO/NDK/android-ndk-r10d/ --arch=arm --system=linux-x86_64  --toolchain=arm-linux-androideabi-clang3.5 --stl=libc++
     
2. Build gmp (note using the copy form https://github.com/Rupan/gmp doens't work)

    1. make/export the standalone toolchain

            export CC="$STANDALONE_TOOLCHAIN/bin/arm-linux-androideabi-gcc --sysroot=$STANDALONE_TOOLCHAIN/sysroot"
            export CXX="$STANDALONE_TOOLCHAIN/bin/arm-linux-androideabi-g++ --sysroot=$STANDALONE_TOOLCHAIN/sysroot"
            export AR="$STANDALONE_TOOLCHAIN/bin/arm-linux-androideabi-ar"
            export RANLIB="$STANDALONE_TOOLCHAIN/bin/arm-linux-androideabi-ranlib"
            export SYSROOT="$STANDALONE_TOOLCHAIN/sysroot"

    2. Optimize compiler
    
            export CFLAGS="-O4 -flto -Ofast -mcpu=cortex-a15 -fprefetch-loop-arrays -mfpu=neon -funroll-all-loops -mtune=cortex-a15 -ftree-vectorize -fomit-frame-pointer -mvectorize-with-neon-quad -mthumb-interwork -finline-small-functions  -ffast-math -marm -ffunction-sections -fdata-sections -fomit-frame-pointer -finline-small-functions "
    3. Configfure
    
            ./configure --host=arm-linux-androideabi
    
    4.  edit config.h and make sure the following is set to zero. (note, this is set by ./configure, so your change will be overwritten)

             /* Define to 1 if you have the `localeconv' function. */
              #define HAVE_LOCALECONV 0
    

    5. Build
    
             make -j

2. build relic 
    1. remove src/CMakeLists.txt:153:      
              
              target_link_libraries(${LIBRARY} rt)
  
   2. Configure
    
    
              cmake -DCMAKE_TOOLCHAIN_FILE=./android.toolchain.cmake -DANDROID_ABI="armeabi-v7a with NEON"  -DANDROID_STANDALONE_TOOLCHAIN=/home/ian/android_standalone/  -DCOMP="-O4 -flto -Ofast -mcpu=cortex-a15 -fprefetch-loop-arrays -mfpu=neon -mfloat-abi=hard -funroll-all-loops -mtune=cortex-a15 -ftree-vectorize -fomit-frame-pointer -mvectorize-with-neon-quad -mthumb-interwork -finline-small-functions  -ffast-math -marm -ffunction-sections -fdata-sections -fomit-frame-pointer -finline-small-functions" -DARCH="NONE"  -DRAND="UDEV" -DWITH="BN;DV;FP;FPX;EP;EPX;PP;PC;MD" -DCHECK=off -DVERBS=off -DDEBUG=on -DBENCH=0 -DTESTS=1 -DARITH=gmp -DFP_PRIME=256 -DFP_QNRES=off -DFP_METHD="BASIC;COMBA;COMBA;MONTY;LOWER;SLIDE" -DFPX_METHD="INTEG;INTEG;LAZYR" -DPP_METHD="LAZYR;OATEP" -DWORD="32" -DCHECK=off and -DTRACE=off -DCMAKE_BUILD_TYPE=Release
    
    
   3. Build
   
                make -j 

3. build libforwardsec

              cmake -DCMAKE_TOOLCHAIN_FILE=../android.toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DANDROID_ABI="armeabi-v7a with NEON" -DANDROID_STANDALONE_TOOLCHAIN=$STANDALONE_TOOLCHAIN ../


4. running (this should work without root)
Load libmgp.so, librelic.so, libforwarsec.so, and an executable into /data/local/tmp/
    
         adb push file /data/local/tmp/ # for ibmgp.so, librelic.so, libforwarsec.so
         adb shell 
         cd /data/local/tmp/
         export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/data/local/tmp/
         ./executable

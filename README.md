# factorial2kr

Tool to perform factorial 2kr analysis.

For an intro on factorial 2 kr design see slides from Prof. Raj Jain [here](https://www.cs.rice.edu/~johnmc/comp528/lecture-notes/Lecture12_13.pdf) or just ask Google.

## Heritage preservation

In the directory `historical` you find a mirror of the [Factorial2k](http://cng1.iet.unipi.it/wiki/index.php/Factorial2kr) tool, which can be used
to realise factorial 2kr design using an unmaintained framework called [ANSWER](http://cng1.iet.unipi.it/wiki/index.php/ANSWER) originally developed as a plug-in of the [Network Simulator - ns2](https://www.isi.edu/nsnam/ns/).

The framework assumes that the raw data to perform the analysis are stored in binary files serialized using a proprietary undocumented structure.

To compile the tool (for example in debug mode using compiler clang++):

```
cd build/debug
../buildme.sh clang++
make
```

This will generate the executable `src/main`, which is intended to be launched from `scripts/launch.py`, also poorly documented but shipped with a configuration file example `scripts/factorial.xml`.

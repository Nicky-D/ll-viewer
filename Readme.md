## This is just a collection of patches and changes I collected over the years to make the second life viewer compile under Linux. It might be useful to some people, but comes with no support. If you find issues feel free to correct them and submit a pull request.



### Required system packages

- build-essentials
- python3-setuptools
- python3-pip
- mesa-common-dev
- libgl1-mesa-dev
- libxinerama-dev
- libxrandr-dev
- libpulse-dev
- libglu1-mesa-dev

### Required pip packages

- cmake (usually newer than the system version)
- ninja

It is suggested to install them to your user directory, eg pip install --user cmake. But then usually you will need to add the directory (often .local/bin) to your PATH

### To build the viewer

- cd into the "indra" directory
- run "cmake --preset LinuxRelease"
- run "cmake --build --preset LinuxRelease"

If all went all the resulting binary is in <source_dir>/build-linux-64/newview/packaged
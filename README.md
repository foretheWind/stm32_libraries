stm32_libraries
===============

This is a library for yuneec platform


How to Use It:

1. Put this library within the same floder of 'ardupilot'；

2. Check the name of library, and change it to 'libraries'；

3. Open the terminal and 'cd' into this 'libraries' floder;

4. Type: make optLIB=3

5. Then you will get libraries compiled with full optimization 
   and a archive file named 'libstm32f37x.a' in the 'libraries' floder;
   
6. All Done;

Caution:

If you want to modify files within 'libraries' floder, please remember 

'make clean' and 'make' to rebuild the whole library to take effect.

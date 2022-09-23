Here I have created an image processing application that uses trees and recursion.
The app is built in C language and is able to compress, decompress and mirror
a .ppm file based on the command line arguments it receives.

The program receives a .ppm image format file. It reads the file and creates a quadratic
tree based on the input. Notice that the image is represented as a square that is later 
divided into 4 smaller squares and those are divided into other 4 smaller squares and so on.
Every square represents a node in the tree with the root being the whole image.

I have created a structure that represents a block based on its RGB colors (the average red,
green and blue), the area it covers and the area its quarters (all four smaller squares).

For the compression part I used recursion to unify every 4 blocks of the same color so the
compressed image would not have at least 4 identical pixels next to each other but one bigger
block.

The decompression part was the opposite of the compression and I recreated the image by breaking
down all bigger blocks.

For the mirroring part, I just recursively swapped the 4 smaller blocks.
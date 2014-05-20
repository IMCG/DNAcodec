/* bin2dec.c - convert a binary string to a decimal number, returns decimal value */

/* Copyright (c) 2014 by Allen Yu
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *  */
int bin2dec(char *bin)   
{
  int  b, k, m, n;
  int  len, sum = 0;

  len = strlen(bin) - 1;
  for(k = 0; k <= len; k++) 
  {
    n = (bin[k] - '0'); // char to numeric value
    if ((n > 1) || (n < 0)) 
    {
      puts("\n\n ERROR! BINARY has only 1 and 0!\n");
      return (-1);
    }
    for(b = 1, m = len; m > k; m--) 
    {
      // 1 2 4 8 16 32 64 ... place-values, reversed here
      b *= 2;
    }
    // sum it up
    sum = sum + n * b;
    //printf("%d*%d + ",n,b);  // uncomment to show the way this works
  }
  return(sum);
}

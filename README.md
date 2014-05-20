DNAcodec
========

Program to encode binary files to DNA sequences with embedded error correction

#Introduction
Under construction

#Installation

##Prerequisites
*lzma
```bash
#For Debian\Ubuntu
sudo apt-get install lzma lzma-dev
```
*libxml2
```bash
#For Debian\Ubuntu
sudo apt-get install libxml2 libxml2-dev
```

##Compilation
As simple as typing "make" in your terminal. If your libxml2 is not located in /usr/include/libxml2, you can override the location by:
```bash
make LIBXML="YOUR libxml2 LOCATION"
```

#Sample Usage

##Encoding files
Run "run_encode.sh" inside the "scripts" directory to convert the binary file to DNA in single-line fasta format.

```bash
cd DNAcodec
scripts/run_encode.sh source_file output_file
```

##Decoding files
Run "run_decode.sh" inside the "scripts" directory to convert DNA in single-line fasta format to binary file.

```bash
cd DNAcodec
scripts/run_encode.sh source_file output_file
```

#Advanced Usage
Under construction

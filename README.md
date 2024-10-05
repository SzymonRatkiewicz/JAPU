# JAPU A.K.A. J̶o̶l̶l̶y̶ ̶a̶s̶s̶ ̶p̶n̶g̶ ̶u̶t̶i̶l̶

### Overview 
JAPU is a simple command-line tool that converts `.png` images into ASCII art. \
Yap yap yap.

### Usage
You can find the exectuable in `bin` directory currently named `japu` feel free to move and rename it,
binary itself currently takes in a few arguments.
 
#### Arguments

| args       | explanation                                                                                                                                               | use &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;|
|------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------|-----------------------------------------------------|
| `filename` | Path to a png file that you want to convert to ASCII art                                                                                                  | `japu ./png-file`                                   |
| `output`   | Path to an output file, **this is bare minimum needed to use JAPU**                                                                                       | `japu ./png-file ./output-file`                     |
|  `-w`      | Optional flag, outputs an html file with basic CSS styling, if a path is not provided, warning will be displayed and a default name will be provided. Additionally this flag could be combined with regular path outputing two files.|`japu ./png-file -w ./html-file` **OR** `japu ./png-file -w` **OR** `japu ./png-file ./output-file -w ./html-file`|

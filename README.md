# Custom Linux/Unix Shell  

## Description  
This project is a custom shell for Linux/Unix systems, built from scratch in C. It provides basic command execution, process management, and supports built-in commands.  

## Features  
- Execute system commands  
- Handle foreground and background processes  
- Support for built-in commands (e.g., `cd`, `exit`, `pwd`)  
- Input parsing and redirection  
- Signal handling (e.g., `Ctrl+C`, `Ctrl+Z`)  

## Installation  
1. Clone the repository:  
   ```sh
   git clone https://github.com/yourusername/custom-shell.git
   cd custom-shell
   ```  
2. Compile the source code:  
   ```sh
   make
   ```
   
## Usage  
- Run system commands:  
  ```sh
  ls -l
  gcc main.c -o program
  ```  
- Change directory:  
  ```sh
  cd /path/to/directory
  ```  
- Exit the shell:  
  ```sh
  exit
  ```  
- Run a process in the background:  
  ```sh
  ./program &
  ```  

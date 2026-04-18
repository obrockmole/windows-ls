# Windows ls

A simple implementation of the Unix `ls` command for Windows.

## Current Features

- List files in the current directory or a specified directory.
- `-a`, `--all`: Do not ignore entries starting with `.`.
- `-f`: Same as `-a -U` (show all and do not sort).
- `-r`, `--reverse`: Reverse the order of the sort.
- `-U`: Do not sort; list entries in the order they appear in the directory.
- `-w <width>`: Set the output width. `0` means no limit.
- `-1`: List one file per line.
- `--help`: Display the help message.
- `--version`: Display version information.

## Installation

### Download

1. Download the latest `ls.exe` file from [releases](https://github.com/obrockmole/windows-ls/releases).
2. Place `ls.exe` in a directory of your choice. See "Adding to Path" below.

### Build From Source

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/obrockmole/windows-ls.git
    cd windows-ls
    ```

2.  **Create a build directory:**
    ```bash
    mkdir build
    cd build
    ```

3.  **Run CMake to configure the project:**
    ```bash
    cmake ..
    ```

4.  **Build the project:**
    ```bash
    cmake --build .
    ```

After the build is complete, you will find `ls.exe` in the `build/Debug` directory.

## Adding to Path (Optional)

To use `ls` from any location, it needs to be added to the system's PATH environment variable.

1. Find the full path to the directory where `ls.exe` was built or downloaded (e.g., `C:\Users\User\Downloads\windows-ls\build\Debug`).
2. Some find it better to move `ls.exe` to a more permanent location, such as `C:\Program Files\windows-ls` or `C:\Users\User\windows-ls`, for a more central location
3. Search for "Environment Variables" in Windows search and find "Edit the system environment variables".
4. It should open the "System Properties" window on the "Advanced" tab. Click on the "Environment Variables..." button at the bottom right.
5. In the "Environment Variables" window, under "User variables for ...." or "System variables", find the variable named "Path" and select it, then click "Edit...". The difference between user and system variables is who has access to the command. If you want to add `ls` for all users, edit the system variable; if you only want it for your account, edit the user variable.
6. In the "Edit environment variable" window, click "New" and add the full path to the directory containing `ls.exe` (e.g., `C:\Program Files\windows-ls`).
7. Click "OK" on all open windows to save the changes.
8. Now you should be able to type `ls` in a command prompt window in any directory.

## Usage

Run the executable from the command line. Usage is the same as the Unix `ls` command:

```bash
ls [OPTION]... [DIRECTORY]...
Options:
  -a, --all                  do not ignore entries starting with .
  -f                         same as -a -U
  -r, --reverse              reverse order while sorting
  -U                         do not sort; list entries in directory order
  -w                         set output width. 0 means no limit
  -1                         list one file per line
      --help     display this help message and exits
      --version  output version information and exits
```
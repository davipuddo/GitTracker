# GitTracker 🔍

A project that aims to keep all repositories up to date. 

## Compile & Run

Open a terminal on the source folder and compile the code with:

```fish
cargo build --release
```

The GitTracker binary file is going to be located at: `target/release/`


## Setup

If it is your first time using GitTracker, run the binary first. It is going to create the repository file at your base config directory: `$HOME/.config` (Linux), `C:\Users\username\AppData\Roaming` (Windows).

After running it check for the folder `GitTracker` or `gittracker` at your respective config directory. Inside it is going to be the `repos.txt` file. Open this file and write the paths to your repositories:

```	
repository 1 path
repository 2 path
```

Note that all paths must be **absolute paths**.

use std::{
    fs::{
        self, File
    }, 
    process::{
        Command,
    },
    path::PathBuf
};

use directories::ProjectDirs;

#[derive(Debug)]
enum TrackerError {
    HomeDirNotFound,
    FileCreationFailed,
    FileOpeningFailed,
    FetchFailed
}

#[derive(Debug)]
enum Status {
    UpToDate,
    Behind,
    Unknow,
    Uncommited,
}

use TrackerError::*;
use Status::*;

fn get_config() -> Result<Vec<String>, TrackerError> {
    if let Some(dir) = ProjectDirs::from("org", "GitTracker", "GitTracker") {
        let mut config_dir = dir.config_dir().to_path_buf();
        config_dir.push("repos.txt");

        match config_dir.try_exists() {
            Err(_) | Ok(false) => {
                if let Err(e) = File::create(config_dir.clone()) {
                    eprintln!("{}", e);
                    return Err(FileCreationFailed);
                }
            }
            _ => ()
        }

        let repos = match fs::read_to_string(config_dir) {
            Ok(data) => data,
            Err(e) => {
                eprintln!("{:?}", e);
                return Err(FileOpeningFailed);
            }
        };

        let lines : Vec<String> = repos.lines()
                                     .filter(|s| !s.is_empty())
                                     .map(|slice| slice.to_string())
                                     .collect();

        return Ok(lines);
    }
    Err(HomeDirNotFound)
}

fn fetch(path: String) -> Result<(), TrackerError> {
    match Command::new("git")
        .arg("fetch")
        .current_dir(path)
        .output()
    {
        Ok(_) => Ok(()),
        Err(e) => {
            eprintln!("{}", e);
            Err(FetchFailed)
        }
    }
}

fn check(path: String) -> Status {
    let output = match Command::new("git")
        .args(["status", "-sb"])
        .current_dir(path)
        .output()
    {
        Ok(status) => status,
        Err(_) => return Unknow
    };

    let output = match String::from_utf8(output.stdout) {
        Ok(str) => str,
        Err(e) => {
            eprintln!("{}", e);
            return Unknow;
        }
    };
        
    let lines : Vec<&str> = output.lines()
                            .filter(|s| !s.is_empty())
                            .collect();

    return if lines.len() == 1 {
        if lines[0].contains(&"[behind".to_string()) {
            Behind
        } else {
            UpToDate
        }
    } else {
        Uncommited
    };
}



fn main() {

    let repos = match get_config() {
        Ok(path) => path,
        Err(e) => panic!("{:?}", e)
    };

    for repo in repos {

        if !repo.is_empty() {
            let name = match PathBuf::from(repo.clone()).file_name() {
                Some(name) => name.to_str().unwrap().to_string(),
                None => String::from("Unknow"),
            };

            print!("{}: ", name);

            if let Err(e) = fetch(repo.clone()) {
                eprintln!("fetching has failed. {:?}", e);
            };

            match check(repo.clone()) {
                UpToDate => println!("Ok"),
                Behind => println!("Behind"),
                Uncommited => println!("Uncommited local changes"),
                Unknow => println!("Repository status is unknow"),
            };
        }
    }
}

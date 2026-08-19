use std::{
    io,
    fs::{
        self, File
    }, 
    path::PathBuf
};

use futures::stream::{self, StreamExt};

use directories::ProjectDirs;

use tokio::process::Command;

#[derive(Debug)]
enum TrackerError {
    HomeDirNotFound,
    DirectoryCreationFailed,
    FileCreationFailed,
    FileOpeningFailed,
    FetchFailed
}

#[derive(Debug, Clone)]
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

        if let Err(e) = fs::create_dir_all(&config_dir) {
            eprintln!("{}", e);
            return Err(DirectoryCreationFailed);
        };

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

async fn fetch(path: String) -> Result<(), TrackerError> {
    match Command::new("git")
        .arg("fetch")
        .current_dir(path)
        .output()
        .await
    {
        Ok(_) => Ok(()),
        Err(e) => {
            eprintln!("{}", e);
            Err(FetchFailed)
        }
    }
}

async fn check(path: String) -> Status {
    let output = match Command::new("git")
        .args([
            "-c", "core.fileMode=false",
            "status", "-sb"
        ])
        .current_dir(&path)
        .output()
        .await
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

#[tokio::main]
async fn main() {

    let repos = match get_config() {
        Ok(path) => path,
        Err(e) => panic!("{:?}", e)
    };

    stream::iter(repos)
    .for_each_concurrent(None, |repo| async move {

        if !repo.is_empty() {
            let name = match PathBuf::from(repo.clone()).file_name() {
                Some(name) => name.to_str().unwrap().to_string(),
                None => String::from("Unknow"),
            };

            if let Err(e) = fetch(repo.clone()).await {
                eprintln!("fetching has failed. {:?}", e);
            };

            match check(repo.clone()).await {
                UpToDate => println!("{}: Ok", name),
                Behind => println!("{}: Behind", name),
                Uncommited => println!("{}: Uncommited local changes", name),
                Unknow => println!("{}: Repository status is unknow", name),
            };
        }
    })
    .await;

    println!("\nPress [Enter] to exit!");
    let mut buffer = String::new();
    let _ = io::stdin().read_line(&mut buffer);
}

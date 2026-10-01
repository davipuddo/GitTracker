use std::{
    io,
    fs::{
        self, File
    }, 
    env,
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

fn get_config_dir() -> Result<PathBuf, TrackerError> {
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
            _ => return Ok(config_dir)
        }
    }
    Err(HomeDirNotFound)
}

fn get_config() -> Result<Vec<String>, TrackerError> {

    let config_dir = get_config_dir()?;

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

async fn track() {

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

fn open_editor() -> Result<(), TrackerError> {

    let config_file = get_config_dir()?
        .clone()
        .into_string()
        .expect("Could not convert config path to a string");

    let editor = match std::env::var("EDITOR") {
        Ok(editor) => editor,
        Err(_) => {
            eprintln!("EDITOR variable is not set!\nDefaulting to VIM");
            String::from("vim")
        }
    };

    std::process::Command::new(&editor)
    .arg(&config_file)
    .status()
    .expect(&format!("Could not run {} {}", editor, &config_file));

    Ok(())
}

fn help() {
    let options = [
        ("edit, -e", "open the configuration file with the editor defined by $EDITOR"),
    ];

    println!("\n{:<10} GitTracker [OPTIONS]", "Usage: ");

    println!("\nOptions:");
    for (left, right) in options {
        println!("  {:<30} {}", left, right);
    }

}

#[tokio::main]
async fn main() {

    let mut args: Vec<String> = env::args().collect();
    args.remove(0);

    let len = args.len();
    if len > 1 {
        eprintln!("Too many arguments");
        help();
    } else if len > 0 {
        match args[0].as_str() {
            "-e" | "edit" => {
                if let Err(e) = open_editor() {
                    panic!("Failed to open the config file: {:?}", e);
                }
            },
            _ => {
                eprintln!("Unknow argument: [{}]", args[0]);
                help();
            }
        };
    } else {
        track().await;
    }
}

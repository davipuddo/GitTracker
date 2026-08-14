mod repos;

use crate::repos::{
    get_config,
    fetch_check,
    get_name,
    Status::*
};

use std::io;

use rayon::prelude::*;

fn main () {
    let repos = match get_config() {
        Ok(path) => path,
        Err(e) => panic!("{:?}", e)
    };

    repos.par_iter()
    .for_each(|repo| {
        if !repo.is_empty() {

            let name = get_name(repo);

            match fetch_check(repo) {
                UpToDate => println!("{}: Ok", name),
                Behind => println!("{}: Behind", name),
                Uncommited => println!("{}: Uncommited local changes", name),
                Unknow => println!("{}: Repository status is unknow", name),
            }
        }
    });

    println!("\nPress [Enter] to exit!");
    let mut buffer = String::new();
    let _ = io::stdin().read_line(&mut buffer);
}
